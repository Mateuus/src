#include "scoexp.h"
#include "..\..\..\..\Eternity\Include\r3dBinMesh.h"

extern 	ClassDesc* 	GetEternityExpDesc();
extern 	TCHAR*		GetString(int id);

	HINSTANCE 	hInstance;
	int 		controlsInit = FALSE;

static 	BOOL 		showPrompts;
static 	BOOL 		exportSelected;


#define 	ETERNITYEXP_CLASS_ID	Class_ID(0x3e3b787e, 0x150e2346)

void ParsePath(char *in, char *dir, char *path)
{
  char	*p;
 
  // parse directory name
  if((p=strrchr(in, '\\')) == NULL) {
    *dir = 0;
    strcpy(path, in);
  } else {
    strncpy(dir, in, p-in + 1);
    dir[p-in + 1] = 0;
    strcpy(path, p + 1);
  }

  // remove file extension
  if(strrchr(path, '.'))
    *strrchr(path, '.') = 0;
 
  return;
}



BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) 
{
 hInstance = hinstDLL;

 // Initialize the custom controls. This should be done only once.
 if (!controlsInit)
 {
   controlsInit = TRUE;
   InitCustomControls(hInstance);
   InitCommonControls();
 }
	
 return (TRUE);
}


__declspec( dllexport ) const TCHAR* LibDescription() 
{
 return _T("Eternity Scene Exporter ");
}

/// MUST CHANGE THIS NUMBER WHEN ADD NEW CLASS 
__declspec( dllexport ) int LibNumberClasses() 
{
	return 1;
}


__declspec( dllexport ) ClassDesc* LibClassDesc(int i) 
{
	switch(i) {
		case 0:  return GetEternityExpDesc();
		default: return 0;
	}
}

__declspec( dllexport ) ULONG LibVersion() 
{
	return VERSION_3DSMAX;
}

// Let the plug-in register itself for deferred loading
__declspec( dllexport ) ULONG CanAutoDefer()
{
	return 1;
}

class EternityExpClassDesc:public ClassDesc
{
 public:
	int		IsPublic() 	{ return 1; }
	void*		Create(BOOL loading = FALSE) { return new EternityExp; } 
	const TCHAR*	ClassName() 	{ return _T("SCOExporter"); }
	SClass_ID	SuperClassID() 	{ return SCENE_EXPORT_CLASS_ID; } 
	Class_ID	ClassID() 	{ return ETERNITYEXP_CLASS_ID; }
	const TCHAR*	Category() 	{ return _T("Standard"); }
};

static EternityExpClassDesc EternityExpDesc;

ClassDesc* GetEternityExpDesc()
{
 return &EternityExpDesc;
}

TCHAR *GetString(int id)
{
 static TCHAR buf[256];

 if (hInstance)
 	return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;

 return NULL;
}

EternityExp::EternityExp()
{
 bSaveScene		= TRUE;
 bIncludeMesh 		= TRUE;
 bIncludeMaterials 	= TRUE;
 bIncludeVertexColors 	= TRUE;

 bIncludeCamera 	= FALSE;
 bIncludeLight 		= TRUE;

 bSaveAnimPath          = FALSE;

 bSaveAsBinary          = FALSE;
 bExportAnimMesh        = FALSE;

 nUVPrecision 		= 12;
 nGeometryPrecision     = 4;
 nStaticFrame 		= 0;
}

EternityExp::~EternityExp()
{
}


static BOOL CALLBACK AboutBoxDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
 switch (msg)
 {
  case WM_INITDIALOG:
 	CenterWindow(hWnd, GetParent(hWnd)); 
	break;
 
  case WM_COMMAND:
	switch (LOWORD(wParam))
	{
	 case IDOK:
		EndDialog(hWnd, 1);
		break;
	}
	break;
  
  default:
	return FALSE;
 }

 return TRUE;
}       

void EternityExp::ShowAbout(HWND hWnd)
{
 DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, AboutBoxDlgProc, 0);
}


void SetButton(HWND hWnd, int btn_id, int checked, int enabled)
{
  EnableWindow(GetDlgItem(hWnd, btn_id), enabled);
  CheckDlgButton(hWnd, btn_id, enabled ? checked : FALSE);
}


