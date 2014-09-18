using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Data;
using System.Text;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;
using System.Data.SqlClient;
using System.Configuration;

public partial class api_Login : WOApiWebPage
{
    protected override void Execute()
    {
        string username = web.Param("username");
        string password = web.Param("password");

        SqlCommand sqcmd = new SqlCommand();
        sqcmd.CommandType = CommandType.StoredProcedure;
        sqcmd.CommandText = "WO_LOGIN";
        sqcmd.Parameters.AddWithValue("@in_IP", LastIP);
        sqcmd.Parameters.AddWithValue("@in_Username", username);
        sqcmd.Parameters.AddWithValue("@in_Password", password);

        if (!CallWOApi(sqcmd))
            return;

        reader.Read();
        int CustomerID = getInt("CustomerID"); ;
        int AccountStatus = getInt("AccountStatus");
        int SessionID = 0;
        int IsDeveloper = 0;

        if (CustomerID > 0)
        {
            SessionID = getInt("SessionID");
            IsDeveloper = getInt("IsDeveloper");
        }

        // region lock
        if (IsDeveloper == 0 && IPLocationCheck.IsRegionLocked(LastIP, CustomerID))
        {
            // special 600 code meaning we're IP locked.
            AccountStatus = 600;
        }

        Response.Write("WO_0");
        Response.Write(string.Format("{0} {1} {2}",
            CustomerID, SessionID, AccountStatus));
    }
}
