//=========================================================================
//	Module: r3dBackgroundTaskDispatcher.cpp
//	Copyright (C) 2011.
//=========================================================================

#include "r3dPCH.h"
#include "r3d.h"

#include "r3dDeviceQueue.h"

#include "r3dBackgroundTaskDispatcher.h"
#include "r3dDebug.h"

extern CRITICAL_SECTION g_ResourceCritSection ;

//////////////////////////////////////////////////////////////////////////

// g_pBackgroundTaskDispatcher will be NULL in other project that do not create it!
r3dBackgroundTaskDispatcher *g_pBackgroundTaskDispatcher = 0;

//////////////////////////////////////////////////////////////////////////

namespace
{

/**	Data passed to worker thread. */
struct ThreadData
{
	/**	Array with tasks. */
	r3dTL::TArray<r3dBackgroundTaskDispatcher::TaskDescriptor> *tasks;
	
	volatile LONG* taskCount ;

	/**	Start work event. */
	HANDLE startEvent;

	/**	Task array guard critical section. */
	CRITICAL_SECTION *taskGuard;

	ThreadData(): tasks(0), taskCount(0), startEvent(0), taskGuard(0) {}
};

//////////////////////////////////////////////////////////////////////////

/**	Worker function */
unsigned int WINAPI BackgroundTaskDispatcherThreadFunc(void* Par)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	ThreadData *data = reinterpret_cast<ThreadData*>(Par);

	while (true)
	{
		WaitForSingleObject(data->startEvent, INFINITE);
		EnterCriticalSection(data->taskGuard);
		//	No tasks, this is signal to terminate
		if (data->tasks->Count() == 0)
			break;
		//	Otherwise pick one task and proceed.
		r3dBackgroundTaskDispatcher::TaskDescriptor td( (*data->tasks)[ 0 ] );
		data->tasks->Erase( 0 ) ;
		LeaveCriticalSection(data->taskGuard);

		//	Exec work function
		if (td.Fn)
		{
			r3dCSHolder block( g_ResourceCritSection ) ;
			td.Fn(td.Params);
		}

		//	Indicate task completion.
		if (td.CompletionFlag)
			InterlockedExchange(td.CompletionFlag, 1L);

		if(td.Params)
			InterlockedExchange(&td.Params->Taken,0L);

		//	If there are more pending tasks in array, set event to signaled state to continue operation
		EnterCriticalSection(data->taskGuard);
		if (data->tasks->Count() > 0)
			SetEvent(data->startEvent);
		--*data->taskCount ;
		LeaveCriticalSection(data->taskGuard);
	}
	delete data;

	return 0;
}
} // unnamed namespace

//////////////////////////////////////////////////////////////////////////

r3dBackgroundTaskDispatcher::r3dBackgroundTaskDispatcher()
: m_ThreadHandle(0)
, m_StartEvent(0)
, m_TaskCount(0)
{
}

//////////////////////////////////////////////////////////////////////////

r3dBackgroundTaskDispatcher::~r3dBackgroundTaskDispatcher()
{

}

//------------------------------------------------------------------------

void
r3dBackgroundTaskDispatcher::Init()
{
	InitializeCriticalSection(&m_TaskCS);
	m_StartEvent = CreateEvent(0, FALSE, FALSE, 0);

	ThreadData *td = new ThreadData;
	td->taskGuard = &m_TaskCS;
	td->startEvent = m_StartEvent;
	td->taskCount = &m_TaskCount;
	td->tasks = &m_Tasks;
	m_ThreadHandle = reinterpret_cast<HANDLE>(_beginthreadex(NULL, 0, BackgroundTaskDispatcherThreadFunc, td, 0, NULL));
	if(m_ThreadHandle == NULL)
		r3dError("Failed to begin thread");

	m_TaskPtrParams.Init( 512 ) ;
}

//------------------------------------------------------------------------

void
r3dBackgroundTaskDispatcher::Close()
{
	//	Send termination signal to background thread.
	EnterCriticalSection(&m_TaskCS);
	m_Tasks.Clear();
	LeaveCriticalSection(&m_TaskCS);

	SetEvent(m_StartEvent);
	//	Wait for thread termination
	WaitForSingleObject(m_ThreadHandle, INFINITE);

	//	Close handles
	CloseHandle(m_ThreadHandle);
	CloseHandle(m_StartEvent);
	DeleteCriticalSection(&m_TaskCS);
}

//------------------------------------------------------------------------

void r3dBackgroundTaskDispatcher::AddTask(const r3dBackgroundTaskDispatcher::TaskDescriptor &td)
{
	EnterCriticalSection(&m_TaskCS);
	m_Tasks.PushBack(td);
	m_TaskCount++ ;
	LeaveCriticalSection(&m_TaskCS);
	SetEvent(m_StartEvent);
}

//------------------------------------------------------------------------

r3dTaskPtrParams*
r3dBackgroundTaskDispatcher::AllocPtrTaskParam()
{
	return m_TaskPtrParams.Alloc() ;
}

//------------------------------------------------------------------------

int
r3dBackgroundTaskDispatcher::GetTaskCount() const
{
	return m_TaskCount ;
}

//------------------------------------------------------------------------

void r3dFinishBackGroundTasks()
{
	if(g_pBackgroundTaskDispatcher == NULL)
		return;
		
	// wait for all background tasks to finish
	for( ; g_pBackgroundTaskDispatcher->GetTaskCount() ; )
	{
		// loading tasks may issude d3d commands into main thread
		ProcessDeviceQueue( r3dGetTime(), 1.0f ) ;
	}
}

//------------------------------------------------------------------------

void r3dTaskParamTimeSlice()
{
	ProcessDeviceQueue( r3dGetTime(), 0.1f ) ;
	Sleep( 1 ) ;
}

void r3dSetAsyncLoading( int onOff )
{
	if( onOff != g_async_loading->GetInt() )
	{
		r3dFinishBackGroundTasks() ;
		g_async_loading->SetInt( onOff ) ;
	}
}