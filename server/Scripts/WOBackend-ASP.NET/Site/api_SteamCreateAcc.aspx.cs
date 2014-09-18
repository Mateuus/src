using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Data;
using System.Text;
using System.Net;
using System.IO;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;
using System.Data.SqlClient;

public partial class api_SteamCreateAcc : WOApiWebPage
{
    string CustomerID = "";
    string username;
    string password;
    string email;
    string steamId;

    bool CreateAccount()
    {
        // create acc
        SqlCommand sqcmd = new SqlCommand();
        sqcmd.CommandType = CommandType.StoredProcedure;
        sqcmd.CommandText = "ECLIPSE_CREATEACCOUNT";
        sqcmd.Parameters.AddWithValue("@in_IP", LastIP);
        sqcmd.Parameters.AddWithValue("@in_Username", username);
        sqcmd.Parameters.AddWithValue("@in_Password", password);
        sqcmd.Parameters.AddWithValue("@in_Email", email);
        sqcmd.Parameters.AddWithValue("@in_Reg_SID", "");
        sqcmd.Parameters.AddWithValue("@in_ReferralID", "1288964929");

        if (!CallWOApi(sqcmd))
            return false;

        // ok, account creation successed
        Response.Write("WO_0");

        reader.Read();
        CustomerID = getString("CustomerID");

        return true;
    }

    void LinkSteamId()
    {
        // link steam account to customer id
        SqlCommand sqcmd = new SqlCommand();
        sqcmd.CommandType = CommandType.StoredProcedure;
        sqcmd.CommandText = "WO_SteamLinkAccount";
        sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);
        sqcmd.Parameters.AddWithValue("@in_SteamID", steamId);

        CallWOApi(sqcmd);
    }

    protected override void Execute()
    {
        username = web.Param("username");
        password = web.Param("password");
        email = web.Param("email");
        steamId = web.Param("steamId");

        if (!CreateAccount())
            return;

        LinkSteamId();
        CreateForumAcc.Create(username, password, email);

        return;
    }
}
