using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Data;
using System.Text;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;
using System.Data.SqlClient;

public partial class api_SteamLogin : WOApiWebPage
{
    protected override void Execute()
    {
        string ticket = web.Param("ticket");

        // get steam user id from auth ticket
        SteamApi api = new SteamApi();
        string SteamID = api.GetSteamId(ticket);
        if(SteamID.Length == 0)
        {
            Response.Write("WO_5");
            Response.Write(api.lastData_);
            return;
        }

        // try to login user based on his steamID
        SqlCommand sqcmd = new SqlCommand();
        sqcmd.CommandType = CommandType.StoredProcedure;
        sqcmd.CommandText = "WO_SteamLogin";
        sqcmd.Parameters.AddWithValue("@in_IP", LastIP);
        sqcmd.Parameters.AddWithValue("@in_SteamID", SteamID);

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
        return;
    }
}
