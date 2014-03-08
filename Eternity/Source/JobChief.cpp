#include "r3dPCH.h"
#include "r3d.h"

#include "JobChief.h"
#include "r3dDebug.h"

JobChief* g_pJobChief = 0;
//------------------------------------------------------------------------

namespace
{
	unsigned int WINAPI JobChiefThreadFunc( void* );
	volatile LONG gTimeToExit = false;

	struct JobDesc
	{
		JobChief::ExecFunc	Func ;
		void*				Data ;

		int					ChunkSize ;
		int					Left ;

		volatile LONG		InAccess ;

		JobDesc();
	} gJobDesc ;

	struct ThreadData
	{
		HANDLE			StartEvent ;
		volatile LONG	Idle ;
		HANDLE			IdleEvent;

		ThreadData()
		: StartEvent(INVALID_HANDLE_VALUE)
		, IdleEvent(INVALID_HANDLE_VALUE)
		{}
	};

	typedef r3dTL::TArray< ThreadData > ThreadDataVec;

	ThreadDataVec		gThreadDataVec;

	typedef r3dTL::TArray<HANDLE> IdleHandles;

	IdleHandles			gIdleHandles;

	void DoJob();
}


//------------------------------------------------------------------------

JobChief::JobChief()
: mInQueueMode( false )
{
}

//------------------------------------------------------------------------

JobChief::~JobChief()
{

}

//------------------------------------------------------------------------

void
JobChief::Init()
{
	SYSTEM_INFO sysinfo;
	GetSystemInfo( &sysinfo );

#if 0
	if( !r_multithreading->GetInt() )
		sysinfo.dwNumberOfProcessors = 1;
#endif


#if 1
	mThreadHandles.Resize( sysinfo.dwNumberOfProcessors - 1 );
#else
	mThreadHandles.Resize( R3D_MAX( (int)sysinfo.dwNumberOfProcessors - 2, 0 ) );
#endif

	gThreadDataVec.Resize( mThreadHandles.Count() );
	gIdleHandles.Resize( mThreadHandles.Count() );

	gTimeToExit = false;

	char EventName[ 128 ] = { 0 };

	DWORD procID = GetCurrentProcessId( );

	for( uint32_t i = 0, e = mThreadHandles.Count(); i < e; i ++ )
	{
		_snprintf( EventName, sizeof EventName - 1, "WarInc_Start_Multithreading%u,%u", procID, i );

		ThreadData& td = gThreadDataVec	[ i ];

		td.StartEvent				= CreateEvent( NULL, FALSE, FALSE, EventName ) ;
		td.Idle						= 1 ;
		
		_snprintf( EventName, sizeof EventName - 1, "WarInc_Idle_Event%u,%u", procID, i );
		td.IdleEvent				= CreateEvent( NULL, TRUE, TRUE, EventName ) ;
		gIdleHandles		[ i ]	= td.IdleEvent;

		mThreadHandles		[ i ]	= (HANDLE)_beginthreadex( NULL, 0, JobChiefThreadFunc, &td, 0, NULL ) ;
        if(mThreadHandles[ i ] == NULL)
            r3dError("Failed to begin thread");


		r3d_assert( mThreadHandles[ i ] );
		r3d_assert( td.StartEvent );
	}
}

//------------------------------------------------------------------------

void
JobChief::Close()
{
	InterlockedExchange( &gTimeToExit, 1 );

	FireThreads();

	for( uint32_t i = 0, e = mThreadHandles.Count(); i < e; i ++ )
	{
		DWORD exitCode ( 0 ) ;

		for( ; !GetExitCodeThread( mThreadHandles[ i ], &exitCode ); )
		{
			Sleep( 0 );
		}
		
		CloseHandle( mThreadHandles[ i ] );
	}

	for( uint32_t i = 0, e = gThreadDataVec.Count(); i < e; i ++ )
	{
		CloseHandle( gThreadDataVec[ i ].StartEvent );
		CloseHandle( gThreadDataVec[ i ].IdleEvent );
	}
}

