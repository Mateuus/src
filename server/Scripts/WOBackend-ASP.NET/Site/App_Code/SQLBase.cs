using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Web;
using System.Data.SqlClient;
using System.Diagnostics;
using System.Configuration;

/// <summary>
/// Summary description for SQLBase
/// </summary>
public class SQLBase
{
    string server = "localhost,1433";
    string user = "wz_api_user";
    string pass = "Mateuus.27";
    string workdb = "Warinc";

    SqlConnection conn_ = null;

    public SQLBase()
    {
        if (ConfigurationManager.AppSettings.Get("WO_Region") == "RU")
        {
            server = "localhost,1433";
            //server = "rudb1.thewarinc.com,11433";
            user = "wz_api_user";
            pass = "Mateuus.27";
        }
        else if (ConfigurationManager.AppSettings.Get("WO_Region") == "PH")
        {
            server = "localhost,1433";
            user = "wz_api_user";
            pass = "Mateuus.27";
        }
        else
        {
            server = "localhost,1433";
            user = "wz_api_user";
            pass = "Mateuus.27";
        }
    }

    ~SQLBase()
    {
        Disconnect();
    }

    public bool Connect()
    {
        if (pass.Length == 0)
            throw new ArgumentException("no password in sql");

        string str = String.Format(
            "user id={0};" +
            "password={1};" + 
            "server={2};" +
            "database={3};" +
            "Trusted_Connection=false;" +
            "connection timeout=30",
            user,
            pass,
            server,
            workdb
            );

        try
        {
            SqlConnection c = new SqlConnection(str);
            c.Open();

            conn_ = c;
        }
        catch (Exception)
        {
            throw new ApiExitException("SQL Connect failed");
            //return false;
        }

        return true;
    }

    public void Disconnect()
    {
        if (conn_ == null)
            return;

        try
        {
            conn_.Close();
        }
        catch { }
        conn_ = null;
    }

    void DumpSqlCommand(SqlCommand sqcmd)
    {
        Debug.WriteLine("CMD: " + sqcmd.CommandText);

        foreach (SqlParameter p in sqcmd.Parameters)
        {
            Debug.WriteLine(string.Format("{0}={1} ({2})",
                p.ParameterName, p.SqlValue, p.SqlDbType.ToString()));
        }

        return;
    }

    public SqlDataReader Select(SqlCommand sqcmd)
    {
        SqlDataReader reader = null;
        try
        {
            sqcmd.Connection = conn_;
            //DumpSqlCommand(sqcmd);
            reader = sqcmd.ExecuteReader();
            return reader;
        }
        catch (Exception e)
        {
#if true
            //@DEBUG
            throw new ApiExitException("SQL Select Failed: " + e.Message);
#else
            throw new ApiExitException("SQL");
#endif
            //return false;
        }
    }
}
