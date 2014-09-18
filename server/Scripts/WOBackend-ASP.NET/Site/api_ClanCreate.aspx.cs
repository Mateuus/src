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

public partial class api_ClanCreate : WOApiWebPage
{
    int IsNeedMoney()
    {
        string CustomerID = web.CustomerID();

        SqlCommand sqcmd = new SqlCommand();
        sqcmd.CommandType = CommandType.StoredProcedure;
        sqcmd.CommandText = "WO_ClanCreateCheckMoney";
        sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);
        
        if (!CallWOApi(sqcmd))
            throw new ApiExitException("IsNeedMoney");

        reader.Read();
        int NeedMoney = getInt("NeedMoney");
        return NeedMoney;
    }

    void CheckIfNeedMoney()
    {
        int NeedMoney = IsNeedMoney();
        Response.Write("WO_0");
        Response.Write(string.Format("{0}", NeedMoney));
    }

    void CreateClan()
    {
        string CustomerID = web.CustomerID();
        string ClanName = web.Param("ClanName");
        string ClanNameColor = web.Param("ClanNameColor");
        string ClanTag = web.Param("ClanTag");
        string ClanTagColor = web.Param("ClanTagColor");
        string ClanEmblemID = web.Param("ClanEmblemID");
        string ClanEmblemColor = web.Param("ClanEmblemColor");

        // see if we can create clan without money
        {
            if (ClanName.Length < 1)
                throw new ApiExitException("ClanName too small");
            if (ClanTag.Length < 1)
                throw new ApiExitException("ClanTag too small");

            SqlCommand sqcmd = new SqlCommand();
            sqcmd.CommandType = CommandType.StoredProcedure;
            sqcmd.CommandText = "WO_ClanCreateCheckParams";
            sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);
            sqcmd.Parameters.AddWithValue("@in_ClanName", ClanName);
            sqcmd.Parameters.AddWithValue("@in_ClanTag", ClanTag);

            if (!CallWOApi(sqcmd))
                return;
        }

        int NeedMoney = IsNeedMoney();
        int balance = 0;
        if (NeedMoney > 0)
        {
            int CLAN_CREATE_ITEMID = 301143;

            // permanent real $ buying
            string BuyIdx = "4";
            if (ConfigurationManager.AppSettings.Get("WO_Region") == "RU")
                BuyIdx = "12";

            SqlCommand sqcmd = new SqlCommand();
            sqcmd.CommandType = CommandType.StoredProcedure;
            sqcmd.CommandText = BuyItem3.GetBuyProcFromIdx(BuyIdx);
            sqcmd.Parameters.AddWithValue("@in_IP", LastIP);
            sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);
            sqcmd.Parameters.AddWithValue("@in_ItemId", CLAN_CREATE_ITEMID);
            sqcmd.Parameters.AddWithValue("@in_BuyDays", 2000);

            if (!CallWOApi(sqcmd))
                return;

            reader.Read();
            balance = getInt("Balance");
        }

        // actually create the clan
        int ClanID = 0;
        {
            SqlCommand sqcmd = new SqlCommand();
            sqcmd.CommandType = CommandType.StoredProcedure;
            sqcmd.CommandText = "WO_ClanCreate";
            sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);
            sqcmd.Parameters.AddWithValue("@in_ClanName", ClanName);
            sqcmd.Parameters.AddWithValue("@in_ClanNameColor", ClanNameColor);
            sqcmd.Parameters.AddWithValue("@in_ClanTag", ClanTag);
            sqcmd.Parameters.AddWithValue("@in_ClanTagColor", ClanTagColor);
            sqcmd.Parameters.AddWithValue("@in_ClanEmblemID", ClanEmblemID);
            sqcmd.Parameters.AddWithValue("@in_ClanEmblemColor", ClanEmblemColor);

            if (!CallWOApi(sqcmd))
                return;

            reader.Read();
            ClanID = getInt("ClanID");
        }

        Response.Write("WO_0");
        Response.Write(string.Format("{0} {1}", ClanID, balance));
    }

    protected override void Execute()
    {
        if (!WoCheckLoginSession())
            return;

        string func = web.Param("func");
        if (func == "check1")
            CheckIfNeedMoney();
        else if (func == "create")
            CreateClan();
        else
            throw new ApiExitException("bad func");
    }
}
