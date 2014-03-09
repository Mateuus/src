#include "r3dPCH.h"
#include "r3d.h"
#include "CkGzip.h"

#include "GameCode/UserProfile.h"

#include "WOBackendAPI.h"

	const char*	gDomainBaseUrl= "/Warinc/api/";
	int		gDomainPort   = 443; // PAX_BUILD - change to 80 and no SSL
	bool		gDomainUseSSL = true;
	
CWOBackendReq::CWOBackendReq(const char* url)
{
	Init(url);
}

CWOBackendReq::CWOBackendReq(const CUserProfile* prof, const char* url)
{
	Init(url);

	AddSessionInfo(prof->CustomerID, prof->SessionID);
}

void CWOBackendReq::Init(const char* url)
{
#ifndef FINAL_BUILD
	// automatically switch to local site version 
	if(stricmp(g_api_ip->GetString(), "198.50.173.40") == 0)
	{
		gDomainBaseUrl= "/Warinc/api/";
		gDomainPort   = 80;
		gDomainUseSSL = false;
	}
#endif

	resp_       = NULL;
	
	savedUrl_   = url;
	resultCode_ = 0;
	bodyStr_    = "";
	bodyLen_    = 0;
	
	int success;
	success = http.UnlockComponent("ARKTOSHttp_decCLPWFQXmU");
	if (success != 1)
	{
		r3dError("failed to unlock CkHttp");
		return;
	}
	http.put_ConnectTimeout(30);
	http.put_ReadTimeout(60);

	// create request
	char fullUrl[512];
	if(url[0] != '/')
	  sprintf(fullUrl, "%s%s", gDomainBaseUrl, url);
	else 
	  sprintf(fullUrl, "%s", url);

	req.UsePost();
	req.put_Path(fullUrl);
	req.put_Utf8(true);
}

CWOBackendReq::~CWOBackendReq()
{
	SAFE_DELETE(resp_);
}

void CWOBackendReq::AddSessionInfo(DWORD id, DWORD sid)
{
	r3d_assert(id);
	
	AddParam("s_id",  (int)id);
	AddParam("s_key", (int)sid);
}

void CWOBackendReq::AddParam(const char* name, const char* val)
{
	req.AddParam(name, val);
}

void CWOBackendReq::AddParam(const char* name, int val)
{
	char	buf[1024];
	sprintf(buf, "%d", val);
	AddParam(name, buf);
}

int CWOBackendReq::ParseResult(CkHttpResponse* resp)
{
	if(resp == NULL)
	{
		r3dOutToLog("WO_API: http timeout\n");
		return 8;
	}

	if(resp->get_StatusCode() != 200)
	{
		r3dOutToLog("WO_API: returned http%d\n", resp->get_StatusCode());
		return 8;
	}
	
	// NOTE: we can't use getBodyStr() because it skip zeroes inside answer body
	resp->get_Body(data_);
	data_.appendChar(0);
	
	// if context is gzipped, uncompress it
	if(data_.getSize() >= 2 && data_.getByte(0) == '$' && data_.getByte(1) == '1')
	{
		CkGzip gzip;
		if(gzip.UnlockComponent("ARKTOSZIP_cRvE6e7mpSqD") == false)
			r3dError("failed to unlock gzip");
	
		// remove header and decompress
		CkByteData udata;
		data_.removeChunk(0, 2);
		if(!gzip.UncompressMemory(data_, udata)) {
			r3dOutToLog("WO_API: decompress failed\n");
			return 9;
		}
		//r3dOutToLog("WO_API: decompressed %d->%d\n", data_.getSize(), udata.getSize());

		// swap data with uncompressed one
		data_ = udata;
		data_.appendChar(0);
	}
	bodyStr_ = (const char*)data_.getData();
	bodyLen_ = data_.getSize() - 1;
	
	// validate http answer
	if(bodyLen_ < 4)
	{
		r3dOutToLog("WO_API: too small answer\n");
		return 9;
	}
	
	// check if we got XML header "<?xml"
	if(bodyStr_[0] == '<' && bodyStr_[1] == '?' && bodyStr_[2] == 'x')
	{
		return 0;
	}
	
	// check for 'WO_x' header
	if(bodyStr_[0] != 'W' || bodyStr_[1] != 'O' || bodyStr_[2] != '_')
	{
		r3dOutToLog("WO_API: wrong header: %s\n", bodyStr_);
		return 9;
	}

	int resultCode = bodyStr_[3] - '0';
	
	// offset body content past header
	bodyStr_ += 4;
	bodyLen_ -= 4;

	// parse result code
	switch(resultCode)
	{
		case 0:	// operation ok
			break;
			
		case 1:	// session not valid
			r3dOutToLog("WO_API: session disconnected\n");
			break;

		default:
			r3dOutToLog("WO_API: failed with error code %d %s\n", resultCode, bodyStr_);
			break;
	}

	return resultCode;
}

bool CWOBackendReq::Issue()
{
	SAFE_DELETE(resp_);

	float t1 = r3dGetTime();
	resp_ = http.SynchronousRequest(g_api_ip->GetString(), gDomainPort, gDomainUseSSL, req);
	#ifndef FINAL_BUILD
	//r3dOutToLog("WOApi: %s NETWORK time: %.4f\n", savedUrl_, r3dGetTime()-t1);
	#endif

	resultCode_ = ParseResult(resp_);
	return resultCode_ == 0;
}


void CWOBackendReq::ParseXML(pugi::xml_document& xmlFile)
{
	pugi::xml_parse_result parseResult = xmlFile.load_buffer_inplace((void*)bodyStr_, bodyLen_);
	if(!parseResult)
		r3dError("Failed to parse server XML, error: %s", parseResult.description());
}