//------------------------------------------------------------------------

void
JobChief::Exec( ExecFunc func, void* data, size_t itemCount )
{
	R3DPROFILE_FUNCTION("JobChief::Exec");
	uint32_t coreCount = mThreadHandles.Count() + 1 ;
	uint32_t perCore = itemCount / coreCount ;

	gJobDesc.Func		= func;
	gJobDesc.Data		= data;
	gJobDesc.Left		= itemCount ;

	if( r_multithreading->GetInt() )
	{
		gJobDesc.ChunkSize	= R3D_MAX( (int)itemCount / 32, 1 );
		FireThreads();
	}
	else
	{
		gJobDesc.ChunkSize = itemCount ;
	}

	DoJob();

	R3DPROFILE_START("JobChief::Exec Wait for result");
	if( r_multithreading->GetInt() )
	{
		if (d_job_chief_idle_events->GetBool())
		{
			WaitForMultipleObjects(gIdleHandles.Count(), &gIdleHandles[0], TRUE, INFINITE);
		}
		else
		{
			bool notAllDone = true ;

			for( ; notAllDone ; )
			{
				notAllDone = false ;

				for( uint32_t i = 0, e = gThreadDataVec.Count(); i < e; i ++ )
				{
					if( !gThreadDataVec[ i ].Idle )
					{
						notAllDone = true ;
						break ;
					}
				}
			}
		}
	}
	R3DPROFILE_END("JobChief::Exec Wait for result");
}

//------------------------------------------------------------------------

void
JobChief::BeginQueueMode()
{
	r3d_assert( !mInQueueMode );
	mInQueueMode = true ;
}

//------------------------------------------------------------------------

void
JobChief::EndQueueMode()
{
	r3d_assert( mInQueueMode );
	mInQueueMode = false ;
}

//------------------------------------------------------------------------

void
JobChief::FireThreads()
{
	R3DPROFILE_FUNCTION("JobChief::FireThreads");
	for( uint32_t i = 0, e = gThreadDataVec.Count(); i < e; i ++ )
	{
		ThreadData& td = gThreadDataVec[ i ] ;
		r3d_assert( td.Idle );

		td.Idle = 0 ;
		ResetEvent(td.IdleEvent);

		SetEvent( td.StartEvent );
	}
}

//------------------------------------------------------------------------

uint32_t
JobChief::GetThreadCount() const
{
	return mThreadHandles.Count() + 1;
}

//------------------------------------------------------------------------

namespace
{
	unsigned int WINAPI JobChiefThreadFunc( void* Par)
	{
		r3dThreadAutoInstallCrashHelper crashHelper;
		ThreadData* data = (ThreadData*)Par ;
		HANDLE StartHandle = data->StartEvent ;

		r3dRandInitInTread rand_in_thread;

		for( ;!gTimeToExit; )
		{
			if( !r_multithreading->GetInt() )
				Sleep( 1 );
			else
			{
				WaitForSingleObject( StartHandle, INFINITE );
				DoJob();
				data->Idle = 1 ;
				SetEvent(data->IdleEvent);
			}
		}

		return 0;
	}

	//------------------------------------------------------------------------

	void DoJob()
	{
		for( ; ; )
		{
			for( ; InterlockedExchange( &gJobDesc.InAccess, 1 ) == 1 ; ) ;

			int newLeft = R3D_MAX( gJobDesc.Left - gJobDesc.ChunkSize, 0 );

			int count = gJobDesc.Left - newLeft ;

			gJobDesc.Left = newLeft ;

			gJobDesc.InAccess = 0 ;

			if( !count )
				break ;

			gJobDesc.Func( gJobDesc.Data, (size_t)newLeft, count );
		}
	}

	//------------------------------------------------------------------------

	JobDesc::JobDesc()
	: Func		( NULL )
	, Data		( NULL )
	, ChunkSize	( 0 )
	, Left		( 0 )
	, InAccess	( 0 )
	{

	}
}

//------------------------------------------------------------------------
