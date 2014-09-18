#pragma once
#include <deque>

#include "CkHttp.h"
#include "CkHttpRequest.h"
#include "CkHttpResponse.h"
#include "CkByteData.h"

class CLogUploader
{
  private:
	static DWORD __stdcall UploadThreadEntry(LPVOID in);
	DWORD		UploadThread();
	HANDLE		uploadThread_;

	CkHttp		http_;
	
	std::deque<__int64> logs_;
	volatile bool	uploadFinished_;
	int		UploadLog(__int64 sessionId);
	int		 UploadDataToServer(__int64 sessionId, const CkByteData& logdata, const CkByteData& dmpdata);
	void		 MoveFiles(__int64 sessionId);
	void		ScanForNewLogs();
  
  public:
	CLogUploader();
	~CLogUploader();
	
	void		Start();
	void		Stop(bool waitForUploadFinish);
};

extern CLogUploader	gLogUploader;
