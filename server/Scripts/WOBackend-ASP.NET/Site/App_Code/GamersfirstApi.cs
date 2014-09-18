using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Web;
using System.Web.Script.Serialization;
using System.Data.SqlClient;
using System.Diagnostics;
using System.Configuration;
using System.Net;
using System.IO;
using System.Xml;
using System.Xml.Serialization;
using System.Text;

public class GamerstirstApi
{
    //static string baseUrl_ = @"http://services-devx.connect.gamersfirst.com:8000/";
    static string baseUrl_ = @"https://services.connect.gamersfirst.com/";

    static string MERCHANT_ID = "1c091916-3c0f-45f0-a5c0-993900caeb90";
    static string SECRET_KEY = "05<R:69o85LRK;y:(sPDY0\"N2.50(1";

    public string lastData_ = "";
    public string lastPost_ = "";
    public string lastUrl_ = "";

    public GamerstirstApi()
    {
    }

    Dictionary<string, string> ParseJsonAnd(string ans)
    {
        try
        {
            JavaScriptSerializer jss = new JavaScriptSerializer();
            Dictionary<string, string> dict = jss.Deserialize<Dictionary<string, string>>(ans);
            return dict;
        }
        catch // eat up deserialization error, as it's completly useless here
        {
            return null;
        }
    }

    public Dictionary<string, string> AuthenticateWithToken(string token)
    {
        Dictionary<string, string> prm = new Dictionary<string, string>();
        prm.Add("token", token);
        string ans = HttpPost("GamersFirstConnect/REST/V1.2/Accounts/AuthenticateWithToken.json", prm, true);
        return ParseJsonAnd(ans);
    }

    public Dictionary<string, string> GetAccountClassification(string accountId)
    {
        Dictionary<string, string> prm = new Dictionary<string, string>();
        prm.Add("accountId", accountId);
        string ans = HttpPost("GamersFirstConnect/REST/V1.2/Accounts/GetAccountClassification.json", prm, false);
        return ParseJsonAnd(ans);
    }

    string HttpPost(string func, Dictionary<string, string> parameters, bool usePost)
    {
        HttpWebRequest webRequest = null;
        HttpWebResponse webResponse = null;

        parameters.Add("MerchantKey", MERCHANT_ID);

        // parameters: name1=value1&name2=value2	
        string postLine = "";
        string md5str = "";
        foreach (KeyValuePair<string, string> kvp in parameters)
        {
            if (postLine != "") postLine += "&";
            postLine += kvp.Key + "=" + kvp.Value;
            md5str += kvp.Value + "|";
        }
        md5str += SECRET_KEY;
        postLine += "&Hash=" + WebHelper.CalculateMD5Hash(md5str).ToLower();
        lastPost_ = postLine;

        if (usePost)
        {
            lastUrl_ = baseUrl_ + func;
            webRequest = (HttpWebRequest)WebRequest.Create(lastUrl_);
            webRequest.KeepAlive = false;
            webRequest.Timeout = 15000;
            webRequest.Method = "POST";
            webRequest.ContentType = "application/x-www-form-urlencoded";
            webRequest.ServicePoint.Expect100Continue = false;  // needed for fix steam 502 error

            // send post data
            byte[] bytes = Encoding.UTF8.GetBytes(postLine);
            webRequest.ContentLength = bytes.Length;    // Count bytes to send
            using (Stream os = webRequest.GetRequestStream())
            {
                os.Write(bytes, 0, bytes.Length);           // Send it
            }
        }
        else
        {
            lastUrl_ = baseUrl_ + func + "?" + postLine;
            webRequest = (HttpWebRequest)WebRequest.Create(lastUrl_);
            webRequest.KeepAlive = false;
            webRequest.Timeout = 15000;
            webRequest.ServicePoint.Expect100Continue = false;
        }

        // get the response
        try
        {
            webResponse = (HttpWebResponse)webRequest.GetResponse();

            Stream strResponse = webResponse.GetResponseStream();
            strResponse.ReadTimeout = 15000;
            StreamReader sr = new StreamReader(strResponse);
            string data = sr.ReadToEnd();
            lastData_ = data;
            return data.Trim();
        }
        catch (WebException ex)
        {
            if (ex.Status == WebExceptionStatus.ProtocolError)
            {
                StreamReader webResponseReader = new StreamReader(ex.Response.GetResponseStream());
                lastData_ = webResponseReader.ReadToEnd();
            }

            throw new ApiExitException("HttpPost " + ex.Message + " - " + lastData_);
        }
        finally
        {
            if (webResponse != null)
                webResponse.Close();
        }
    }


}
