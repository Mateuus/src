#include "r3dPCH.h"
#include "r3d.h"

#include "r3dBackgroundTaskDispatcher.h"

// no occlusion if server build
#ifdef WO_SERVER 
int g_UseHZB = 0;

#else //WO_SERVER// no server
int g_UseHZB = 0;

#endif //WO_SERVER

LPDIRECT3DVERTEXDECLARATION9 R3D_WORLD_VERTEX::pDecl = 0;
D3DVERTEXELEMENT9 R3D_WORLD_VERTEX::VBDecl[] = 
{
	{0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
	{0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0},
	{0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
	{0, 32, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT, 0},
	{0, 44, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BINORMAL, 0}, // from texcoord 3 to 6 instanced data in r3dObj.cpp for mesh instancing!
	D3DDECL_END()
};

LPDIRECT3DVERTEXDECLARATION9 R3D_WORLD_VERTEX::getDecl()
{
	if(pDecl == 0)
	{
		R3D_ENSURE_MAIN_THREAD();

		( r3dDeviceTunnel::CreateVertexDeclaration(VBDecl, &pDecl) );
	}
	return pDecl;
}

LPDIRECT3DVERTEXDECLARATION9 R3D_MESH_VERTEX::pDecl = 0;
const D3DVERTEXELEMENT9 R3D_MESH_VERTEX::VBDecl[] = 
{
	{ 0, 0,		D3DDECLTYPE_SHORT4N,	D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,	0 },
	{ 0, 8,		D3DDECLTYPE_SHORT2N,	D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,	0 },
	{ 0, 12,	D3DDECLTYPE_UBYTE4N,	D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,		0 },
	{ 0, 16,	D3DDECLTYPE_UBYTE4N,	D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT,	0 },
	D3DDECL_END()
};

const /*static*/ unsigned R3D_MESH_VERTEX::VBDeclElemCount = sizeof VBDecl / sizeof VBDecl[ 0 ];

LPDIRECT3DVERTEXDECLARATION9 R3D_MESH_VERTEX::getDecl()
{
	R3D_ENSURE_MAIN_THREAD();

	if(pDecl == 0)
	{
		( r3dDeviceTunnel::CreateVertexDeclaration(VBDecl, &pDecl) );
	}
	return pDecl;
}

//------------------------------------------------------------------------
/*static*/

LPDIRECT3DVERTEXDECLARATION9 R3D_BENDING_MESH_VERTEX::pDecl ;

/*static*/ const D3DVERTEXELEMENT9 R3D_BENDING_MESH_VERTEX::VBDecl[] =
{
	{ 0, 0,		D3DDECLTYPE_SHORT4N,	D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,	0 },
	{ 0, 8,		D3DDECLTYPE_SHORT2N,	D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,	0 },
	{ 0, 12,	D3DDECLTYPE_UBYTE4N,	D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,		0 },
	{ 0, 16,	D3DDECLTYPE_UBYTE4N,	D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT,	0 },
	{ 0, 20,	D3DDECLTYPE_UBYTE4N,	D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,	1 },
	D3DDECL_END()
} ;

const /*static*/ unsigned R3D_BENDING_MESH_VERTEX::VBDeclElemCount = sizeof VBDecl / sizeof VBDecl[ 0 ];

/*static*/
LPDIRECT3DVERTEXDECLARATION9 R3D_BENDING_MESH_VERTEX::getDecl()
{
	R3D_ENSURE_MAIN_THREAD();

	if(pDecl == 0)
	{
		( r3dDeviceTunnel::CreateVertexDeclaration(VBDecl, &pDecl) );
	}

	return pDecl;
}

typedef USHORT ( WINAPI * CaptureStackBackTraceFunc )( ULONG, ULONG, PVOID*, PULONG ) ;

CaptureStackBackTraceFunc g_CaptureStackBackTrace ;

//------------------------------------------------------------------------
/*static*/

LPDIRECT3DVERTEXDECLARATION9 R3D_MESH_PRECISE_VERTEX::pDecl;

//------------------------------------------------------------------------
/*static*/

D3DVERTEXELEMENT9 R3D_MESH_PRECISE_VERTEX::VBDecl[] = 
{
	{ 0, 0,		D3DDECLTYPE_FLOAT3,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,	0 },
	{ 0, 12,	D3DDECLTYPE_FLOAT2,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,	0 },
	{ 0, 20,	D3DDECLTYPE_UBYTE4N,	D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,		0 },
	{ 0, 24,	D3DDECLTYPE_UBYTE4N,	D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT,	0 },
	D3DDECL_END()
};

//------------------------------------------------------------------------
/*static*/

LPDIRECT3DVERTEXDECLARATION9 R3D_MESH_PRECISE_VERTEX::getDecl()
{
	R3D_ENSURE_MAIN_THREAD();
	if(pDecl == 0)
	{
		( r3dDeviceTunnel::CreateVertexDeclaration( VBDecl, &pDecl ) );
	}
	return pDecl;
}

//------------------------------------------------------------------------

LPDIRECT3DVERTEXDECLARATION9 R3D_DEBUG_VERTEX::pDecl = 0;
D3DVERTEXELEMENT9 R3D_DEBUG_VERTEX::VBDecl[] = 
{
	{0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
	{0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,   0},
	{0, 16, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
	{0, 24, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
	D3DDECL_END()
};

LPDIRECT3DVERTEXDECLARATION9 R3D_DEBUG_VERTEX::getDecl()
{
	R3D_ENSURE_MAIN_THREAD();

	if(pDecl == 0)
	{
		( r3dDeviceTunnel::CreateVertexDeclaration(VBDecl, &pDecl) );
	}
	return pDecl;
}

void R3D_DEBUG_VERTEX::ComputeNormals(R3D_DEBUG_VERTEX* v, int count)
{
	r3d_assert(count == 3 || count == 4);

	r3dPoint3D ac = v[2].Pos - v[0].Pos;
	r3dPoint3D ab = v[1].Pos - v[0].Pos;
	r3dPoint3D cp = ab.Cross(ac);
	cp.Normalize();
	
	for (int i = 0; i < count; ++i)
	{
		v[i].Normal = cp;
	}
}


LPDIRECT3DVERTEXDECLARATION9 R3D_SIMPLEWORLD_VERTEX::pDecl = 0;
D3DVERTEXELEMENT9 R3D_SIMPLEWORLD_VERTEX::VBDecl[] = 
{
	{0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
	{0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0},
	{0, 24, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,	  0},
	{0, 28, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
	D3DDECL_END()
};

LPDIRECT3DVERTEXDECLARATION9 R3D_SIMPLEWORLD_VERTEX::getDecl()
{
	if(pDecl == 0)
	{
		( r3dDeviceTunnel::CreateVertexDeclaration(VBDecl, &pDecl) );
	}
	return pDecl;
}

LPDIRECT3DVERTEXDECLARATION9 R3D_SCREEN_VERTEX::pDecl = 0;
D3DVERTEXELEMENT9 R3D_SCREEN_VERTEX::VBDecl[] = 
{
	{0,  0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITIONT, 0},
	{0, 16, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,	  0},
	{0, 20, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,	  1},
	{0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
	{0, 32, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1},
	{0, 40, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 2},
	{0, 48, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 3},
	D3DDECL_END()
};

LPDIRECT3DVERTEXDECLARATION9 R3D_SCREEN_VERTEX::getDecl()
{
	if(pDecl == 0)
	{
		( r3dDeviceTunnel::CreateVertexDeclaration(VBDecl, &pDecl) );
	}
	return pDecl;
}

//------------------------------------------------------------------------
/*static*/

/*static*/ D3DVERTEXELEMENT9 R3D_MINIMALIST_VERTEX_2D::VBDecl[] = 
{
	{ 0, 0, D3DDECLTYPE_SHORT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
	D3DDECL_END()
};

/*static*/ LPDIRECT3DVERTEXDECLARATION9	R3D_MINIMALIST_VERTEX_2D::pDecl = 0 ;

LPDIRECT3DVERTEXDECLARATION9 R3D_MINIMALIST_VERTEX_2D::getDecl()
{
	if(pDecl == 0)
	{
		( r3dDeviceTunnel::CreateVertexDeclaration(VBDecl, &pDecl) );
	}
	return pDecl;
}

//------------------------------------------------------------------------

LPDIRECT3DVERTEXDECLARATION9 R3D_POS_VERTEX::pDecl = 0;
D3DVERTEXELEMENT9 R3D_POS_VERTEX::VBDecl[] = 
{
	{0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
	D3DDECL_END()
};

LPDIRECT3DVERTEXDECLARATION9 R3D_POS_VERTEX::getDecl()
{
	if(pDecl == 0)
	{
		( r3dDeviceTunnel::CreateVertexDeclaration(VBDecl, &pDecl) );
	}
	return pDecl;
}

r3dRenderLayer	*r3dRenderer    = NULL;
r3dMouse	*Mouse		= NULL;
r3dKeyboard	*Keyboard	= NULL;
r3dGamepad	*Gamepad	= NULL;
r3dInputMappingMngr* InputMappingMngr = NULL;

HANDLE		r3d_CurrentProcess;
HANDLE		r3d_MainThread;

	int		_r3d_bLogToDebug   = 1;
	int		_r3d_bLogToConsole = 0;
	int		_r3d_bPrintCallStackInit = 0;
static	int		_r3d_logInited     = 0;
static  int     _r3d_art_log_inited = 0 ;
static	int		_r3d_logOpened     = 0;
static	FILE*		_r3d_logFile       = NULL;
	int		_r3d_iLogIndent    = 0;
	int		_r3d_iLogSizeIndent= 1;

	int		_r3d_bTerminateOnZ = 1;

	float		_srv_World_Scale     = 1.0f;

  // limit by N fps, also this is minimum value for zero or negative time frames
const	float		_r3d_FrameLimiter    = (1.0f / 4000.0f);

	float		_r3d_StartFrameTime  = 0.0f;
static	float		_r3d_EndFrameTime    = 0.0f + _r3d_FrameLimiter;
static	float		_r3d_LastFrameTime   = _r3d_FrameLimiter;
static  float		_r3d_AverageFrameTimes[30] = {0};
static  int			_r3d_AverageFrameTimeCounter = 0;
static float		_r3d_AverageFrameTime = _r3d_FrameLimiter;

	
//
// press SHIFT-CONTROL-Z for thread termination..
//
void r3d_UtilityThread(DWORD in)
{
  while(1) {
    Sleep(100);

    int bShift = (GetAsyncKeyState(VK_SHIFT)&0x8000);
    int bCntrl = (GetAsyncKeyState(VK_CONTROL)&0x8000);

    if(_r3d_bTerminateOnZ && bShift && bCntrl && (GetAsyncKeyState('Z')&0x8000)) {
      r3dOutToLog("s-c-Z\n");
      ClipCursor(NULL);
      HRESULT res = TerminateProcess(r3d_CurrentProcess, 0);
    }
  }
  
  return;
}

static const BYTE UTF8_BOM[3] = { 0xEF, 0xBB, 0xBF };

void r3dInitLogFile()
{
	time_t 	t;
	FILE	*f;

  time(&t);
  if((f = fopen("r3dlog.txt", "wt")) != NULL) {
    //fwrite(UTF8_BOM, 1, 3, f);
    fprintf(f, "--- Log started at %s\n", ctime(&t));
    fclose(f);
  }
  
  return;
}

void r3dChangeLogFile(const char* fname)
{
  r3dOutToLog("Changing log to %s\n", fname);
  
  if(_r3d_logOpened) {
    fclose(_r3d_logFile);
  }
  
  _r3d_logFile = fopen(fname, "wt");
  //if(_r3d_logFile) {
  //  fwrite(UTF8_BOM, 1, 3, _r3d_logFile);
  //}
}

void r3dCloseLogFile()
{
  if(_r3d_logFile) 
  {
    fclose(_r3d_logFile);
    _r3d_logOpened = 0;
    _r3d_logFile   = NULL;
  }
}

bool r3dOutToLog(const char* Str, ...)
{
  static CRITICAL_SECTION _cs_log;

  if(!_r3d_logInited) {
    _r3d_logInited = 1;

    InitializeCriticalSectionAndSpinCount(&_cs_log, 4000);

    r3dInitLogFile();
  }

  r3dCSHolder csHolder(_cs_log);

  // buffer with indented spaces
  char buf[4096];
  int ioff = _r3d_iLogIndent*_r3d_iLogSizeIndent;	// indent offset
  r3d_assert(ioff < 500);
  for(int i=0; i<ioff; i++) buf[i] = ' ';

  va_list ap;
  va_start(ap, Str);
  StringCbVPrintfA(buf + ioff, sizeof(buf) - ioff, Str, ap);
  va_end(ap);

  if(!_r3d_logOpened) 
  {
    _r3d_logOpened = 1;
    _r3d_logFile   = fopen("r3dlog.txt", "at");
  }

  if(_r3d_logFile) 
  {
    fprintf(_r3d_logFile, "%06d.%03d| %s", int(r3dGetTime()), (int(r3dGetTime() * 1000) % 1000), buf);
    //fprintf(_r3d_logFile, "%s", buf);
    fflush(_r3d_logFile);
  }

  if(_r3d_bLogToDebug)
  {
    OutputDebugString(buf);
  }

  if(_r3d_bLogToConsole)
  {
    printf(buf);
  }

  return true;
}

static r3dTL::TArray< r3dString > gs_Art_Bugs ;
static r3dString gs_Art_Bug_Comment ;

extern bool r3dArtBug(const char* Str, ...)
{
  static CRITICAL_SECTION cs;

  if(!_r3d_art_log_inited) {
    _r3d_art_log_inited = 1;

    InitializeCriticalSectionAndSpinCount(&cs, 4000);
  }

  r3dCSHolder csHolder(cs);

  // buffer with indented spaces
  char buf[4096];

  va_list ap;
  va_start(ap, Str);
  StringCbVPrintfA(buf, sizeof(buf), Str, ap);
  va_end(ap);

  if( gs_Art_Bug_Comment.Length() > 0 )
  {
    r3dOutToLog( "%s (%s)\n", buf, gs_Art_Bug_Comment.c_str() ) ;
  }
  else
  {
    r3dOutToLog( "%s", buf ) ;
  }

#ifndef FINAL_BUILD
  char buf2[ 8192 ] ;
  if( gs_Art_Bug_Comment.Length() > 0 )
  {
    sprintf( buf2, "%s (%s)\n", buf, gs_Art_Bug_Comment.c_str() ) ;
    gs_Art_Bugs.PushBack( buf2 ) ;
  }
  else
  {
    gs_Art_Bugs.PushBack( buf ) ;
  }
#endif

  return true;
}

void r3dArtBugComment( const char* Str )
{
	if( Str )
		gs_Art_Bug_Comment = Str ;
	else
		gs_Art_Bug_Comment = "" ;
}

void r3dPurgeArtBugs()
{
	gs_Art_Bugs.Clear() ;
}

void r3dShowArtBugs()
{
#ifndef FINAL_BUILD
	r3dString uberBug ;

	for( uint32_t i = 0, e = gs_Art_Bugs.Count() ; i < e; i ++ )
	{
		uberBug += gs_Art_Bugs[ i ] ;
	}

	if(strlen(uberBug.c_str())>3)
		MessageBoxA( r3dRenderer->HLibWin, uberBug.c_str(), "Art Bugs", MB_OK ) ;
#endif
}

void r3dSetFiltering( r3dFilter Filter, int Stage )
{
 //r3dRenderer->Flush();

 int FMode;
 int FMagMode;
 int MipFMode;

 switch( Filter )
 {
 case R3D_POINT:
   FMagMode = FMode    = D3DTEXF_POINT;
   MipFMode = D3DTEXF_POINT;
   break;
 case R3D_BILINEAR:
   FMagMode = FMode    = D3DTEXF_LINEAR;
   MipFMode = D3DTEXF_LINEAR;
   break;
 case R3D_ANISOTROPIC:
	 FMagMode = D3DTEXF_LINEAR;
   if( r_anisotropy->GetInt() > 1 )
   {
    FMode   = D3DTEXF_ANISOTROPIC;
   }
   else
   {
    FMode   = D3DTEXF_LINEAR;
   }
   // otherwise you get D3D error
   MipFMode = D3DTEXF_LINEAR;
   break;
 }

 if (Stage <0)
 {
  for (int i=0;i<8;i++)
  {
   r3dRenderer->pd3ddev->SetSamplerState(i, D3DSAMP_MAGFILTER, FMagMode );
   r3dRenderer->pd3ddev->SetSamplerState(i, D3DSAMP_MINFILTER, FMode );
   r3dRenderer->pd3ddev->SetSamplerState(i, D3DSAMP_MIPFILTER, MipFMode );

  }
 }
 else
  {
   r3dRenderer->pd3ddev->SetSamplerState(Stage, D3DSAMP_MAGFILTER, FMagMode );
   r3dRenderer->pd3ddev->SetSamplerState(Stage, D3DSAMP_MINFILTER, FMode );
   r3dRenderer->pd3ddev->SetSamplerState(Stage, D3DSAMP_MIPFILTER, MipFMode );
  }
}

void r3dSetMaxAnisotropy( int value, int stage )
{
	value = R3D_MIN( value, (int)r3dRenderer->d3dCaps.MaxAnisotropy ) ;

	D3D_V( r3dRenderer->pd3ddev->SetSamplerState( stage, D3DSAMP_MAXANISOTROPY, value ) );
}

void r3dSetAnisotropy( int value, int stage )
{
	r3dSetMaxAnisotropy( value, stage ) ;
	
	D3D_V( r3dRenderer->pd3ddev->SetSamplerState( stage, D3DSAMP_MAGFILTER,D3DTEXF_LINEAR ) ) ;
	D3D_V( r3dRenderer->pd3ddev->SetSamplerState( stage, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR ) ) ;

	if( value > 1 )
	{
		D3D_V( r3dRenderer->pd3ddev->SetSamplerState( stage, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC ) ) ;
	}
	else
	{
		D3D_V( r3dRenderer->pd3ddev->SetSamplerState( stage, D3DSAMP_MINFILTER, D3DTEXF_LINEAR ) ) ;
	}
}

void r3dSetDefaultMaxAnisotropy()
{
	for( UINT i = 0; i < 16; i ++ )
	{
		r3dSetMaxAnisotropy( r_anisotropy->GetInt(), i ) ;
	}
}

int r3dToggleScreenShot()
{
	time_t curtime;
	time(&curtime);
	struct tm curdate = *localtime(&curtime);

	char			FileName[128];
#ifndef FINAL_BUILD
	sprintf(FileName, "Screen_%02d%02d%04d_%02d%02d%02d.tga", curdate.tm_mday, curdate.tm_mon, 1900+curdate.tm_year, curdate.tm_hour, curdate.tm_min, curdate.tm_sec);
#else
	sprintf(FileName, "Screen_%02d%02d%04d_%02d%02d%02d.jpg", curdate.tm_mday, curdate.tm_mon, 1900+curdate.tm_year, curdate.tm_hour, curdate.tm_min, curdate.tm_sec);
#endif

	r_screenshot_name->SetString( FileName );
	r_do_screenshot->SetInt( 1 );

	if( r_allow_depth_screenshot->GetInt() )
	{
		char DFileName[128];
		sprintf(DFileName, "Screen_%02d%02d%04d_%02d%02d%02d_depth.tga", curdate.tm_mday, curdate.tm_mon, 1900+curdate.tm_year, curdate.tm_hour, curdate.tm_min, curdate.tm_sec);

		r_depth_screenshot_name->SetString( DFileName );
		r_do_depth_screenshot->SetInt( 1 );
	}

	return 1;
}

IDirect3DTexture9* _r3d_screenshot_copy = NULL;	// saved screenshot copy
int _r3d_sscopy_width  = 0;	// expected width of screenshot copy
int _r3d_sscopy_height = 0;	// expected height of screenshot copy

//////////////////////////////////////////////////////////////////////////

struct ScreenShotSaveTaskParams: public r3dTaskParams
{
	char *fileName;
	ID3DXBuffer *buf;
	
	ScreenShotSaveTaskParams()
	: fileName(0)
	, buf(0)
	{}

	~ScreenShotSaveTaskParams()
	{
		free(fileName);
		SAFE_RELEASE(buf);
	}
};

//////////////////////////////////////////////////////////////////////////

void ScreenShotSaver(r3dTaskParams *tp)
{
	ScreenShotSaveTaskParams *sstp = static_cast<ScreenShotSaveTaskParams*>(tp);
	if (sstp && sstp->buf && sstp->fileName)
	{
		FILE *f = fopen_for_write(sstp->fileName, "wb");
		fwrite(sstp->buf->GetBufferPointer(), 1, sstp->buf->GetBufferSize(), f);
		fclose(f);
	}
}

//////////////////////////////////////////////////////////////////////////

int r3dUpdateScreenShot()
{
	if( r_do_screenshot->GetInt() )
	{
		r_do_screenshot->SetInt( 0 ); 

		IDirect3DSurface9 *BBuf = 0;

		IDirect3DDevice9 *d = r3dRenderer->pd3ddev;

		r3dRenderer->GetRT(0, &BBuf);

		IDirect3DSurface9 *sysmemBB = r3dRenderer->GetTempSurfaceForScreenShots();

		if (!SUCCEEDED(d->GetRenderTargetData(BBuf, sysmemBB)))
			return 0;
		
		static r3dTaskParramsArray< ScreenShotSaveTaskParams > tpArr ;
		if (tpArr.MaxElemsCount() == 0)
		{
			tpArr.Init(8);
		}

		ScreenShotSaveTaskParams *tp = tpArr.Alloc();

		tp->fileName = strdup(r_screenshot_name->GetString());

		r3dBackgroundTaskDispatcher::TaskDescriptor td;
		td.Fn = &ScreenShotSaver;
		td.Params = tp;

#ifndef FINAL_BUILD
		D3DXSaveSurfaceToFileInMemory(&tp->buf, D3DXIFF_TGA, sysmemBB, NULL, NULL);
#else
		D3DXSaveSurfaceToFileInMemory(&tp->buf, D3DXIFF_JPG, sysmemBB, NULL, NULL);
#endif
		g_pBackgroundTaskDispatcher->AddTask(td);

		BBuf->Release();
	}
	
	// if requested, make scaled copy of screenshot
	if( _r3d_sscopy_width > 0)
	{
		SAFE_RELEASE(_r3d_screenshot_copy);

		IDirect3DSurface9* BBuf ;
		r3dRenderer->GetRT( 0, &BBuf) ;

		HRESULT hr = r3dRenderer->pd3ddev->CreateTexture(_r3d_sscopy_width, _r3d_sscopy_height, 0, 0, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &_r3d_screenshot_copy, NULL) ;

		r3dRenderer->CheckOutOfMemory( hr ) ;

		if(D3D_OK == hr )
		{
			IDirect3DSurface9* pSurf0 = NULL;
			_r3d_screenshot_copy->GetSurfaceLevel(0, &pSurf0);
			D3DXLoadSurfaceFromSurface(pSurf0, NULL, NULL, BBuf, NULL, NULL, D3DX_DEFAULT, 0);
			SAFE_RELEASE(pSurf0);
		}
		BBuf->Release();

		_r3d_sscopy_width  = 0;
		_r3d_sscopy_height = 0;
	}

	return 1;
}



// Name: VectortoRGBA()
// Desc: Turns a normalized vector into RGBA form. Used to encode vectors into
//       a height map. 
//-----------------------------------------------------------------------------
DWORD r3dVectortoRGBA( r3dVector *v, FLOAT fHeight )
{
    DWORD r = (DWORD)( 127.0f * v->X + 128.0f );
    DWORD g = (DWORD)( 127.0f * v->Y + 128.0f );
    DWORD b = (DWORD)( 127.0f * v->Z + 128.0f );
    DWORD a = (DWORD)( 255.0f * fHeight );
    
    return( (a<<24L) + (r<<16L) + (g<<8L) + (b<<0L) );
}




// Create Texture Transformation Matrix for automatic UV generation from world coordinates
//  U = World_X * ScaleX
//  V = World_Z * ScaleZ
// TextureStage should be set as following:
void r3dCreateWorldTexGenMatrix(D3DXMATRIX *mR, float ScaleX, float ScaleZ)
{
  D3DXMATRIX m1, m2, m3, m4;

  // inverse view matrix

  m1 = r3dRenderer->ViewMatrix ;

  D3DXMatrixInverse(&m1, NULL, &m1);
  // swap yz matrix
  m2._11 = 1;  m2._12 = 0;  m2._13 = 0;  m2._14 = 0;
  m2._21 = 0;  m2._22 = 0;  m2._23 = 1;  m2._24 = 0;
  m2._31 = 0;  m2._32 = 1;  m2._33 = 0;  m2._34 = 0;
  m2._41 = 0;  m2._42 = 0;  m2._43 = 0;  m2._44 = 1;
  // scale matrix
  D3DXMatrixScaling(&m3, ScaleX, ScaleZ, ScaleZ);

  *mR = m1 * m2 * m3;
  return;
}
		    
void r3dDrawNormQuad(r3dCamera &Cam, const r3dPoint3D& Pos, const r3dPoint3D& Norm, float Size, const r3dColor24& col)
{
 r3dPoint3D p1,p2;

 R3D_DEBUG_VERTEX V[4];

 if(Norm.x != Norm.y || Norm.x != Norm.z )
   p1.Assign( Norm.y - Norm.z, Norm.z - Norm.x, Norm.x - Norm.y );
 else
   p1.Assign( Norm.y + Norm.z, Norm.z - Norm.x, -Norm.x - Norm.y );

 p1 *= Size * R3D_SQRT_2 / p1.Length();
 p2 = Norm.Cross(p1);

 V[0].Pos = Pos + p1;
 V[1].Pos = Pos + p2;
 V[2].Pos = Pos - p1;
 V[3].Pos = Pos - p2;

 V[0].color = col;
 V[1].color = col;
 V[2].color = col;
 V[3].color = col;


 V[0].tu   = 1; V[0].tv = 1;
 V[1].tu   = 0; V[1].tv = 1;
 V[2].tu   = 0; V[2].tv = 0;
 V[3].tu   = 1; V[3].tv = 0;

 R3D_DEBUG_VERTEX::ComputeNormals(V, 4);

 r3dRenderer->Render3DPolygon(4, V);
}

	
void r3dSetWireframe(int bOn)
{
  r3dRenderer->Flush();
  r3dRenderer->pd3ddev->SetRenderState(D3DRS_FILLMODE, bOn ? D3DFILL_WIREFRAME : D3DFILL_SOLID);
}

int r3dGetPitchHeight( int count, D3DFORMAT fmt )
{
	switch( fmt )
	{
	case D3DFMT_DXT1:
	case D3DFMT_DXT2:
	case D3DFMT_DXT3:
	case D3DFMT_DXT4:
	case D3DFMT_DXT5:
		return R3D_MAX( count / 4, 1 );
	default:
		return count;
	}
}

int r3dGetPixelSize( int count, D3DFORMAT fmt )
{
	switch( fmt )
	{
	case D3DFMT_R8G8B8               : return count * 3 ;
	case D3DFMT_A8R8G8B8             : return count * 4 ;
	case D3DFMT_X8R8G8B8             : return count * 4 ;
	case D3DFMT_R5G6B5               : return count * 2 ;
	case D3DFMT_X1R5G5B5             : return count * 2 ;
	case D3DFMT_A1R5G5B5             : return count * 2 ;
	case D3DFMT_A4R4G4B4             : return count * 2 ;
	case D3DFMT_R3G3B2               : return count * 1 ;
	case D3DFMT_A8                   : return count * 1 ;
	case D3DFMT_A8R3G3B2             : return count * 2 ;
	case D3DFMT_X4R4G4B4             : return count * 2 ;
	case D3DFMT_A2B10G10R10          : return count * 4 ;
	case D3DFMT_A8B8G8R8             : return count * 4 ;
	case D3DFMT_X8B8G8R8             : return count * 4 ;
	case D3DFMT_G16R16               : return count * 4 ;
	case D3DFMT_A2R10G10B10          : return count * 4 ;
	case D3DFMT_A16B16G16R16         : return count * 8 ;
	case D3DFMT_A8P8                 : return count * 2 ;
	case D3DFMT_P8                   : return count * 1 ;
	case D3DFMT_L8                   : return count * 1 ;
	case D3DFMT_A8L8                 : return count * 2 ;
	case D3DFMT_A4L4                 : return count * 1 ;
	case D3DFMT_V8U8                 : return count * 2 ;
	case D3DFMT_L6V5U5               : return count * 2 ;
	case D3DFMT_X8L8V8U8             : return count * 4 ;
	case D3DFMT_Q8W8V8U8             : return count * 4 ;
	case D3DFMT_V16U16               : return count * 4 ;
	case D3DFMT_A2W10V10U10          : return count * 4 ;
	case D3DFMT_UYVY                 : return count * 4 ;
	case D3DFMT_R8G8_B8G8            : return count * 2 ;
	case D3DFMT_YUY2                 : return count * 2 ;
	case D3DFMT_G8R8_G8B8            : return count * 2 ;
	case D3DFMT_DXT1                 : return count / 2 ;
	case D3DFMT_DXT2                 : return count * 1 ;
	case D3DFMT_DXT3                 : return count * 1 ;
	case D3DFMT_DXT4                 : return count * 1 ;
	case D3DFMT_DXT5                 : return count * 1 ;
	case D3DFMT_D16_LOCKABLE         : return count * 2 ;
	case D3DFMT_D32                  : return count * 4 ;
	case D3DFMT_D15S1                : return count * 2 ;
	case D3DFMT_D24S8                : return count * 4 ;
	case D3DFMT_D24X8                : return count * 4 ;
	case D3DFMT_D24X4S4              : return count * 4 ;
	case D3DFMT_D16                  : return count * 2 ;
	case D3DFMT_D32F_LOCKABLE        : return count * 4 ;
	case D3DFMT_D24FS8               : return count * 4 ;
	case D3DFMT_D32_LOCKABLE         : return count * 4 ;
	case D3DFMT_S8_LOCKABLE          : return count * 1 ;
	case D3DFMT_L16                  : return count * 2 ;
	case D3DFMT_INDEX16              : return count * 2 ;
	case D3DFMT_INDEX32              : return count * 4 ;
	case D3DFMT_Q16W16V16U16         : return count * 8 ;
	case D3DFMT_MULTI2_ARGB8         : return count * 4 ;
	case D3DFMT_R16F                 : return count * 2 ;
	case D3DFMT_G16R16F              : return count * 4 ;
	case D3DFMT_A16B16G16R16F        : return count * 8 ;
	case D3DFMT_R32F                 : return count * 4 ;
	case D3DFMT_G32R32F              : return count * 8 ;
	case D3DFMT_A32B32G32R32F        : return count * 16 ;
	case D3DFMT_CxV8U8               : return count * 2 ;
	case D3DFMT_A1                   : return count / 8 ;
	case D3DFMT_A2B10G10R10_XR_BIAS  : return count * 4 ;
	case INTZ_FORMAT				 : return count * 4 ;
	case DF24_FORMAT				 : return count * 4 ;
	case NULL_RT_FORMAT				 : return 0;
	case D3DFMT_UNKNOWN				 : return 0;
	default:
		r3d_assert( false );
		return 0;
	}
}

static int	     _TimerInited = 0;
static double	     _fTicksPerSec;
static LARGE_INTEGER _llTimerStart;

float r3dGetTime()
{
 if (!_TimerInited)
 {
   _TimerInited = 1;

   // Use QueryPerformanceFrequency() to get frequency of timer.  
   LARGE_INTEGER qwTicksPerSec;
   QueryPerformanceFrequency( &qwTicksPerSec );
   _fTicksPerSec = (double)qwTicksPerSec.QuadPart;
   QueryPerformanceCounter(&_llTimerStart);
 }

 LARGE_INTEGER qwTime;
 QueryPerformanceCounter(&qwTime);

 return float((double)(qwTime.QuadPart - _llTimerStart.QuadPart) / _fTicksPerSec);
}

float r3dGetFrameTime()
{
  return _r3d_LastFrameTime;
}

float r3dGetFrameTime2()
{
	return R3D_MIN(_r3d_LastFrameTime, 0.1f); // limit delta to 10fps
}

float r3dGetAveragedFrameTime()
{
	return R3D_MIN(_r3d_AverageFrameTime, 0.1f);
}

void r3dStartFrame()
{
	if(r3dRenderer && r3dRenderer->DeviceAvailable)
		r3dRenderer->ResetQueryCounters();

  _r3d_StartFrameTime = r3dGetTime();

#if !DISABLE_PROFILER
  if(r3dProfiler::Instance())
	  r3dProfiler::Instance()->StartFrame();
#endif

  r3dMaterial::ResetMaterialFilter();
}

static double FPSSum = 0;
static int FPSCount = 0;
static float lastFPSTime = 0.0f;
static float AVGFps = 0.f;

void r3dEndFrame()
{
#if !DISABLE_PROFILER
  if(r3dProfiler::Instance())
    r3dProfiler::Instance()->EndFrame();
#endif

  _r3d_EndFrameTime  = r3dGetTime();
  _r3d_LastFrameTime = _r3d_EndFrameTime - _r3d_StartFrameTime;
  _r3d_AverageFrameTimes[_r3d_AverageFrameTimeCounter] = _r3d_LastFrameTime;
  _r3d_AverageFrameTimeCounter = (_r3d_AverageFrameTimeCounter+1)%30;
  _r3d_AverageFrameTime = 0;
  for(int i=0; i<30; ++i)
	  _r3d_AverageFrameTime += _r3d_AverageFrameTimes[i];
  _r3d_AverageFrameTime /= 30.0f;

#ifdef _DEBUG
  if(_r3d_LastFrameTime > 1.0f) {
    r3dOutToLog("!!!warning!!! frame took longer that 1 sec.\n");
  }
#endif

  // check for very small frame times
  // and also check for negative frames (it can happens if time was readed from other CPU.)
  //  in that case we just report minimal frametime
  if(_r3d_LastFrameTime < _r3d_FrameLimiter) 
    _r3d_LastFrameTime = _r3d_FrameLimiter;
  if(_r3d_AverageFrameTime < _r3d_FrameLimiter) 
	  _r3d_AverageFrameTime = _r3d_FrameLimiter;
 
//  if( r_avarage_fps->GetInt() )
  {
	  float newTime = r3dGetTime();
	  if( newTime - lastFPSTime > 0.125f )
	  {
		  AVGFps = float(FPSSum / FPSCount);
		  lastFPSTime = newTime;
		  FPSCount = 0;
		  FPSSum = 0;
	  }

	  FPSSum += 1.f / r3dGetFrameTime();
	  FPSCount ++;
  }
//  else
//  {
// 	  AVGFps = 1.f / r3dGetFrameTime();
//   }
}

void r3dResetFrameTime()
{
  _r3d_StartFrameTime = r3dGetTime();
  _r3d_LastFrameTime  = _r3d_FrameLimiter;
  _r3d_AverageFrameTime = _r3d_FrameLimiter;
}


float r3dGetAvgFPS()
{
	return AVGFps;
}


int _r3d_bSilent_r3dError = 0;

static std::vector< const char* > ExtraErrorInfo ;

void r3dPushExtralErrorInfo( const char* info )
{
	ExtraErrorInfo.push_back( info );
}

void r3dPopExtraErrorInfo()
{
	ExtraErrorInfo.pop_back();
}

const char* r3dError(const char* fmt, ...)
{
	static char buf[4096];

	va_list ap;
	va_start(ap, fmt);
	StringCbVPrintfA(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	
	if(_r3d_bSilent_r3dError)
	{
		r3dOutToLog("!!! ERROR: %s\n", buf);
		// note: let server generate minidump on r3dError
		*(DWORD*)0 = 0;
		throw buf;
	}

	r3dOutToLog("!!! ERROR: %s\n", buf);

	for( size_t i = 0, e = ExtraErrorInfo.size(); i < e; i ++ )
	{
		r3dOutToLog( "Additional ERROR info: %s\n", ExtraErrorInfo[ i ] );
	}

	r3dOutToLog("... exiting\n");

	ShowWindow(win::hWnd, FALSE); // hide window, otherwise in fullscreen it's impossible to debug
	if(IsDebuggerPresent()) 
		__asm int 3;
	else 
	{
		// please do not remove this!!! Otherwise it will just silently crash
		MessageBox(0, buf, "Error!", MB_OK);
	}

	// do crash, do current handler will fire
	*(DWORD*)0 = 0;
	throw buf;
}

#ifdef  __cplusplus
extern "C" {
#endif
_CRTIMP void __cdecl _wassert(_In_z_ const wchar_t * _Message, _In_z_ const wchar_t *_File, _In_ unsigned _Line);
_CRTIMP void __cdecl _assert(_In_z_ const char * _Message, _In_z_ const char *_File, _In_ unsigned _Line);
#ifdef  __cplusplus
}
#endif

void r3d_actual_assert(const char *expr, const char *filename, unsigned lineno, bool crash)
{
  r3dOutToLog("!!! r3d_assert '%s' at %s line %d\n", expr, filename, lineno);
  if(crash) {
    *(DWORD*)NULL = 0;
  } else {
    _assert(expr, filename, lineno);
  }
}

float r3dRandomFloat()
{
 return float(random(1000) - 500)/500.0f;
}


r3dVector r3dMakeAOVector()
{
 r3dVector V;
 r3dVector vUp = r3dVector(0,1,0);

 while (true)
 {
  float x = r3dRandomFloat();
  float y = r3dRandomFloat();
  float z = r3dRandomFloat();

  if (x*x+y*y+z*z >1) continue;
  
  V.Assign(x,y,z); 
  if (V.Dot(vUp) < 0 ) continue;

  V.Normalize();
  return V;
 }
}

	float 		u_GetRandom();
	float 		u_GetRandom(float r1, float r2);

r3dPoint3D r3dGetConfusionPosition(const r3dPoint3D& P, float Confusion)
{
  r3dPoint3D Temp;

  Temp    = P;
  Temp.X += u_GetRandom(-Confusion, Confusion);
  Temp.Z += u_GetRandom(-Confusion, Confusion);
 
  return Temp;
}

r3dPoint3D r3dGetRingConfusionPosition(const r3dPoint3D& P, float Confusion)
{
  r3dPoint3D Temp;

  float Angle = (float)random(360);
  float cosx  = r3dFastCos(Angle);
  float siny  = r3dFastSin(Angle);

  Temp    = P;
  Temp.X += Confusion * cosx;
  Temp.Z += Confusion * siny;
 
  return Temp;
}


r3dDevStrength r3dGetDeviceStrength()
{
	D3DADAPTER_IDENTIFIER9 ident;

	if( r3dRenderer )
	{
		D3D_V( r3dRenderer->pd3d->GetAdapterIdentifier( 0, 0, &ident ) );
	}
	else
	{
		IDirect3D9* d3d = Direct3DCreate9( D3D_SDK_VERSION );
		r3d_assert( d3d );
		D3D_V( d3d->GetAdapterIdentifier( 0, 0, &ident ) );
		SAFE_RELEASE( d3d );
	}


	switch( ident.VendorId )
	{
		// ATI/AMD
	case 0x1002:
		switch( ident.DeviceId )
		{
			// ATI Radeon HD 2350 Series
			// ATI Radeon HD 2400 Series
			// ATI Radeon HD 3410
		case 0x94C3: 
			return S_WEAK;
			// ATI Radeon HD 2350
		case 0x94C7:
			return S_WEAK;			
			// ATI Radeon HD 2400 XT
		case 0x94C1:
			return S_WEAK;
			// ATI Radeon HD 2400 Series
		case 0x94CC:
			return S_WEAK;
			// ATI Radeon HD 2600 X2 Series
		case 0x958A:
			return S_MEDIUM;
			// ATI Radeon HD 2600 XT
		case 0x9588:
			return S_MEDIUM;
			// ATI Radeon HD 2900 GT
		case 0x9405:
			return S_MEDIUM;
			// ATI Radeon HD 2900 XT
		case 0x9400:
			return S_MEDIUM;
			// ATI Radeon 3100 Graphics
		case 0x9611:
		case 0x9615:
			return S_WEAK;
			// ATI Radeon HD 3200 Graphics
		case 0x9610:
			return S_WEAK;
			// ATI Radeon HD 3300 Graphics
		case 0x9614:
			return S_WEAK;
			// ATI Radeon HD 3400 Series
			// ATI Radeon HD 3550
			// ATI Radeon HD 3570
			// ATI Radeon HD 4250
		case 0x95C0:
			return S_WEAK;
			// ATI Radeon HD 3400 Series
			// ATI Radeon HD 3450
			// ATI Radeon HD 3550
			// ATI Radeon HD 4230
			// ATI Radeon HD 4250
		case 0x95C5:
			return S_WEAK;			
			// ATI Radeon HD 2600 PRO
			// ATI Radeon HD 3600 Series
			// ATI Radeon HD 3610
		case 0x9589:
			return S_MEDIUM;
			// ATI Radeon HD 3600 Series
			// ATI Radeon HD 3730
			// ATI Radeon HD3750
			// ATI Radeon HD 4570
			// ATI Radeon HD 4580
			// ATI Radeon HD 4570
			// ATI Radeon HD 4580
			// ATI Radeon HD 4610
		case 0x9598:
			return S_MEDIUM;
			// ATI Radeon HD 3600 Series
		case 0x9591:
			return S_MEDIUM;
			// ATI Radeon HD 3690
			// ATI Radeon HD 3800 Series
			// ATI Radeon HD 3850
			// ATI Radeon HD 3850
			// ATI Radeon HD 4730
			// ATI Radeon HD 4750
		case 0x9505:
			return S_MEDIUM;
			// ATI Radeon HD 3690
			// ATI Radeon HD 3800 Series
			// ATI Radeon HD 3870
			// ATI Radeon HD 4750
		case 0x9501:
			return S_MEDIUM;		
			// ATI Radeon HD 3830
		case 0x9507:
			return S_MEDIUM;
			// ATI Radeon HD 3850 X2
		case 0x9513:
			return S_STRONG;
			// ATI Radeon HD 3870 X2
		case 0x950F:
			return S_STRONG;
			// ATI Radeon HD 4200
		case 0x9710:
			return S_WEAK;
			// ATI Radeon HD 4250
		case 0x9715:
			return S_WEAK;
			// ATI Radeon HD 4290
		case 0x9714:
			return S_WEAK;
			// ATI Radeon HD 4300/4500 Series
			// ATI Radeon HD 4450
			// ATI Radeon HD 4520
			// ATI Radeon HD 4550
		case 0x954F:
			return S_WEAK;
			// ATI Radeon HD 4550
			// ATI Radeon HD 4590
		case 0x9540:
			return S_MEDIUM;			
			// ATI Radeon HD 4710
			// ATI Radeon HD 4600 Series
		case 0x9490:
			return S_MEDIUM;
			// ATI Radeon HD 4600 Series
			// ATI Radeon HD 4650
			// ATI Radeon HD 4670
			// ATI Radeon HD 4700
			// ATI Radeon HD 4720
		case 0x9498:
			return S_MEDIUM;			
			// ATI Radeon HD 4700 Series
			// ATI Radeon HD 4730 Series
			// ATI Radeon HD 4810 Series
			// ATI Radeon HD 4820
		case 0x944E:
			return S_MEDIUM;
			// ATI Radeon HD 4700 Series
		case 0x94B4:
			return S_MEDIUM;			
			// ATI Radeon HD 4770
		case 0x94B3:
			return S_MEDIUM;
			// ATI Radeon HD 4770
		case 0x94B5:
			return S_MEDIUM;
			// ATI Radeon HD 4800 Series
		case 0x944A:
			return S_MEDIUM;
			// ATI Radeon HD 4800 Series
			// ATI Radeon HD 4870
		case 0x9440:
			return S_MEDIUM;
			// ATI Radeon HD 4800 Series
			// ATI Radeon HD 4850
		case 0x9442:
			return S_MEDIUM;
			// ATI Radeon HD 4800 Series
			// ATI Radeon HD 4830
		case 0x944C:
			return S_MEDIUM;
			// ATI Radeon HD 4800 Series
		case 0x9460:
			return S_MEDIUM;
			// ATI Radeon HD 4800 Series
		case 0x9462:
			return S_MEDIUM;					
			// ATI Radeon HD 4850 X2
		case 0x9443:
			return S_STRONG;
			// ATI Radeon HD 4870 X2
		case 0x9441:
			return S_STRONG;
			// ATI Radeon HD 5400 Series
			// ATI Radeon HD 5470
			// ATI Radeon HD 5490
			// ATI Radeon HD 5530
		case 0x68F9:
			return S_MEDIUM;
			// ATI Radeon HD 5500 Series
			// ATI Radeon HD 5630
		case 0x68D9:
			return S_MEDIUM;
			// ATI Radeon HD 5500 Series
			// ATI Radeon HD 5630
		case 0x68DA:
			return S_MEDIUM;	
			// ATI Radeon HD 5600 Series
			// ATI Radeon HD 5690
			// ATI Radeon HD 5730
		case 0x68D8:
			return S_MEDIUM;
			// ATI Radeon HD 5600 Series
		case 0x68B9:
			return S_MEDIUM;						
			// ATI Radeon HD 5700 Series
		case 0x68B8:
			return S_MEDIUM;
			// ATI Radeon HD 5700 Series
		case 0x68BE:
			return S_MEDIUM;
			// ATI Radeon HD 5800 Series
		case 0x6898:
			return S_STRONG;
			// ATI Radeon HD 5800 Series
		case 0x6899:
			return S_STRONG;
			// ATI Radeon HD 5800 Series
		case 0x689E:
			return S_STRONG;
			// ATI Radeon HD 5900 Series
		case 0x689C:
			return S_STRONG;
		    // "AMD Radeon HD 6200 series Graphics" 	
		case 0x9804:
			return S_WEAK;
			// AMD Radeon HD 6200 series Graphics
		case 0x9807:
			return S_WEAK;
			// AMD Radeon HD 6300 series Graphics
		case 0x9802:
			return S_WEAK;
			// AMD Radeon HD 6300 series Graphics
		case 0x9803:
			return S_WEAK;
			// AMD Radeon HD 6300 series Graphics
		case 0x9806:
			return S_WEAK;
			// AMD Radeon HD 6300M Series
		case 0x68E5:
			return S_WEAK;
			// AMD Radeon HD 6300M Series
		case 0x68E4:
			return S_WEAK;
			// AMD Radeon HD 6370D
		case 0x9642:
			return S_WEAK;
			// AMD Radeon HD 6380G
		case 0x9643:
			return S_WEAK;
			// AMD Radeon HD 6400 Series
		case 0x6770:
			return S_WEAK;
			// AMD Radeon HD 6400M Series
		case 0x6760:
			return S_WEAK;
			// AMD Radeon HD 6410D
		case 0x9644:
			return S_WEAK;
			// AMD Radeon HD 6430M
		case 0x6761:
			return S_WEAK;
			// AMD Radeon HD 6450
		case 0x6779:
			return S_WEAK;
			// AMD Radeon E6460
		case 0x6763:
			return S_WEAK;
			// AMD Radeon HD 6480G
		case 0x9648:
			return S_WEAK;
			// AMD Radeon HD 6520G
		case 0x9647:
			return S_MEDIUM;
			// AMD Radeon HD 6530D
		case 0x964A:
			return S_MEDIUM;
			// AMD Radeon HD 6550D
		case 0x9640:
			return S_MEDIUM;
			//	AMD Radeon HD 6570
		case 0x6759:
			return S_STRONG;
			// AMD Radeon 6600M and 6700M Series
		case 0x6741:
			return S_MEDIUM;
			// AMD Radeon HD 6600A Series
		case 0x6750:
			return S_MEDIUM;
			// AMD Radeon HD 6620G
		case 0x9641:
			return S_MEDIUM;
			// AMD Radeon HD 6625M Graphics
		case 0x6742:
			return S_MEDIUM;
			// AMD Radeon HD 6670
		case 0x6758:
			return S_STRONG;
			// AMD Radeon HD 6700 Series
		case 0x68BA:
			return S_STRONG;
			// AMD Radeon HD 6700 Series
		case 0x68BF:
			return S_STRONG;
			// AMD Radeon HD 6700 Series
		case 0x673E:
			return S_STRONG;
			// AMD Radeon HD 6700M Series
		case 0x6740:
			return S_MEDIUM;
			// AMD Radeon E6760
		case 0x6743:
			return S_MEDIUM;
			// AMD Radeon HD 6800 Series
		case 0x6738:
			return S_STRONG;
			// AMD Radeon HD 6800 Series
		case 0x6739:
			return S_STRONG;
			// AMD Radeon HD 6800 Series
		case 0x689B:
			return S_STRONG;
			// AMD Radeon HD 6800M Series
		case 0x68A8:
			return S_STRONG;
			// AMD Radeon HD 6900 Series
		case 0x6718:
			return S_STRONG;
			// AMD Radeon HD 6900 Series
		case 0x6719:
			return S_STRONG;
			// AMD Radeon HD 6900 Series
		case 0x671D:
			return S_STRONG;
			// AMD Radeon HD 6900 Series
		case 0x671F:
			return S_STRONG;
			// AMD Radeon HD 6900M Series
		case 0x6720:
			return S_STRONG;
		}
		break;

		// nVidia
	case 0x10DE:
		switch( ident.DeviceId )
		{
			// "NVIDIA GeForce 6800 Ultra"
		case 0x0040: 
			return S_WEAK;
			// "NVIDIA GeForce 6800"
		case 0x0041: 
			return S_WEAK;
			// "NVIDIA GeForce 6800 LE"
		case 0x0042: 
			return S_WEAK;
			// "NVIDIA GeForce 6800 XE"
		case 0x0043: 
			return S_WEAK;
			// "NVIDIA GeForce 6800 XT"
		case 0x0044: 
			return S_WEAK;
			// "NVIDIA GeForce 6800 GT"
		case 0x0045: 
			return S_WEAK;
			// "NVIDIA GeForce 6800 GT "
		case 0x0046: 
			return S_WEAK;
			// "NVIDIA GeForce 6800 GS"
		case 0x0047: 
			return S_WEAK;
			// "NVIDIA GeForce 6800 XT "
		case 0x0048: 
			return S_MEDIUM;
			// "NVIDIA Quadro FX 3400"
		case 0x004D: 
			return S_MEDIUM;
			// "NVIDIA Quadro FX 4000"
		case 0x004E: 
			return S_WEAK;
			// "NVIDIA GeForce 7800 GTX"
		case 0x0090: 
			return S_WEAK;
			// "NVIDIA GeForce 7800 GTX "
		case 0x0091: 
			return S_WEAK;
			// "NVIDIA GeForce 7800 GT"
		case 0x0092: 
			return S_WEAK;
			// "NVIDIA GeForce 7800 GS"
		case 0x0093: 
			return S_WEAK;
			// "NVIDIA GeForce 7800 SLI"
		case 0x0095: 
			return S_WEAK;
			// "NVIDIA Quadro FX 4500"
		case 0x009D: 
			return S_WEAK;
			// "NVIDIA GeForce 6800 GS "
		case 0x00C0: 
			return S_WEAK;
			// "NVIDIA GeForce 6800 "
		case 0x00C1: 
			return S_WEAK;
			// "NVIDIA GeForce 6800 LE "
		case 0x00C2: 
			return S_WEAK;
			// "NVIDIA GeForce 6800 XT  "
		case 0x00C3: 
			return S_WEAK;
			// "NVIDIA Quadro FX 3450/4000 SDI"
		case 0x00CD: 
			return S_WEAK;
			// "NVIDIA Quadro FX 1400"
		case 0x00CE: 
			return S_WEAK;
			// "NVIDIA GeForce 6600 GT"
		case 0x00F1: 
			return S_WEAK;
			// "NVIDIA GeForce 6600"
		case 0x00F2: 
			return S_WEAK;
			// "NVIDIA GeForce 6200"
		case 0x00F3: 
			return S_WEAK;
			// "NVIDIA GeForce 6600 LE"
		case 0x00F4: 
			return S_WEAK;
			// "NVIDIA GeForce 7800 GS "
		case 0x00F5: 
			return S_WEAK;
			// "NVIDIA GeForce 6800 GS/XT"
		case 0x00F6: 
			return S_WEAK;
			// "NVIDIA Quadro FX 3400/4400"
		case 0x00F8: 
			return S_WEAK;
			// "NVIDIA GeForce 6800 Series GPU"
		case 0x00F9: 
			return S_WEAK;
			// "NVIDIA GeForce 6600 GT "
		case 0x0140: 
			return S_WEAK;
			// "NVIDIA GeForce 6600 "
		case 0x0141: 
			return S_WEAK;
			// "NVIDIA GeForce 6600 LE "
		case 0x0142: 
			return S_WEAK;
			// "NVIDIA GeForce 6600 VE"
		case 0x0143: 
			return S_WEAK;
			// "NVIDIA GeForce 6610 XL"
		case 0x0145: 
			return S_WEAK;
			// "NVIDIA GeForce 6700 XL"
		case 0x0147: 
			return S_WEAK;
			// "NVIDIA Quadro NVS 440"
		case 0x014A: 
			return S_WEAK;
			// "NVIDIA Quadro FX 540M"
		case 0x014C: 
			return S_WEAK;
			// "NVIDIA Quadro FX 550"
		case 0x014D: 
			return S_WEAK;
			// "NVIDIA Quadro FX 540"
		case 0x014E: 
			return S_WEAK;
			// "NVIDIA GeForce 6200 "
		case 0x014F: 
			return S_WEAK;
			// "NVIDIA GeForce 6500"
		case 0x0160: 
			return S_WEAK;
			// "NVIDIA GeForce 6200 TurboCache(TM)"
		case 0x0161: 
			return S_WEAK;
			// "NVIDIA GeForce 6200SE TurboCache(TM)"
		case 0x0162: 
			return S_WEAK;
			// "NVIDIA GeForce 6200 LE"
		case 0x0163: 
			return S_WEAK;
			// "NVIDIA Quadro NVS 285"
		case 0x0165: 
			return S_WEAK;
			// "NVIDIA GeForce 6250"
		case 0x0169: 
			return S_WEAK;
			// "NVIDIA GeForce 7100 GS"
		case 0x016A: 
			return S_WEAK;
			// "NVIDIA GeForce 8800 GTX"
		case 0x0191: 
			return S_WEAK;
			// "NVIDIA GeForce 8800 GTS"
		case 0x0193: 
			return S_WEAK;
			// "NVIDIA GeForce 8800 Ultra"
		case 0x0194: 
			return S_WEAK;
			// "NVIDIA Tesla C870"
		case 0x0197: 
			return S_WEAK;
			// "NVIDIA Quadro FX 5600"
		case 0x019D: 
			return S_WEAK;
			// "NVIDIA Quadro FX 4600"
		case 0x019E: 
			return S_WEAK;
			// "NVIDIA GeForce 7350 LE"
		case 0x01D0: 
			return S_WEAK;
			// "NVIDIA GeForce 7300 LE"
		case 0x01D1: 
			return S_WEAK;
			// "NVIDIA GeForce 7550 LE"
		case 0x01D2: 
			return S_WEAK;
			// "NVIDIA GeForce 7300 SE/7200 GS"
		case 0x01D3: 
			return S_WEAK;
			// "NVIDIA Quadro NVS 120M"
		case 0x01DB: 
			return S_WEAK;
			// "NVIDIA GeForce 7500 LE"
		case 0x01DD: 
			return S_WEAK;
			// "NVIDIA Quadro FX 350"
		case 0x01DE: 
			return S_WEAK;
			// "NVIDIA GeForce 7300 GS"
		case 0x01DF: 
			return S_WEAK;
			// "NVIDIA GeForce 6800  "
		case 0x0211: 
			return S_WEAK;
			// "NVIDIA GeForce 6800 LE  "
		case 0x0212: 
			return S_WEAK;
			// "NVIDIA GeForce 6800 GT  "
		case 0x0215: 
			return S_WEAK;
			// "NVIDIA GeForce 6800 XT   "
		case 0x0218: 
			return S_WEAK;
			// "NVIDIA GeForce 6200  "
		case 0x0221: 
			return S_WEAK;
			// "NVIDIA GeForce 6200 A-LE"
		case 0x0222: 
			return S_WEAK;
			// "NVIDIA GeForce 6150"
		case 0x0240: 
			return S_WEAK;
			// "NVIDIA GeForce 6150 LE"
		case 0x0241: 
			return S_WEAK;
			// "NVIDIA GeForce 6100"
		case 0x0242: 
			return S_WEAK;
			// "NVIDIA Quadro NVS 210S / NVIDIA GeForce 6150LE"
		case 0x0245: 
			return S_WEAK;
			// "NVIDIA GeForce 7900 GTX"
		case 0x0290: 
			return S_WEAK;
			// "NVIDIA GeForce 7900 GT/GTO"
		case 0x0291: 
			return S_WEAK;
			// "NVIDIA GeForce 7900 GS"
		case 0x0292: 
			return S_MEDIUM;
			// "NVIDIA GeForce 7950 GX2"
		case 0x0293: 
			return S_MEDIUM;
			// "NVIDIA GeForce 7950 GX2"
		case 0x0294: 
			return S_WEAK;
			// "NVIDIA GeForce 7950 GT"
		case 0x0295: 
			return S_WEAK;
			// "NVIDIA Quadro FX 5500"
		case 0x029C: 
			return S_WEAK;
			// "NVIDIA Quadro FX 3500"
		case 0x029D: 
			return S_WEAK;
			// "NVIDIA Quadro FX 1500"
		case 0x029E: 
			return S_WEAK;
			// "NVIDIA Quadro FX 4500 X2"
		case 0x029F: 
			return S_WEAK;
			// "NVIDIA GeForce 7600 GT"
		case 0x02E0: 
			return S_WEAK;
			// "NVIDIA GeForce 7600 GS"
		case 0x02E1: 
			return S_WEAK;
			// "NVIDIA GeForce 7300 GT"
		case 0x02E2: 
			return S_WEAK;
			// "NVIDIA GeForce 7900 GS"
		case 0x02E3: 
			return S_WEAK;
			// "NVIDIA GeForce 7950 GT"
		case 0x02E4: 
			return S_WEAK;
			// "NVIDIA GeForce 7650 GS"
		case 0x038B: 
			return S_WEAK;
			// "NVIDIA GeForce 7650 GS "
		case 0x0390: 
			return S_WEAK;
			// "NVIDIA GeForce 7600 GT "
		case 0x0391: 
			return S_WEAK;
			// "NVIDIA GeForce 7600 GS "
		case 0x0392: 
			return S_WEAK;
			// "NVIDIA GeForce 7300 GT "
		case 0x0393: 
			return S_WEAK;
			// "NVIDIA GeForce 7600 LE"
		case 0x0394: 
			return S_WEAK;
			// "NVIDIA GeForce 7300 GT  "
		case 0x0395: 
			return S_WEAK;
			// "NVIDIA Quadro FX 560"
		case 0x039E: 
			return S_WEAK;
			// "NVIDIA GeForce 6150SE nForce 430"
		case 0x03D0: 
			return S_WEAK;
			// "NVIDIA GeForce 6100 nForce 405"
		case 0x03D1: 
			return S_WEAK;
			// "NVIDIA GeForce 6100 nForce 400"
		case 0x03D2: 
			return S_WEAK;
			// "NVIDIA GeForce 6100 nForce 420"
		case 0x03D5: 
			return S_WEAK;
			// "NVIDIA GeForce 7025 / NVIDIA nForce 630a"
		case 0x03D6: 
			return S_WEAK;
			// "NVIDIA GeForce 8600 GTS"
		case 0x0400: 
			return S_WEAK;
			// "NVIDIA GeForce 8600 GT"
		case 0x0401: 
			return S_WEAK;
			// "NVIDIA GeForce 8600 GT "
		case 0x0402: 
			return S_WEAK;
			// "NVIDIA GeForce 8600 GS"
		case 0x0403: 
			return S_WEAK;
			// "NVIDIA GeForce 8400 GS"
		case 0x0404: 
			return S_WEAK;
			// "NVIDIA GeForce 8300 GS"
		case 0x0406: 
			return S_WEAK;
			// "NVIDIA Quadro FX 370"
		case 0x040A: 
			return S_WEAK;
			// "NVIDIA Quadro FX 570"
		case 0x040E: 
			return S_WEAK;
			// "NVIDIA Quadro FX 1700"
		case 0x040F: 
			return S_WEAK;
			// "NVIDIA GeForce GT 330"
		case 0x0410: 
			return S_WEAK;
			// "NVIDIA GeForce 8400 SE"
		case 0x0420: 
			return S_WEAK;
			// "NVIDIA GeForce 8500 GT"
		case 0x0421: 
			return S_WEAK;
			// "NVIDIA GeForce 8400 GS "
		case 0x0422: 
			return S_WEAK;
			// "NVIDIA GeForce 8300 GS "
		case 0x0423: 
			return S_WEAK;
			// "NVIDIA GeForce 8400 GS  "
		case 0x0424: 
			return S_WEAK;
			// "NVIDIA GeForce 9400 GT"
		case 0x042C: 
			return S_WEAK;
			// "NVIDIA Quadro NVS 290"
		case 0x042F: 
			return S_WEAK;
			// "NVIDIA GeForce 7050 PV / NVIDIA nForce 630a"
		case 0x053A: 
			return S_WEAK;
			// "NVIDIA GeForce 7050 PV / NVIDIA nForce 630a "
		case 0x053B: 
			return S_WEAK;
			// "NVIDIA GeForce 7025 / NVIDIA nForce 630a "
		case 0x053E: 
			return S_WEAK;
			// "NVIDIA GeForce GTX 295"
		case 0x05E0: 
			return S_MEDIUM;
			// "NVIDIA GeForce GTX 280"
		case 0x05E1: 
			return S_MEDIUM;
			// "NVIDIA GeForce GTX 260"
		case 0x05E2: 
			return S_MEDIUM;
			// "NVIDIA GeForce GTX 285"
		case 0x05E3: 
			return S_MEDIUM;
			// "NVIDIA GeForce GTX 275"
		case 0x05E6: 
			return S_MEDIUM;
			// "NVIDIA Tesla C1060"
			// "NVIDIA Tesla T10 Processor"
			// "NVIDIA Tesla T10 Processor "
			// "NVIDIA Tesla M1060"
		case 0x05E7: 
			return S_MEDIUM;
			// "NVIDIA GeForce GTX 260 "
		case 0x05EA: 
			return S_MEDIUM;
			// "NVIDIA GeForce GTX 295 "
		case 0x05EB: 
			return S_MEDIUM;
			// "NVIDIA Quadroplex 2200 D2"
		case 0x05ED: 
			return S_MEDIUM;
			// "NVIDIA Quadroplex 2200 S4"
		case 0x05F8: 
			return S_MEDIUM;
			// "NVIDIA Quadro CX"
		case 0x05F9: 
			return S_MEDIUM;
			// "NVIDIA Quadro FX 5800"
		case 0x05FD: 
			return S_MEDIUM;
			// "NVIDIA Quadro FX 4800"
		case 0x05FE: 
			return S_MEDIUM;
			// "NVIDIA Quadro FX 3800"
		case 0x05FF: 
			return S_MEDIUM;
			// "NVIDIA GeForce 8800 GTS 512"
		case 0x0600: 
			return S_MEDIUM;
			// "NVIDIA GeForce 9800 GT"
		case 0x0601: 
			return S_MEDIUM;
			// "NVIDIA GeForce 8800 GT"
		case 0x0602: 
			return S_MEDIUM;
			// "NVIDIA GeForce GT 230"
		case 0x0603: 
			return S_MEDIUM;
			// "NVIDIA GeForce 9800 GX2"
		case 0x0604: 
			return S_STRONG;
			// "NVIDIA GeForce 9800 GT "
		case 0x0605: 
			return S_MEDIUM;
			// "NVIDIA GeForce 8800 GS"
		case 0x0606: 
			return S_MEDIUM;
			// "NVIDIA GeForce GTS 240"
		case 0x0607: 
			return S_MEDIUM;
			// "NVIDIA GeForce 8800 GS "
		case 0x060D: 
			return S_MEDIUM;
			// "NVIDIA GeForce 9600 GSO"
		case 0x0610: 
			return S_MEDIUM;
			// "NVIDIA GeForce 8800 GT "
		case 0x0611: 
			return S_MEDIUM;
			// "NVIDIA GeForce 9800 GTX/9800 GTX+"
		case 0x0612: 
			return S_MEDIUM;
			// "NVIDIA GeForce 9800 GTX+"
		case 0x0613: 
			return S_MEDIUM;
			// "NVIDIA GeForce 9800 GT  "
		case 0x0614: 
			return S_MEDIUM;
			// "NVIDIA GeForce GTS 250"
		case 0x0615: 
			return S_MEDIUM;
			// "NVIDIA Quadro FX 4700 X2"
		case 0x0619: 
			return S_MEDIUM;
			// "NVIDIA Quadro FX 3700"
		case 0x061A: 
			return S_MEDIUM;
			// "NVIDIA Quadro VX 200"
		case 0x061B: 
			return S_MEDIUM;
			// "NVIDIA GeForce 9600 GT"
		case 0x0622: 
			return S_MEDIUM;
			// "NVIDIA GeForce 9600 GS"
		case 0x0623: 
			return S_MEDIUM;
			// "NVIDIA GeForce 9600 GSO 512"
		case 0x0625: 
			return S_MEDIUM;
			// "NVIDIA GeForce GT 130"
		case 0x0626: 
			return S_MEDIUM;
			// "NVIDIA GeForce GT 140"
		case 0x0627: 
			return S_MEDIUM;
			// "NVIDIA GeForce 9600 GT "
		case 0x062D: 
			return S_MEDIUM;
			// "NVIDIA GeForce 9600 GT  "
		case 0x062E: 
			return S_WEAK;
			// "NVIDIA GeForce 9600 GSO "
		case 0x0635: 
			return S_MEDIUM;
			// "NVIDIA GeForce 9600 GT   "
		case 0x0637: 
			return S_MEDIUM;
			// "NVIDIA Quadro FX 1800"
		case 0x0638: 
			return S_MEDIUM;
			// "NVIDIA GeForce 9500 GT"
		case 0x0640: 
			return S_MEDIUM;
			// "NVIDIA GeForce 9400 GT "
		case 0x0641: 
			return S_MEDIUM;
			// "NVIDIA GeForce 9500 GT "
		case 0x0643: 
			return S_MEDIUM;
			// "NVIDIA GeForce 9500 GS"
		case 0x0644: 
			return S_MEDIUM;
			// "NVIDIA GeForce 9500 GS "
		case 0x0645: 
			return S_MEDIUM;
			// "NVIDIA GeForce GT 120"
		case 0x0646: 
			return S_MEDIUM;
			// "NVIDIA Quadro FX 380"
		case 0x0658: 
			return S_MEDIUM;
			// "NVIDIA Quadro FX 580"
		case 0x0659: 
			return S_MEDIUM;
			// "NVIDIA GeForce 9400 GT  "
		case 0x065B: 
			return S_MEDIUM;
			// "NVIDIA GeForce G210"
		case 0x065F: 
			return S_MEDIUM;
			// "NVIDIA GeForce 9300 GE"
		case 0x06E0: 
			return S_MEDIUM;
			// "NVIDIA GeForce 9300 GS"
		case 0x06E1: 
			return S_MEDIUM;
			// "NVIDIA GeForce 8400"
		case 0x06E2: 
			return S_MEDIUM;
			// "NVIDIA GeForce 8400 SE "
		case 0x06E3: 
			return S_MEDIUM;
			// "NVIDIA GeForce 8400 GS    "
		case 0x06E4: 
			return S_MEDIUM;
			// "NVIDIA GeForce G100"
		case 0x06E6: 
			return S_MEDIUM;
			// "NVIDIA GeForce 9300 SE"
		case 0x06E7: 
			return S_MEDIUM;
			// "NVIDIA GeForce 9300M GS"
		case 0x06E9: 
			return S_MEDIUM;
			// "NVIDIA Quadro NVS 420"
		case 0x06F8: 
			return S_MEDIUM;
			// "NVIDIA Quadro FX 370 LP"
		case 0x06F9: 
			return S_MEDIUM;
			// "NVIDIA Quadro NVS 450"
		case 0x06FA: 
			return S_MEDIUM;
			// "NVIDIA Quadro NVS 295"
		case 0x06FD: 
			return S_MEDIUM;
			// "NVIDIA HICx16 + Graphics"
			// "NVIDIA HICx8 + Graphics"
		case 0x06FF: 
			return S_MEDIUM;
			// "NVIDIA GeForce 7150 / NVIDIA nForce 630i"
		case 0x07E0: 
			return S_MEDIUM;
			// "NVIDIA GeForce 7100 / NVIDIA nForce 630i"
		case 0x07E1: 
			return S_WEAK;
			// "NVIDIA GeForce 7050 / NVIDIA nForce 630i"
		case 0x07E2: 
			return S_WEAK;
			// "NVIDIA GeForce 7050 / NVIDIA nForce 610i"
		case 0x07E3: 
			return S_WEAK;
			// "NVIDIA GeForce 7050 / NVIDIA nForce 620i"
		case 0x07E5: 
			return S_WEAK;
			// "NVIDIA GeForce 9200"
		case 0x0846: 
			return S_WEAK;
			// "NVIDIA GeForce 9100"
		case 0x0847: 
			return S_WEAK;
			// "NVIDIA GeForce 8300"
		case 0x0848: 
			return S_WEAK;
			// "NVIDIA GeForce 8200"
		case 0x0849: 
			return S_WEAK;
			// "NVIDIA nForce 730a"
		case 0x084A: 
			return S_WEAK;
			// "NVIDIA GeForce 9200 "
		case 0x084B: 
			return S_WEAK;
			// "NVIDIA nForce 980a/780a SLI"
		case 0x084C: 
			return S_WEAK;
			// "NVIDIA nForce 750a SLI"
		case 0x084D: 
			return S_WEAK;
			// "NVIDIA GeForce 8100 / nForce 720a"
		case 0x084F: 
			return S_WEAK;
			// "NVIDIA GeForce 9400"
		case 0x0860: 
			return S_WEAK;
			// "NVIDIA GeForce 9400 "
		case 0x0861: 
			return S_WEAK;
			// "NVIDIA GeForce 9300"
		case 0x0864: 
			return S_WEAK;
			// "NVIDIA ION"
		case 0x0865: 
			return S_WEAK;
			// "NVIDIA nForce 760i SLI"
		case 0x0868: 
			return S_WEAK;
			// "NVIDIA GeForce 9400   "
		case 0x086A: 
			return S_WEAK;
			// "NVIDIA GeForce 9300 / nForce 730i"
		case 0x086C: 
			return S_WEAK;
			// "NVIDIA GeForce 9200  "
		case 0x086D: 
			return S_WEAK;
			// "NVIDIA GeForce 9200   "
		case 0x0871: 
			return S_WEAK;
			// "NVIDIA ION "
		case 0x0874: 
			return S_WEAK;
			// "NVIDIA ION  "
		case 0x0876: 
			return S_WEAK;
			// "NVIDIA GeForce 9400    "
		case 0x087A: 
			return S_WEAK;
			// "NVIDIA ION   "
		case 0x087D: 
			return S_WEAK;
			// "NVIDIA ION LE"
		case 0x087E: 
			return S_WEAK;
			// "NVIDIA ION LE "
		case 0x087F: 
			return S_WEAK;
			// "NVIDIA GeForce GT 220 "
		case 0x0A20: 
			return S_WEAK;
			// "NVIDIA GeForce 210"
		case 0x0A23: 
			return S_WEAK;
			// "NVIDIA GeForce G210 "
		case 0x0A60: 
			return S_WEAK;
			// "NVIDIA GeForce 205"
		case 0x0A62: 
			return S_WEAK;
			// "NVIDIA ION    "
		case 0x0A64: 
			return S_WEAK;
			// "NVIDIA GeForce 210 "
		case 0x0A65: 
			return S_WEAK;
			// "NVIDIA Quadro FX 380 LP"
		case 0x0A78: 
			return S_WEAK;
			// "NVIDIA GeForce GT 330 "
		case 0x0CA0: 
			return S_WEAK;
			// "NVIDIA GeForce GT 320"
		case 0x0CA2: 
			return S_WEAK;
			// "NVIDIA GeForce GT 240"
		case 0x0CA3: 
			return S_WEAK;
			// "NVIDIA GeForce GT 340"
		case 0x0CA4: 
			return S_WEAK;
			// "NVIDIA GeForce GT 330  "
		case 0x0CA7: 
			 return S_WEAK;
		    //	"NVIDIA GeForce GTX 460"
		case 0x0E22: 
			return S_STRONG;
		};
		break;
	}

	return S_WEAK;
}

wchar_t* utf8ToWide(const char* str)
{
	if(str == NULL || *str == 0)
		return L"";

	static wchar_t wbuf[2048];
	int wchars = ::MultiByteToWideChar(
		CP_UTF8,		// convert from UTF-8
		MB_ERR_INVALID_CHARS,	// error on invalid chars
		str,			// source UTF-8 string
		strlen(str) + 1,	// total length of source UTF-8 string, encluding EOF
		wbuf,
		2048			// size of destination buffer, in WCHAR's
		);
	return wbuf;
}

char* wideToUtf8(const wchar_t* str)
{
	if(str == NULL || *str == 0)
		return "";

	static char buf[2048];
	int chars = ::WideCharToMultiByte(
		CP_UTF8,		// convert to UTF-8
		0,			// WC_ERR_INVALID_CHARS for Vista+ error on invalid chars
		str,			// source string
		wcslen(str) + 1,	// total length of source string, encluding EOF
		buf,
		2048,			// size of destination buffer, in bytes
		NULL,
		NULL
		);
	return buf;
}

#ifdef _DEBUG
#include <DbgHelp.h>
#endif

void r3dPrintCallStack()
{
#ifdef _DEBUG
	void* ptrs[ 33 ] = {} ;

	memset( ptrs, -1, sizeof ptrs ) ;

	if( !_r3d_bPrintCallStackInit )
	{
		if( HMODULE hMod = LoadLibrary( "kernel32" ) )
		{
			g_CaptureStackBackTrace = (CaptureStackBackTraceFunc) GetProcAddress( hMod, "RtlCaptureStackBackTrace" ) ;

			FreeLibrary( hMod ) ;
		}
		
		_r3d_bPrintCallStackInit = 1 ;
	}

	if( g_CaptureStackBackTrace )
	{
		enum
		{
			NAME_LENGTH = 128,
			SYMBOL_INFO_SIZE = sizeof(SYMBOL_INFO) + NAME_LENGTH
		};

		char info_data[ SYMBOL_INFO_SIZE ] = { 0 } ;
		SYMBOL_INFO* info = (SYMBOL_INFO*)info_data ;

		info->SizeOfStruct = sizeof(SYMBOL_INFO) ;
		info->MaxNameLen = NAME_LENGTH ;

		g_CaptureStackBackTrace ( 1, 32, ptrs, NULL ) ;

		HANDLE curProcess = GetCurrentProcess() ;

		DWORD64 dsp = 0 ;

		r3dOutToLog( "Callstack:\n" );

		for( void** p = ptrs ; (int)*p != -1 ; p ++ )
		{
			if( SymFromAddr( curProcess, (int)*p, 0, info ) )
			{
				r3dOutToLog( "%s\n", info->Name ) ;
			}
			else
			{
				r3dOutToLog( "%p\n", *p ) ;
			}
		}
	}
#endif
}
