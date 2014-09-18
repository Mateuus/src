using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Web;
using System.Data.SqlClient;
using System.Diagnostics;
using System.Configuration;
using System.Net;
using System.IO;
using System.Xml;
using System.Xml.Serialization;
using System.Text;

public class GNAApi
{
    string baseUrl_ = @"https://api.gamenet.ru/restapi";
    string serviceId_ = "300005010000000000";
    public string lastUrl_ = "";
    public string lastData_ = "";

    public class getServiceAccountResp
    {
    }

    public GNAApi()
    {
    }

    ~GNAApi()
    {
    }

    public GNAXML.getServiceToken getServiceToken(string userId)
    {
        Dictionary<string, string> prm = new Dictionary<string, string>();
        prm.Add("method", "auth.getServiceToken");
        prm.Add("serviceId", serviceId_);
        prm.Add("authType", "UserId");
        prm.Add("authId", userId);
        string ans = HttpGet(baseUrl_, prm);

        try
        {
            XmlSerializer xs = new XmlSerializer(typeof(GNAXML.getServiceToken));
            MemoryStream memoryStream = new MemoryStream(WebHelper.StringToUTF8ByteArray(ans));
            GNAXML.getServiceToken resp = (GNAXML.getServiceToken)xs.Deserialize(memoryStream);
            return resp;
        }
        catch (Exception e)
        {
            throw e;
        }
    }

    public GNAXML.checkSubscribeStatus checkSubscribeStatus(string userId)
    {
        Dictionary<string, string> prm = new Dictionary<string, string>();
        prm.Add("method", "user.checkSubscribeStatus");
        prm.Add("serviceId", serviceId_);
        prm.Add("authType", "UserId");
        prm.Add("authId", userId);
        string ans = HttpGet(baseUrl_, prm);

        try
        {
            XmlSerializer xs = new XmlSerializer(typeof(GNAXML.checkSubscribeStatus));
            MemoryStream memoryStream = new MemoryStream(WebHelper.StringToUTF8ByteArray(ans));
            GNAXML.checkSubscribeStatus resp = (GNAXML.checkSubscribeStatus)xs.Deserialize(memoryStream);
            return resp;
        }
        catch (Exception e)
        {
            throw e;
        }
    }

    public GNAXML.getShortInfo getShortInfo(string userId, string appKey)
    {
        Dictionary<string, string> prm = new Dictionary<string, string>();
        prm.Add("method", "user.getShortInfo");
        prm.Add("userId", userId);
        prm.Add("appKey", appKey);
        prm.Add("contactId", userId);
        string ans = HttpGet(baseUrl_, prm);

        try
        {
            XmlSerializer xs = new XmlSerializer(typeof(GNAXML.getShortInfo));
            MemoryStream memoryStream = new MemoryStream(WebHelper.StringToUTF8ByteArray(ans));
            GNAXML.getShortInfo resp = (GNAXML.getShortInfo)xs.Deserialize(memoryStream);
            return resp;
        }
        catch (Exception e)
        {
            throw e;
        }
    }

    string HttpGet(string uri, Dictionary<string, string> parameters)
    {
        HttpWebRequest webRequest = null;
        HttpWebResponse webResponse = null;

        // parameters: name1=value1&name2=value2	
        string postLine = "";
        foreach (KeyValuePair<string, string> kvp in parameters)
        {
            if (postLine != "") postLine += "&";
            postLine += System.Web.HttpUtility.UrlEncode(kvp.Key);
            postLine += "=";
            postLine += System.Web.HttpUtility.UrlEncode(kvp.Value);
        }

        string url_for_get = uri + "?" + postLine;
        lastUrl_ = url_for_get;
        webRequest = (HttpWebRequest)WebRequest.Create(url_for_get);
        webRequest.KeepAlive = true;
        webRequest.Timeout = 90000;
        //webRequest.ContentType = "application/x-www-form-urlencoded";
        //webRequest.Method = "POST";
        webRequest.AllowAutoRedirect = true;

        try
        {
            // get the response
            webResponse = (HttpWebResponse)webRequest.GetResponse();

            Stream strResponse = webResponse.GetResponseStream();
            strResponse.ReadTimeout = 90000;
            StreamReader sr = new StreamReader(strResponse);
            string data = sr.ReadToEnd();
            lastData_ = data;
            return data.Trim();
        }
        catch (WebException ex)
        {
            throw new ApiExitException("GNA " + ex.Message);
        }
        finally
        {
            if (webResponse != null)
                webResponse.Close();
        }
    }


}
