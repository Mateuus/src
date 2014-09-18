using System;
using System.Collections.Generic;
using System.Web;
using System.IO;
using System.Net;
using System.Text;

/// <summary>
/// Summary description for CreateForumAcc
/// </summary>
public class CreateForumAcc
{
    public static void Create(string username, string password, string email)
    {
        Dictionary<string, string> parameters = new Dictionary<string, string>();
        parameters.Add("WOKey", "f$4gkzkdk3zj");
        parameters.Add("user", username);
        parameters.Add("passwd", password);
        parameters.Add("email", email);

        string full_uri = "http://forums.thewarinc.com/wo/wo_adduser.php";

        //
        // copy-paste from HttpPost
        //

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

        webRequest = (HttpWebRequest)WebRequest.Create(full_uri);
        webRequest.KeepAlive = false;
        webRequest.Timeout = 15000;
        webRequest.Method = "POST";
        webRequest.ContentType = "application/x-www-form-urlencoded";

        // send post data
        byte[] bytes = Encoding.UTF8.GetBytes(postLine);
        webRequest.ContentLength = bytes.Length;    // Count bytes to send
        using (Stream os = webRequest.GetRequestStream())
        {
            os.Write(bytes, 0, bytes.Length);           // Send it
        }

        // get the response
        try
        {
            webResponse = (HttpWebResponse)webRequest.GetResponse();

            Stream strResponse = webResponse.GetResponseStream();
            strResponse.ReadTimeout = 15000;
            StreamReader sr = new StreamReader(strResponse);
            string data = sr.ReadToEnd();
        }
        catch (WebException ex)
        {
            throw new ApiExitException("Steam " + ex.Message);
        }
        finally
        {
            if (webResponse != null)
                webResponse.Close();
        }
    }
}
