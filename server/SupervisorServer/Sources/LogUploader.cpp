#include "r3dPCH.h"
#include "r3d.h"

#include "CkByteData.h"
#include "CkGZip.h"

#include "LogUploader.h"
#include "SupervisorServer.h"
#include "SupervisorConfig.h"

CLogUploader	gLogUploader;

static	const char*	gDomainBaseUrl= "/Warinc/";
static	int		gDomainPort   = 443;
static	bool		gDomainUseSSL = true;
// see CLogUploader::Start for dev mode override

CLogUploader::CLogUploader()
{
	uploadThread_ = NULL;

	bool success = http_.UnlockComponent("ARKTOSHttp_decCLPWFQXmU");
	if(!success || !http_.IsUnlocked()) {
		r3dError("unable to unlock http component", MB_OK);
		return;
	}
	http_.put_ConnectTimeout(30);
	http_.put_ReadTimeout(60);
	
	uploadFinished_ = false;
}

CLogUploader::~CLogUploader()
{
	Stop(false);
}

void CLogUploader::Start()
{
	if(gSupervisorConfig->uploadLogs_ == 2) 
	{
		r3dOutToLog("LogUploader: Working in DEV mode\n");

		g_api_ip->SetString("127.0.0.1");
		gDomainBaseUrl= "/warbrasil/";
		gDomainPort   = 80;
		gDomainUseSSL = false;
	}

	ScanForNewLogs();
	
	// create upload thread
	DWORD temp;
	uploadThread_ = CreateThread(
		NULL,
		0,
		&UploadThreadEntry,
		(void *)this,		// argument
		0,			// creation flags
		&temp			// thread ID
		);

	return;
}

void CLogUploader::Stop(bool waitForUploadFinish)
{
	if(uploadThread_ == NULL)
		return;
		
	r3dOutToLog("LogUploader: stopping, %d in queue\n", logs_.size());
	
	//TODO: waitForUploadFinish

	TerminateThread(uploadThread_, 0);
	uploadThread_ = NULL;
}

void CLogUploader::ScanForNewLogs()
{
	WIN32_FIND_DATA ffblk;
	HANDLE h;

	h = FindFirstFile("logss\\*.txt", &ffblk);
	if(h != INVALID_HANDLE_VALUE) do 
	{
		__int64 sessionId = 0;
		sscanf(ffblk.cFileName + 3, "%I64x", &sessionId); // skip GS_
		
		if(gSupervisorServer.IsActiveSession(sessionId)) {
			//r3dOutToLog(" Active Session\n");
			continue;
		}

		r3dOutToLog("LogUploader: found %I64x\n", sessionId);
		logs_.push_back(sessionId);
	
	} while(FindNextFile(h, &ffblk) != 0);
	
	FindClose(h);
	
	uploadFinished_ = logs_.size() == 0 ? true : false;
}

DWORD __stdcall CLogUploader::UploadThreadEntry(LPVOID in)
{
	CLogUploader* This = (CLogUploader*)in;
	return This->UploadThread();
}

int CLogUploader::UploadDataToServer(__int64 sessionId, const CkByteData& logdata, const CkByteData& dmpdata)
{
	char strSessionId[64];
	char strSize[64];
	sprintf(strSessionId, "%I64u", sessionId);

	float t1 = r3dGetTime();

	CkGzip gzip;
	if(gzip.UnlockComponent("ARKTOSZIP_cRvE6e7mpSqD") == false)
		r3dError("failed to unlock gzip");
	
	CkByteData cdata1;
	if(!gzip.CompressMemory(logdata, cdata1)) {
		r3dOutToLog("!!! can't compress log file %s\n", strSessionId);
		return 0;
	}

	r3dOutToLog("LogUploader: %s, %d->%d start, %d in queue\n", strSessionId, logdata.getSize(), cdata1.getSize(), logs_.size());
	
	CkHttpRequest	req;
	char fullUrl[512];
	sprintf(fullUrl, "%s%s", gDomainBaseUrl, "api_SrvUploadLogFile.aspx");

	req.UseUpload();
	req.put_Path(fullUrl);
	req.AddParam("skey1", "CfFkqQWjfgksYG56893GDhjfjZ20");
	req.AddParam("sessionId", strSessionId);

	if(!req.AddBytesForUpload("logFile", strSessionId, cdata1))
	{
		r3dOutToLog("!!! can't add log file %s to upload - %s\n", strSessionId, req.lastErrorText());
		return 0;
	}
	sprintf(strSize, "%d", logdata.getSize());
	req.AddParam("logSize", strSize);

	// add dump file if it exists
	CkByteData cdata2;
	if(dmpdata.getSize() > 0)
	{
		if(!gzip.CompressMemory(dmpdata, cdata2)) {
			r3dOutToLog("!!! can't compress dump file %s\n", strSessionId);
			return 0;
		}
		if(!req.AddBytesForUpload("dmpFile", strSessionId, cdata2))
		{
			r3dOutToLog("!!! can't add log file %s to upload - %s\n", strSessionId, req.lastErrorText());
			return 0;
		}
		sprintf(strSize, "%d", dmpdata.getSize());
		req.AddParam("dmpSize", strSize);
	}

	// add jpg files
	WIN32_FIND_DATA ffblk;
	char jpgfname[MAX_PATH];
	sprintf(jpgfname, "logss\\GS_%I64x*.jpg", sessionId);
	HANDLE h = FindFirstFile(jpgfname, &ffblk);
	if(h != INVALID_HANDLE_VALUE)
	{
		int numJpgFiles = 0;
		do
		{
			char jpgFileId[64];
			sprintf(jpgFileId, "jpgFile%d", numJpgFiles);
			sprintf(jpgfname, "logss\\%s", ffblk.cFileName);
			
			CkByteData jpgdata;
			jpgdata.loadFile(jpgfname);
			if(!req.AddBytesForUpload(jpgFileId, ffblk.cFileName, jpgdata))
			{
				r3dOutToLog("!!! can't add file %s to upload - %s\n", jpgfname, req.lastErrorText());
				break;
			}
			numJpgFiles++;
		} while(FindNextFile(h, &ffblk ) != 0);
		FindClose(h);
	}


	// start upload
	CkHttpResponse* resp = NULL;
	resp = http_.SynchronousRequest(g_api_ip->GetString(), gDomainPort, gDomainUseSSL, req);

	if(resp == NULL) {
		r3dOutToLog("LogUploader: %s - timeout %s\n", strSessionId, http_.lastErrorText());
		return 0;
	}
	
	if(resp->get_StatusCode() != 200) {
		r3dOutToLog("LogUploader: %s - http%d\n", strSessionId, resp->get_StatusCode());
		//r3dOutToLog("%s\n", resp->bodyStr());
		SAFE_DELETE(resp);
		return 0;
	}
	
	const char* bodyStr = resp->bodyStr();
	if(stricmp(bodyStr, "WO_0") != 0) {
		r3dOutToLog("LogUploader: !!! %s - failed %s\n", strSessionId, bodyStr);
		SAFE_DELETE(resp);
		return 0;
	}
	
	SAFE_DELETE(resp);

	float t2 = r3dGetTime();
	r3dOutToLog("LogUploader: %s finish, %f sec\n", strSessionId, t2 - t1);
	return 1;
}

