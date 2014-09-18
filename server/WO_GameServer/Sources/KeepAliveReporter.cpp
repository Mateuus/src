#include "r3dPCH.h"
#include "r3d.h"

#include "KeepAliveReporter.h"

CKeepAliveReporter gKeepAliveReporter;

CKeepAliveReporter::CKeepAliveReporter()
{
  hMapFile = NULL;
  info     = NULL;
}

CKeepAliveReporter::~CKeepAliveReporter()
{
  Close();
}

void CKeepAliveReporter::Init(DWORD in_gameId)
{
  char mmname[128];
  sprintf(mmname, "WO_GServer_%08x", in_gameId);

  // Open the shared file
  hMapFile = OpenFileMapping(
    FILE_MAP_ALL_ACCESS, // read/write access
    FALSE,		 // do not inherit the name
    mmname);		 // name of mapping object
  if(hMapFile == NULL) {
    r3dError("unable to OpenFileMapping: %d\n", GetLastError());
    return;
  }

  // Map to the file
  info = (MMapGameInfo_s*)MapViewOfFile(
    hMapFile, // handle to map object
    FILE_MAP_ALL_ACCESS,	// read/write permission
    0,
    0,
    sizeof(MMapGameInfo_s)); 
  if(info == NULL) {
    r3dError("unable to MapViewOfFile: %d\n", GetLastError());
  }
  
  r3dOutToLog("MMapGameInfo_s at %p\n", info);
  r3d_assert(in_gameId == info->gameId);

  return;
}

void CKeepAliveReporter::Close()
{
  if(info)
  {
    UnmapViewOfFile(info);
    info = NULL;
  }

  if(hMapFile)
  {
    CloseHandle(hMapFile);
    hMapFile = NULL;
  }
}

