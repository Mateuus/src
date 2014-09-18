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

public partial class api_Gamersfirst : WOApiWebPage
{
    void LoginGamersfirst()
    {
        string token = web.Param("token");

        // get id from auth ticket
        GamerstirstApi api = new GamerstirstApi();
        Dictionary<string, string> dict = api.AuthenticateWithToken(token);
        if (dict == null || dict["Result"] == null || dict["Result"] != "0")
        {
            // auth failed
            Response.Write("WO_5");
            Response.Write(api.lastData_);
            return;
        }

        string g1Id = dict["AccountId"];
        string g1PayCode = dict["PremiumCode"];
        string g1UserName = dict["Username"];

        // try to login user based on his Gamersfirst ID
        SqlCommand sqcmd = new SqlCommand();
        sqcmd.CommandType = CommandType.StoredProcedure;
        sqcmd.CommandText = "WO_G1Login";
        sqcmd.Parameters.AddWithValue("@in_IP", LastIP);
        sqcmd.Parameters.AddWithValue("@in_G1ID", g1Id);

        if (!CallWOApi(sqcmd))
            return;

        reader.Read();
        int CustomerID = getInt("CustomerID"); ;
        int AccountStatus = getInt("AccountStatus");
        int SessionID = getInt("SessionID");

        Response.Write("WO_0");
        Response.Write(string.Format("{0} {1} {2} {3} {4} :{5}",
            CustomerID, SessionID, AccountStatus,
            g1Id, g1PayCode, g1UserName));

        return;
    }

    string DetectAccountClassification(string g1Id)
    {
        try
        {
            GamerstirstApi api = new GamerstirstApi();
            Dictionary<string, string> dict = api.GetAccountClassification(g1Id);
            if (dict == null || dict["Result"] == null)
            {
                return "WR_API_FAIL";
            }
            if (dict["Result"] != "0")
            {
                return "WR_NONE";
            }

            string Classification = dict["Classification"];
            return Classification;
        }
        catch
        {
            // this call can fail and we can live with it
            return "WR_HTTP_FAIL";
        }
    }

    int GetPayCodeFromClassification(string Classification)
    {
        switch (Classification)
        {
            case "WRPaidTier5": return 1;
            case "WRPaidTier4": return 2;
            case "WRPaidTier3": return 3;
            case "WRPaidTier2": return 4;
            case "WRPaidTier1": return 5;

            case "WRFreeTier4": return 6;
            case "WRFreeTier3": return 7;
            case "WRFreeTier2": return 8;
            case "WRFreeTier1": return 9;

            default: return 10;
        }
    }

    void CreateGamersfirstAccount()
    {
        string username = web.Param("username");
        string password = web.Param("password");
        string email = web.Param("email");
        string g1Id = web.Param("g1Id");

        string g1PayClass = DetectAccountClassification(g1Id);
        int g1PayCode = GetPayCodeFromClassification(g1PayClass);

        // create acc
        SqlCommand sqcmd = new SqlCommand();
        sqcmd.CommandType = CommandType.StoredProcedure;
        sqcmd.CommandText = "ECLIPSE_CREATEACCOUNT";
        sqcmd.Parameters.AddWithValue("@in_IP", LastIP);
        sqcmd.Parameters.AddWithValue("@in_Username", username);
        sqcmd.Parameters.AddWithValue("@in_Password", password);
        sqcmd.Parameters.AddWithValue("@in_Email", email);
        sqcmd.Parameters.AddWithValue("@in_Reg_SID", string.Format("G1AccClass {0} to {1}", g1PayClass, g1PayCode));
        sqcmd.Parameters.AddWithValue("@in_ReferralID", "1289081317");

        if (!CallWOApi(sqcmd))
            return;

        // ok, account creation successed
        Response.Write("WO_0");
        reader.Read();
        string CustomerID = getString("CustomerID");

        // link gamersfirst id
        {
            sqcmd = new SqlCommand();
            sqcmd.CommandType = CommandType.StoredProcedure;
            sqcmd.CommandText = "WO_G1LinkAccount";
            sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);
            sqcmd.Parameters.AddWithValue("@in_G1ID", g1Id);
            sqcmd.Parameters.AddWithValue("@in_G1PayCode", g1PayCode);

            CallWOApi(sqcmd);
        }

        CreateForumAcc.Create(username, password, email);
        return;
    }

    protected override void Execute()
    {
        string func = web.Param("func");
        if (func == "login")
            LoginGamersfirst();
        else if (func == "create")
            CreateGamersfirstAccount();
        else
            throw new ApiExitException("bad func");

        return;
    }
}
