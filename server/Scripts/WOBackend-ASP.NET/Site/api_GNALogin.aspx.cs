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

public partial class api_GNALogin : WOApiWebPage
{
    void ManualAccCreate_GNALogin(string userId, string nickname)
    {
        //
        // step 2 - auth/create user at game
        //
        SqlCommand sqcmd = new SqlCommand();
        sqcmd.CommandType = CommandType.StoredProcedure;
        sqcmd.CommandText = "WO_GNALogin";
        sqcmd.Parameters.AddWithValue("@in_IP", LastIP);
        sqcmd.Parameters.AddWithValue("@in_GNAUserId", userId);
        sqcmd.Parameters.AddWithValue("@in_GNANick", nickname);
        if (!CallWOApi(sqcmd))
            return;
    }

    void ManualAccCreate_Exec()
    {
        string accBase = "wcg";
        string[] accIds = new string[]{
            "400001000003861400",
        };

        for(int i=0; i<accIds.Length; i++)
        {
            string nickname = accBase + (i + 1).ToString();
            ManualAccCreate_GNALogin(accIds[i], nickname);
            Response.Write(string.Format("created {0} {1}<br>", accIds[i], nickname));
        }
    }


    protected override void Execute()
    {
        if (ConfigurationManager.AppSettings.Get("WO_Region") != "RU")
            throw new ApiExitException("bad region");

        string userId = web.Param("userId");
        string appKey = web.Param("appKey");
        string token = web.Param("token");

        //string userId = "400001000001726560";
        //string appKey = "86301ad7903d76036a33ccd608fe052c50fc20d4";
        //string token = "8ea5516da6bec0bc96cda5fb44fd184bf6cb5c43";

        //
        // step1 - auth user at gamenet
        // 
        GNAApi gna = new GNAApi();
        GNAXML.getServiceToken resp1 = gna.getServiceToken(userId);
        if (resp1.error != null)
        {
            // login failed
            Response.Write("WO_0");
            Response.Write(string.Format("{0} {1} {2} {3} {4}",
                0, 0, 0, resp1.error.code, resp1.error.message));
            return;
        }

        // at this point, we must have resp1 data
        if (resp1.token != token)
        {
            // auth fail if tokens is not the same
            Response.Write("WO_0");
            Response.Write(string.Format("{0} {1} {2} {3} bad_token",
                0, 0, 0, resp1.token));
            return;
        }

        //
        // check subscription for game
        //
        GNAXML.checkSubscribeStatus resp2 = gna.checkSubscribeStatus(userId);
        if(resp2.status != "free" && resp2.status != "premium" && resp2.status != "trial")
        {
            // account is blocked
            Response.Write("WO_0");
            Response.Write(string.Format("{0} {1} {2} {3}",
                1, 200, 0, resp2.status));
            return;
        }

        //
        // get user info for nickname
        //
        GNAXML.getShortInfo resp3 = gna.getShortInfo(userId, appKey);
        string nickname = resp3.userInfo.nickname;
        if (nickname.Length > 31)
            nickname = nickname.Substring(0, 31);

        //
        // step 2 - auth/create user at game
        //
        SqlCommand sqcmd = new SqlCommand();
        sqcmd.CommandType = CommandType.StoredProcedure;
        sqcmd.CommandText = "WO_GNALogin";
        sqcmd.Parameters.AddWithValue("@in_IP", LastIP);
        sqcmd.Parameters.AddWithValue("@in_GNAUserId", userId);
        sqcmd.Parameters.AddWithValue("@in_GNANick", nickname);
        if (!CallWOApi(sqcmd))
            return;

        reader.Read();
        int CustomerID = getInt("CustomerID");
        int AccountStatus = getInt("AccountStatus");
        int SessionID = getInt("SessionID");

        Response.Write("WO_0");
        Response.Write(string.Format("{0} {1} {2}",
            CustomerID, SessionID, AccountStatus));
    }
}