// Dialog proc
static BOOL CALLBACK ExportDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
 Interval animRange;
 ISpinnerControl  *spin;

 EternityExp *exp = (EternityExp*)GetWindowLong(hWnd,GWL_USERDATA);

 switch (msg)
 {
  case WM_INITDIALOG:
	exp = (EternityExp*)lParam;
	SetWindowLong(hWnd, GWL_USERDATA,lParam);
	CenterWindow(hWnd, GetParent(hWnd));

	SetButton(hWnd, IDC_SCENE,     exp->bSaveScene,        1);
	SetButton(hWnd, IDC_MESHDATA,  exp->bIncludeMesh,      1);
	SetButton(hWnd, IDC_MATERIAL,  exp->bIncludeMaterials, 1);
	SetButton(hWnd, IDC_CAMERA,    exp->bIncludeCamera,    exp->NumCameras);
	SetButton(hWnd, IDC_LIGHT,     exp->bIncludeLight,     exp->NumLights);

	SetButton(hWnd, IDC_ANIMPATH,  exp->bSaveAnimPath,     exp->bAnimPathPresent);
	SetButton(hWnd, IDC_ANIMMESH,  exp->bExportAnimMesh,   exp->bAnimMeshPresent);

	SetButton(hWnd, IDC_VERTEXCOLORS, exp->bIncludeVertexColors, 1);
	SetButton(hWnd, IDC_ISBINARY,     exp->bSaveAsBinary, 0);

	// Setup the spinner controls for the UV floating point precision
	spin = GetISpinner(GetDlgItem(hWnd, IDC_UV_PREC_SPIN));
	spin->LinkToEdit(GetDlgItem(hWnd,IDC_UV_PREC), EDITTYPE_INT );
	spin->SetLimits(1, 32, TRUE);
	spin->SetScale(1.0f);
	spin->SetValue(exp->nUVPrecision, FALSE);
	ReleaseISpinner(spin);

	// Setup the spinner controls for the Geometry floating point precision
	spin = GetISpinner(GetDlgItem(hWnd, IDC_VERT_PREC_SPIN)); 
	spin->LinkToEdit(GetDlgItem(hWnd,IDC_VERT_PREC), EDITTYPE_INT ); 
	spin->SetLimits(1, 32, TRUE); 
	spin->SetScale(1.0f);
	spin->SetValue(exp->nGeometryPrecision, FALSE);
	ReleaseISpinner(spin);
	
	// Setup the spinner control for the static frame#
	animRange    = exp->GetInterface()->GetAnimRange();
	exp->nStaticFrame = exp->GetInterface()->GetTime() / GetTicksPerFrame();
	spin = GetISpinner(GetDlgItem(hWnd, IDC_STATIC_FRAME_SPIN)); 
	spin->LinkToEdit(GetDlgItem(hWnd,IDC_STATIC_FRAME), EDITTYPE_INT ); 
	spin->SetLimits(animRange.Start() / GetTicksPerFrame(), animRange.End() / GetTicksPerFrame(), TRUE); 
	spin->SetScale(1.0f);
	spin->SetValue(exp->nStaticFrame, FALSE);
	ReleaseISpinner(spin);
	
	break;

 case CC_SPINNER_CHANGE:
	spin = (ISpinnerControl*)lParam; 
	break;

 case WM_COMMAND:
	switch (LOWORD(wParam))
	{
	 case IDC_ANIMPATH:
	 {
		if(HIWORD(wParam) == BN_CLICKED)
		  exp->bSaveAnimPath ^= 1;

		EnableWindow(GetDlgItem(hWnd, IDOK), (exp->bSaveAnimPath || exp->bSaveScene));
		break;
	 }

	 case IDC_SCENE:
	 {
		if(HIWORD(wParam) == BN_CLICKED) 
		  exp->bSaveScene ^= 1;

		EnableWindow(GetDlgItem(hWnd, IDOK), (exp->bSaveAnimPath || exp->bSaveScene));

		static	int	Controls[] = {
		  IDC_MESHDATA,
		  IDC_MATERIAL,
		  IDC_LIGHT,
		  IDC_CAMERA,
		};
		for(int i=0; i<4; i++) {
		  EnableWindow(GetDlgItem(hWnd, Controls[i]), exp->bSaveScene);
		}
		break;
	 }

  	 case IDOK:
		exp->bSaveScene        = IsDlgButtonChecked(hWnd, IDC_SCENE);
		exp->bIncludeMesh      = IsDlgButtonChecked(hWnd, IDC_MESHDATA);
		exp->bIncludeMaterials = IsDlgButtonChecked(hWnd, IDC_MATERIAL);
		exp->bIncludeCamera    = IsDlgButtonChecked(hWnd, IDC_CAMERA);
		exp->bIncludeLight     = IsDlgButtonChecked(hWnd, IDC_LIGHT);
		exp->bIncludeVertexColors = IsDlgButtonChecked(hWnd, IDC_VERTEXCOLORS);

		exp->bSaveAnimPath     = IsDlgButtonChecked(hWnd, IDC_ANIMPATH);
		exp->bExportAnimMesh   = IsDlgButtonChecked(hWnd, IDC_ANIMMESH);

  	        exp->bSaveAsBinary     = IsDlgButtonChecked(hWnd, IDC_ISBINARY); 

		spin = GetISpinner(GetDlgItem(hWnd, IDC_UV_PREC_SPIN)); 
		exp->nUVPrecision = spin->GetIVal();
		ReleaseISpinner(spin);

		spin = GetISpinner(GetDlgItem(hWnd, IDC_VERT_PREC_SPIN)); 
		exp->nGeometryPrecision = spin->GetIVal();
		ReleaseISpinner(spin);


		spin = GetISpinner(GetDlgItem(hWnd, IDC_STATIC_FRAME_SPIN)); 
		exp->nStaticFrame = spin->GetIVal() * GetTicksPerFrame();
		ReleaseISpinner(spin);
			
		EndDialog(hWnd, 1);
		break;
  
      	 case IDCANCEL:
		EndDialog(hWnd, 0);
		break;
	}
	break;
 
 default:
	return FALSE;
 }
 
 return TRUE;
}       

