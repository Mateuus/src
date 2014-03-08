#include "plugin.h"
#include "npfunctions.h"

NPError NPP_New(NPMIMEType pluginType,	NPP instance,	uint16_t mode,	int16_t argc,	char* argn[],	char* argv[],	NPSavedData* saved)
{
  if(instance == NULL)	return NPERR_INVALID_INSTANCE_ERROR;

	//if cmdLine is passed so do launch the game
  instance->pdata = 0;
	for(int i=0; i<argc; i++)
	{
		if(_strcmpi(argn[i], "cmdLine")==0)
		{
			wchar_t wcs[256];
			mbstowcs(wcs, argv[i], sizeof(wcs));
			instance->pdata = new CPlugin(instance, wcs);
			break;
		}
	}
	
	return NPERR_NO_ERROR;
}

// here is the place to clean up and destroy the CPlugin object
NPError NPP_Destroy (NPP instance, NPSavedData** save)
{
  if(instance == NULL)	return NPERR_INVALID_INSTANCE_ERROR;

  CPlugin * pPlugin = (CPlugin *)instance->pdata;
  if(pPlugin != NULL)
    delete pPlugin;
  return NPERR_NO_ERROR;
}

// during this call we know when the plugin window is ready or
// is about to be destroyed so we can do some gui specific
// initialization and shutdown
NPError NPP_SetWindow (NPP instance, NPWindow* pNPWindow)
{    
  if(instance == NULL)	return NPERR_INVALID_INSTANCE_ERROR;
  if(pNPWindow == NULL)	return NPERR_GENERIC_ERROR;
  if(pNPWindow->window == NULL)	return NPERR_GENERIC_ERROR;
	if(instance->pdata == NULL)	return NPERR_GENERIC_ERROR;
	CPlugin* p = (CPlugin*)instance->pdata;
  p->SetWindow((HWND)pNPWindow->window);
  return NPERR_NO_ERROR;
}

NPError	NPP_GetValue(NPP instance, NPPVariable variable, void *value)
{
  if(instance == NULL)	return NPERR_INVALID_INSTANCE_ERROR;

  switch (variable)
	{
  case NPPVpluginNameString:
    *((char **)value) = "WIU";
    break;
  case NPPVpluginDescriptionString:
    *((char **)value) = "WI game updater";
    break;
  default:
    return NPERR_GENERIC_ERROR;
  }
  return NPERR_NO_ERROR;
}



NPError NPP_NewStream(NPP instance,	NPMIMEType type,	NPStream* stream, NPBool seekable,	uint16_t* stype)
{
  if(instance == NULL)	return NPERR_INVALID_INSTANCE_ERROR;
  return NPERR_NO_ERROR;
}
int32_t NPP_WriteReady (NPP instance, NPStream *stream)
{
  if(instance == NULL)	return NPERR_INVALID_INSTANCE_ERROR;
  return 0x0fffffff;
}
int32_t NPP_Write (NPP instance, NPStream *stream, int32_t offset, int32_t len, void *buffer)
{   
  if(instance == NULL)	return NPERR_INVALID_INSTANCE_ERROR;
  return len;
}
NPError NPP_DestroyStream (NPP instance, NPStream *stream, NPError reason)
{
  if(instance == NULL)	return NPERR_INVALID_INSTANCE_ERROR;
  return NPERR_NO_ERROR;
}
void NPP_StreamAsFile (NPP instance, NPStream* stream, const char* fname)
{
}
void NPP_Print (NPP instance, NPPrint* printInfo)
{
}
void NPP_URLNotify(NPP instance, const char* url, NPReason reason, void* notifyData)
{
}
NPError NPP_SetValue(NPP instance, NPNVariable variable, void *value)
{
  if(instance == NULL)	return NPERR_INVALID_INSTANCE_ERROR;
  return NPERR_NO_ERROR;
}
int16_t	NPP_HandleEvent(NPP instance, void* event)
{
  return 0;
}






NPNetscapeFuncs NPNFuncs;

