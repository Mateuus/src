using System;
using System.Collections.Generic;
using System.Web;
using System.Data;
using System.Web.UI;
using System.Web.UI.WebControls;
using System.Data.SqlClient;
using System.IO;
using System.IO.Compression;
using System.Globalization;

/// <summary>
/// Summary description for WOApiWebPage
/// </summary>
public abstract class WOApiWebPage : System.Web.UI.Page
{
    protected SQLBase sql = new SQLBase();
    public SqlDataReader reader = null;

    protected WebHelper web = null;
    protected string LastIP = "0.0.0.0";

    protected GZIPResponse GResponse = new GZIPResponse();
    protected string SERVER_API_KEY = "CfFkqQWjfgksYG56893GDhjfjZ20";

    abstract protected void Execute();

    public WOApiWebPage()
	{
    }

    protected static string xml_attr(string name, Object var)
    {
        string xml = string.Format("{0}=\"{1}\"\n", name, HttpUtility.HtmlEncode(var.ToString()));
        return xml;
    }

    protected static void msWriteInt(MemoryStream ms, Int32 data)
    {
        Byte b1 = Convert.ToByte((data >> 0) & 0xff);
        Byte b2 = Convert.ToByte((data >> 8) & 0xff);
        Byte b3 = Convert.ToByte((data >> 16) & 0xff);
        Byte b4 = Convert.ToByte((data >> 24) & 0xff);
        ms.WriteByte(b1);
        ms.WriteByte(b2);
        ms.WriteByte(b3);
        ms.WriteByte(b4);
    }
    protected static void msWriteShort(MemoryStream ms, Int32 data)
    {
        Byte b1 = Convert.ToByte((data >> 0) & 0xff);
        Byte b2 = Convert.ToByte((data >> 8) & 0xff);
        ms.WriteByte(b1);
        ms.WriteByte(b2);
    }
    protected static void msWriteByte(MemoryStream ms, Int32 data)
    {
        Byte b1 = Convert.ToByte((data >> 0) & 0xff);
        ms.WriteByte(b1);
    }
    protected static void msWriteUtf8String(MemoryStream ms, string str)
    {
        System.Text.UTF8Encoding encoding = new System.Text.UTF8Encoding();
        Byte[] byteArray = encoding.GetBytes(str);

        msWriteShort(ms, byteArray.Length);
        foreach (Byte b in byteArray)
            ms.WriteByte(b);
    }

    public static byte[] GZipMemory(byte[] Buffer)
    {
        MemoryStream ms = new MemoryStream();
        GZipStream GZip = new GZipStream(ms, CompressionMode.Compress);

        GZip.Write(Buffer, 0, Buffer.Length);
        GZip.Close();

        byte[] Result = ms.ToArray();
        ms.Close();
        return Result;
    }

    public static byte[] InflateMemory(byte[] Buffer)
    {
        MemoryStream ms = new MemoryStream();
        DeflateStream GZip = new DeflateStream(ms, CompressionMode.Compress);

        GZip.Write(Buffer, 0, Buffer.Length);
        GZip.Close();

        byte[] Result = ms.ToArray();
        ms.Close();
        return Result;
    }


    protected int getInt(string name)
    {
        try
        {
            int var = Convert.ToInt32(reader[name]);
            return var;
        }
        catch (Exception)
        {
            throw new ApiExitException("bad int field " + name);
        }
    }

    protected string getString(string name)
    {
        try
        {
            return reader[name].ToString();
        }
        catch (Exception)
        {
            throw new ApiExitException("no field " + name);
        }
    }

    protected bool WoCheckLoginSession()
    {
        string CustomerID = web.CustomerID();
        string SessionID = web.SessionKey();

        /*
        // special GUID for server calls
        if (SessionKey == "{9CCA70CD-62C3-4341-A559-2EFD285B0FC0}")
        {
            return true;
        }*/

        SqlCommand sqcmd = new SqlCommand();
        sqcmd.CommandType = CommandType.StoredProcedure;
        sqcmd.CommandText = "WO_UPDATELOGINSESSION";
        sqcmd.Parameters.AddWithValue("@in_IP", LastIP);
        sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);
        sqcmd.Parameters.AddWithValue("@in_SessionID", SessionID);

        if (!CallWOApi(sqcmd))
            return false;

        return true;
    }

    public bool CallWOApi(SqlCommand sqcmd)
    {
        // need to close previous reader here, because of
        // "There is already an open DataReader associated with this Command which must be closed first"
        if (reader != null)
        {
            reader.Close();
            reader = null;
        }

        reader = sql.Select(sqcmd);
        if (reader == null)
            return false;
        reader.Read();

        try
        {
            string rm = "";
            try
            {
                rm = reader["ResultMsg"].ToString();
            }
            catch { }

            int rc = Convert.ToInt32(reader["ResultCode"]);
            if (rc != 0)
            {
                Response.Write("WO_" + rc + " " + rm);
                return false;
            }
        }
        catch (Exception)
        {
            throw new ApiExitException("ResultCode not set");
            //return false;
        }

        // move to actual result data
        reader.NextResult();
        return true;
    }

    protected void Page_Load(object sender, EventArgs _e)
    {
        try
        {
            // need to set culture for floating point separator! (need to be '.')
            System.Threading.Thread.CurrentThread.CurrentCulture = new CultureInfo("en-US");

            web = new WebHelper(this);
            LastIP = Request.UserHostAddress;

            sql.Connect();

            Response.ContentEncoding = System.Text.Encoding.UTF8;
            Execute();

            if (GResponse.Length > 0)
            {
                GResponse.Flush(Response);
            }
        }
        catch (ApiExitException e)
        {
            Response.Write("WO_5" + e.Message);
        }
        catch (Exception e)
        {
            Response.Write("WO_5" + e.ToString());
        }

        // close associated sql resources
        if(reader != null)
        {
            reader.Close();
            reader = null;
        }

        sql.Disconnect();
    }
}