// Dummy function for progress bar
DWORD WINAPI fn(LPVOID arg)
{
	return(0);
}

// Start the exporter!
// This is the real entrypointto the exporter. Aft er the user has selected
// the filename (and he's prompted for overwrite etc.) this method is called.
int EternityExp::DoExport(const TCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts, DWORD options) 
{
 ParsePath((char *)name, DirName, FileName);

// Set a global prompt display switch
 
 showPrompts 	= suppressPrompts ? FALSE : TRUE;
 exportSelected = (options & SCENE_EXPORT_SELECTED) ? TRUE : FALSE;

 // Grab the interface pointer.
 ip = i;

 //
 // preprocess scene
 //
 nTotalNodeCount  = 0;
 nCurNode         = 0;
 NumLights        = 0;
 NumCameras       = 0;
 bAnimPathPresent = 0;
 bAnimMeshPresent = 0;
 PreProcess(ip->GetRootNode(), nTotalNodeCount);

 if(!DialogBoxParam(
    hInstance, 
    MAKEINTRESOURCE(IDD_ETERNITYEXPORT_DLG),
    ip->GetMAXHWnd(), 
    ExportDlgProc, 
    (LPARAM)this)) 
 {
   return 1;
 }

 sprintf(szFmtUVStr,       "%%4.%df", nUVPrecision);
 sprintf(szFmtGeometryStr, "%%4.%df", nGeometryPrecision);

 // Startup the progress bar.
 ip->ProgressStart(GetString(IDS_PROGRESS_MSG), TRUE, fn, NULL);

 char buf[256];
 if(bSaveAnimPath) {
   sprintf(buf, "%s\\%s.pth", DirName, FileName);
   fAnimPath  = fopen(buf,"wt");
 }

 if(bIncludeLight) {
   sprintf(buf, "%s\\%s.lit", DirName, FileName);
   fLights = fopen(buf, "wt");
 }
	
 // First we write out a file header with global information. 
 ExportGlobalInfo();

 // Export list of material definitions
 if(bIncludeMaterials )
   ExportMaterialList();

 // Call our node enumerator.
 // The nodeEnum function will recurse into itself and
 // export each object found in the scene.
 for(int idx=0; idx<ip->GetRootNode()->NumberOfChildren(); idx++)
 {
   if(ip->GetCancel())
     break;
   nodeEnum(ip->GetRootNode()->GetChildNode(idx));
 }

 // We're done. Finish the progress bar.
 ip->ProgressEnd();

 if(bSaveAnimPath) fclose(fAnimPath);
 if(bIncludeLight) fclose(fLights);

 return 1;
}

BOOL EternityExp::SupportsOptions(int ext, DWORD options) {
	assert(ext == 0);	// We only support one extension
	return(options == SCENE_EXPORT_SELECTED) ? TRUE : FALSE;
	}


