using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;
using System.Text;

/// <summary>
/// Summary description for WebHelper
/// </summary>
public class WebHelper
{
    System.Web.UI.Page page_;
	public WebHelper(System.Web.UI.Page in_page)
	{
        page_ = in_page;
	}

    /// <summary>
    /// Converts the String to UTF8 Byte array and is used in De serialization
    /// </summary>
    /// <param name="pXmlString"></param>
    /// <returns></returns>
    public static Byte[] StringToUTF8ByteArray(String pXmlString)
    {
        UTF8Encoding encoding = new UTF8Encoding();
        Byte[] byteArray = encoding.GetBytes(pXmlString);
        return byteArray;
    }

    public static string CalculateMD5Hash(string input)
    {
        // step 1, calculate MD5 hash from input
        System.Security.Cryptography.MD5 md5 = System.Security.Cryptography.MD5.Create();
        byte[] inputBytes = System.Text.Encoding.ASCII.GetBytes(input);
        byte[] hash = md5.ComputeHash(inputBytes);

        // step 2, convert byte array to hex string
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < hash.Length; i++)
        {
            sb.Append(hash[i].ToString("X2"));
        }
        return sb.ToString();
    }

    public string Param(string paramName)
    {
        NameValueCollection nvc = page_.Request.Form; 
        string value = nvc[paramName];

        // enable GET params as well
        if (value == null)
        {
            nvc = page_.Request.QueryString;
            value = nvc[paramName];
        }

        if (value == null)
        {
#if true
            //@DEBUG
            throw new ApiExitException("no parameter " + paramName);
#else
            throw new ApiExitException("no parameter");
#endif
        }
        return value;
    }

    public string SessionKey()
    {
        return Param("s_key");
    }

    public string CustomerID()
    {
        return Param("s_id");
    }
}