NPError OSCALL NP_GetEntryPoints(NPPluginFuncs* pFuncs)
{
  if(pFuncs == NULL)	return NPERR_INVALID_FUNCTABLE_ERROR;
  if(pFuncs->size < sizeof(NPPluginFuncs))	return NPERR_INVALID_FUNCTABLE_ERROR;

  pFuncs->version       = (NP_VERSION_MAJOR << 8) | NP_VERSION_MINOR;
  pFuncs->newp          = NPP_New;
  pFuncs->destroy       = NPP_Destroy;
  pFuncs->setwindow     = NPP_SetWindow;
  pFuncs->newstream     = NPP_NewStream;
  pFuncs->destroystream = NPP_DestroyStream;
  pFuncs->asfile        = NPP_StreamAsFile;
  pFuncs->writeready    = NPP_WriteReady;
  pFuncs->write         = NPP_Write;
  pFuncs->print         = NPP_Print;
  pFuncs->event         = NPP_HandleEvent;
  pFuncs->urlnotify     = NPP_URLNotify;
  pFuncs->getvalue      = NPP_GetValue;
  pFuncs->setvalue      = NPP_SetValue;
  pFuncs->javaClass     = NULL;

  return NPERR_NO_ERROR;
}


char* NP_GetMIMEDescription()
{
  return "application/npwiu";
}


NPError OSCALL NP_Initialize(NPNetscapeFuncs* pFuncs)
{
	//MessageBox(0,"1","1", MB_OK);
  if(pFuncs == NULL) return NPERR_INVALID_FUNCTABLE_ERROR;
  if(((((uint32_t)pFuncs->version) & 0xff00) >> 8) > NP_VERSION_MAJOR) return NPERR_INCOMPATIBLE_VERSION_ERROR;
  if(pFuncs->size < sizeof(NPNetscapeFuncs)) return NPERR_INVALID_FUNCTABLE_ERROR;

  NPNFuncs.size                    = pFuncs->size;
  NPNFuncs.version                 = pFuncs->version;
  NPNFuncs.geturlnotify            = pFuncs->geturlnotify;
  NPNFuncs.geturl                  = pFuncs->geturl;
  NPNFuncs.posturlnotify           = pFuncs->posturlnotify;
  NPNFuncs.posturl                 = pFuncs->posturl;
  NPNFuncs.requestread             = pFuncs->requestread;
  NPNFuncs.newstream               = pFuncs->newstream;
  NPNFuncs.write                   = pFuncs->write;
  NPNFuncs.destroystream           = pFuncs->destroystream;
  NPNFuncs.status                  = pFuncs->status;
  NPNFuncs.uagent                  = pFuncs->uagent;
  NPNFuncs.memalloc                = pFuncs->memalloc;
  NPNFuncs.memfree                 = pFuncs->memfree;
  NPNFuncs.memflush                = pFuncs->memflush;
  NPNFuncs.reloadplugins           = pFuncs->reloadplugins;
  NPNFuncs.getJavaEnv              = NULL;
  NPNFuncs.getJavaPeer             = NULL;
  NPNFuncs.getvalue                = pFuncs->getvalue;
  NPNFuncs.setvalue                = pFuncs->setvalue;
  NPNFuncs.invalidaterect          = pFuncs->invalidaterect;
  NPNFuncs.invalidateregion        = pFuncs->invalidateregion;
  NPNFuncs.forceredraw             = pFuncs->forceredraw;
  NPNFuncs.getstringidentifier     = pFuncs->getstringidentifier;
  NPNFuncs.getstringidentifiers    = pFuncs->getstringidentifiers;
  NPNFuncs.getintidentifier        = pFuncs->getintidentifier;
  NPNFuncs.identifierisstring      = pFuncs->identifierisstring;
  NPNFuncs.utf8fromidentifier      = pFuncs->utf8fromidentifier;
  NPNFuncs.intfromidentifier       = pFuncs->intfromidentifier;
  NPNFuncs.createobject            = pFuncs->createobject;
  NPNFuncs.retainobject            = pFuncs->retainobject;
  NPNFuncs.releaseobject           = pFuncs->releaseobject;
  NPNFuncs.invoke                  = pFuncs->invoke;
  NPNFuncs.invokeDefault           = pFuncs->invokeDefault;
  NPNFuncs.evaluate                = pFuncs->evaluate;
  NPNFuncs.getproperty             = pFuncs->getproperty;
  NPNFuncs.setproperty             = pFuncs->setproperty;
  NPNFuncs.removeproperty          = pFuncs->removeproperty;
  NPNFuncs.hasproperty             = pFuncs->hasproperty;
  NPNFuncs.hasmethod               = pFuncs->hasmethod;
  NPNFuncs.releasevariantvalue     = pFuncs->releasevariantvalue;
  NPNFuncs.setexception            = pFuncs->setexception;

  return NPERR_NO_ERROR;
}

NPError OSCALL NP_Shutdown()
{
  return NPERR_NO_ERROR;
}