// Before recursing into the children of a node, we will export it.
// The benefit of this is that a nodes parent is always before the
// children in the resulting file. This is desired since a child's
// transformation matrix is optionally relative to the parent.
BOOL EternityExp::nodeEnum(INode* node)
{
 nCurNode++;
 ip->ProgressUpdate((int)((float)nCurNode/nTotalNodeCount*100.0f));

 // Stop recursing if the user pressed Cancel
 if(ip->GetCancel())
    return FALSE;

 // Only export if exporting everything or it's selected
 if(!exportSelected || node->Selected())
 {
  // The ObjectState is a 'thing' that flows down the pipeline containing
  // all information about the object. By calling EvalWorldState() we tell
  // max to eveluate the object at end of the pipeline.
  ObjectState os = node->EvalWorldState(nStaticFrame);

  do
  {
    if(!os.obj)
      break;

     // Targets are actually geomobjects, but we will export them
     // from the camera and light objects, so we skip them here.
     if(os.obj->ClassID() == Class_ID(TARGET_CLASS_ID, 0))
       break;

     if(bSaveAnimPath) switch(os.obj->SuperClassID())
     {
       case GEOMOBJECT_CLASS_ID:
       case CAMERA_CLASS_ID:
       case LIGHT_CLASS_ID:
	 ExportAnimPath(node);
	 break;
     }

     if(bSaveScene) switch(os.obj->SuperClassID())
     {
       case GEOMOBJECT_CLASS_ID:
	 if(bIncludeMesh)
	   ExportGeomObject(node);
	 break;

       case CAMERA_CLASS_ID:
	 if(bIncludeCamera)
	   ExportCameraObject(node);
	 break;

       case LIGHT_CLASS_ID:
	 if(bIncludeLight)
	   ExportLightObject(node);
	 break;
     }

     break;
   } while(0);
 }

 // For each child of this node, we recurse into ourselves
 // until no more children are found.
 for(int c = 0; c < node->NumberOfChildren(); c++)
   if(!nodeEnum(node->GetChildNode(c)))
     return FALSE;

 return TRUE;
}


void EternityExp::PreProcess(INode* node, int& nodeCount)
{
  nodeCount++;

  // Add the nodes material to out material list
  // Null entries are ignored when added...
  mtlList.AddMtl(node->GetMtl());

  // do specific checks for each type of objects
  do
  {
    ObjectState os = node->EvalWorldState(GetStaticFrame());
    if(!os.obj)
      break;

    switch(os.obj->SuperClassID()) {
      case GEOMOBJECT_CLASS_ID:
      {
	// skip targets
	if(os.obj->ClassID() == Class_ID(TARGET_CLASS_ID, 0))
	  break;

	// check if we have any mesh animation here
	if(!bAnimMeshPresent) {
	  Interval animRange = ip->GetAnimRange();
	  Interval objRange  = os.obj->ObjectValidity(GetStaticFrame());
	  // If the animation range is not fully included in the validity
	  // interval of the object, then we're animated.
	  if(!objRange.InInterval(animRange))
	    bAnimMeshPresent = 1;
	}

	break;
      }

      case CAMERA_CLASS_ID:
	NumCameras++;
	break;

      case LIGHT_CLASS_ID:
        NumLights++;
	break;
    }

  } while(0);

  // check if we have animation in any of our nodes
  if(!bAnimPathPresent) {
    BOOL bPos, bRot, bScale;
    if(CheckForAnimation(node, bPos, bRot, bScale))
      bAnimPathPresent = 1;
  }

  // For each child of this node, we recurse into ourselves
  // and increment the counter until no more children are found.
  for(int c = 0; c < node->NumberOfChildren(); c++)
  {
    PreProcess(node->GetChildNode(c), nodeCount);
  }

  return;
}



BOOL MtlKeeper::AddMtl(Mtl* mtl)
{
  if(!mtl)
    return FALSE;

  int numMtls = mtlTab.Count();

  for(int i=0; i<numMtls; i++) {
    if(mtlTab[i] == mtl)
      return FALSE;
  }

  mtlTab.Append(1, &mtl, 25);

  return TRUE;
}

int MtlKeeper::GetMtlID(Mtl* mtl)
{
  int numMtls = mtlTab.Count();
  for(int i=0; i<numMtls; i++) {
    if(mtlTab[i] == mtl)
      return i;
  }

  return -1;
}

int MtlKeeper::Count()
{
  return mtlTab.Count();
}

Mtl* MtlKeeper::GetMtl(int id)
{
  return mtlTab[id];
}
