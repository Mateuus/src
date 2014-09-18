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

public class SteamApi
{
    string baseUrl_ = @"https://api.steampowered.com/";
    string microTxnUrl_ = @"ISteamMicroTxn/";
    string APP_ID = "107900";
    string PUBLISHER_KEY = "344B9A063B9E8E4A13CF0ADADE86CD6E";
    public string lastData_ = "";
    public string lastPost_ = "";
    public string lastUrl_ = "";

    public SteamApi()
    {
    }

    ~SteamApi()
    {
    }

    public string GetSteamId(string ticket)
    {
        SteamXML.AuthResponse resp = AuthenticateUserTicket(ticket);

        if (resp.error != null)
        {
            return "";
        }

        if (resp.params_ == null || resp.params_.result != "OK")
        {
            return "";
        }

        return resp.params_.steamid.ToString();
    }

    SteamXML.AuthResponse ParseAuthResp(string ans)
    {
        try
        {
            XmlSerializer xs = new XmlSerializer(typeof(SteamXML.AuthResponse));
            MemoryStream memoryStream = new MemoryStream(WebHelper.StringToUTF8ByteArray(ans));
            SteamXML.AuthResponse resp = (SteamXML.AuthResponse)xs.Deserialize(memoryStream);
            return resp;
        }
        catch (Exception e)
        {
            throw e;
        }
    }

    public SteamXML.AuthResponse AuthenticateUserTicket(string ticket)
    {
        Dictionary<string, string> prm = new Dictionary<string, string>();
        prm.Add("format", "xml");
        prm.Add("appid", APP_ID);
        prm.Add("key", PUBLISHER_KEY);

        prm.Add("ticket", ticket);

        string ans = HttpPost("ISteamUserAuth/AuthenticateUserTicket/V0001/", prm, false);
        return ParseAuthResp(ans);
    }


    SteamXML.MicroTnxResponse ParseMicroTnxResp(string ans)
    {
        try
        {
            XmlSerializer xs = new XmlSerializer(typeof(SteamXML.MicroTnxResponse));
            MemoryStream memoryStream = new MemoryStream(WebHelper.StringToUTF8ByteArray(ans));
            SteamXML.MicroTnxResponse resp = (SteamXML.MicroTnxResponse)xs.Deserialize(memoryStream);
            return resp;
        }
        catch (Exception e)
        {
            throw e;
        }
    }

    public SteamXML.MicroTnxResponse GetUserInfo(string steamId)
    {
        Dictionary<string, string> prm = new Dictionary<string, string>();
        prm.Add("format", "xml");
        prm.Add("appid", APP_ID);
        prm.Add("key", PUBLISHER_KEY);

        prm.Add("steamid", steamId);

        string ans = HttpPost("ISteamMicroTxn/GetUserInfo/v0001/", prm, false);
        return ParseMicroTnxResp(ans);
    }

    public string MarkSteamAchievementAchieved( string steamID, string achievementName )
    {
        Dictionary<string, string> prm = new Dictionary<string, string>();
        prm.Add("format", "json");
        prm.Add("appid", APP_ID);
        prm.Add("key", PUBLISHER_KEY);
        prm.Add("steamid", steamID );
        prm.Add("count", Convert.ToString( 1 ));
        prm.Add("name[0]", Convert.ToString( achievementName ) );
        prm.Add("value[0]", Convert.ToString( 1 ) );

        string ans = HttpPost("ISteamUserStats/SetUserStatsForGame/V0001/", prm, true);
        return ans;
        
    }

    public SteamXML.MicroTnxResponse InitTxn(
        string orderId, string steamId, 
        string currency, string PriceUSDCents,
        string itemId, string itemDesc, 
        string language)
    {
        Dictionary<string, string> prm = new Dictionary<string, string>();
        prm.Add("format", "xml");
        prm.Add("appid", APP_ID);
        prm.Add("key", PUBLISHER_KEY);

        prm.Add("orderid", orderId);
        prm.Add("steamid", steamId);
        prm.Add("itemcount", "1");
        prm.Add("language", language);
        prm.Add("currency", currency);
        prm.Add("itemid[0]", itemId);
        prm.Add("qty[0]", "1");
        prm.Add("amount[0]", PriceUSDCents);
        prm.Add("description[0]", itemDesc);

        string ans = HttpPost(microTxnUrl_ + "InitTxn/V0001/", prm, true);
        return ParseMicroTnxResp(ans);
    }

    public SteamXML.MicroTnxResponse FinalizeTxn(string orderId)
    {
        Dictionary<string, string> prm = new Dictionary<string, string>();
        prm.Add("format", "xml");
        prm.Add("appid", APP_ID);
        prm.Add("key", PUBLISHER_KEY);

        prm.Add("orderid", orderId.ToString());

        string ans = HttpPost(microTxnUrl_ + "FinalizeTxn/V0001/", prm, true);
        return ParseMicroTnxResp(ans);
    }

    public SteamXML.MicroTnxResponse QueryTxn(string orderId)
    {
        Dictionary<string, string> prm = new Dictionary<string, string>();
        prm.Add("format", "xml");
        prm.Add("appid", APP_ID);
        prm.Add("key", PUBLISHER_KEY);

        prm.Add("orderid", orderId.ToString());

        string ans = HttpPost(microTxnUrl_ + "QueryTxn/V0001/", prm, false);
        return ParseMicroTnxResp(ans);
    }

    string HttpPost(string func, Dictionary<string, string> parameters, bool usePost)
    {
        HttpWebRequest webRequest = null;
        HttpWebResponse webResponse = null;

        // parameters: name1=value1&name2=value2	
        string postLine = "";
        foreach (KeyValuePair<string, string> kvp in parameters)
        {
            if (postLine != "") postLine += "&";
            postLine += kvp.Key; //steam web api DOES NOT work with keys encoded by some reason
            postLine += "=";
            postLine += System.Web.HttpUtility.UrlPathEncode(kvp.Value);
        }
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

            throw new ApiExitException("Steam " + ex.Message + " - " + lastData_);
        }
        finally
        {
            if (webResponse != null)
                webResponse.Close();
        }
    }


}