void CLogUploader::MoveFiles(__int64 sessionId)
{
#if 1
	// do not move logs - just delete them
	char fname1[MAX_PATH];
	WIN32_FIND_DATA ffblk;
	sprintf(fname1, "logss\\GS_%I64x*.*", sessionId);
	HANDLE h = FindFirstFile(fname1, &ffblk);
	if(h != INVALID_HANDLE_VALUE)
	{
		do
		{
			sprintf(fname1, "logss\\%s", ffblk.cFileName);
			::DeleteFile(fname1);
		} while(FindNextFile(h, &ffblk ) != 0);
		FindClose(h);
	}
#else
	time_t time1;
	time(&time1);
	struct tm time2;
	localtime_s(&time2, &time1);

	char dir[MAX_PATH];
	sprintf(dir, "saved_logss\\%d-%02d-%02d", 1900 + time2.tm_year, time2.tm_mon, time2.tm_mday);
	CreateDirectory("saved_logss", NULL);
	CreateDirectory(dir, NULL);

	char fname1[MAX_PATH];
	char fname2[MAX_PATH];
	sprintf(fname1, "logss\\GS_%I64x.txt", sessionId);
	sprintf(fname2, "%s\\GS_%I64x.txt", dir, sessionId);
	::MoveFile(fname1, fname2);

	sprintf(fname1, "logss\\GS_%I64x.dmp", sessionId);
	sprintf(fname2, "%s\\GS_%I64x.dmp", dir, sessionId);
	::MoveFile(fname1, fname2);

	// move jpegs
	WIN32_FIND_DATA ffblk;
	sprintf(fname1, "logss\\GS_%I64x*.jpg", sessionId);
	HANDLE h = FindFirstFile(fname1, &ffblk);
	if(h != INVALID_HANDLE_VALUE)
	{
		do
		{
			sprintf(fname1, "logss\\%s", ffblk.cFileName);
			sprintf(fname2, "%s\\%s", dir, ffblk.cFileName);
			::MoveFile(fname1, fname2);
		} while(FindNextFile(h, &ffblk ) != 0);
		FindClose(h);
	}
#endif	
}

int CLogUploader::UploadLog(__int64 sessionId)
{
	char fname1[MAX_PATH];
	char fname2[MAX_PATH];
	sprintf(fname1, "logss\\GS_%I64x.txt", sessionId);
	sprintf(fname2, "logss\\GS_%I64x.dmp", sessionId);

	CkByteData data;
	if(!data.loadFile(fname1)) {
		r3dOutToLog("!!! can't open log file %s\n", sessionId);
		return 0;
	}
	
	// load .dmp file (if it exists)
	CkByteData crashdata;
	crashdata.loadFile(fname2);
	
	if(UploadDataToServer(sessionId, data, crashdata) == 0) 
	{
		r3dOutToLog("LogUploader: fatal error, removing all queue and sleeping for 5 min\n");

		logs_.clear();
		uploadFinished_ = true;

		::Sleep(5 * 60 * 1000);
		return 0;
	}
	
	// upload success - move files
	MoveFiles(sessionId);
	return 1;
}

DWORD CLogUploader::UploadThread()
{
	bool finished_ = false;
	while(!finished_)
	{
		try
		{
			if(logs_.size() == 0)
			{
				::Sleep(1000);
				ScanForNewLogs();
				continue;
			}
				
			__int64 sessionId = logs_.front();
			logs_.pop_front();
			
			UploadLog(sessionId);
			::Sleep(50);
			continue;
		}
		catch(const char* err)
		{
			r3dOutToLog("CLogUploader: failed %s\n", err);
		}
	}

	return 0;
}