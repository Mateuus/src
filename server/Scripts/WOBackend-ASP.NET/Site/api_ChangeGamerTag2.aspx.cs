using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Data;
using System.Text;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;
using System.Data.SqlClient;

public partial class api_ChangeGamerTag2 : WOApiWebPage
{
    bool ValidateGamertag(string gamertag)
    {
        if (gamertag.Length > 16)
            return false;

        foreach (char c in gamertag)
        {
            if (!Char.IsLetterOrDigit(c))
                return false;
        }
        return true;
    }

    protected override void Execute()
    {
        if (!WoCheckLoginSession())
            return;

        string CustomerID = web.CustomerID();
        string ItemID = web.Param("ItemID");
        string BuyIdx = web.Param("BuyIdx");
        string gametag = web.Param("gametag");

        if (!ValidateGamertag(gametag))
        {
            Response.Write("WO_4");
            return;
        }

        // check if gamer tag exists
        {
            SqlCommand sqcmd = new SqlCommand();
            sqcmd.CommandType = CommandType.StoredProcedure;
            sqcmd.CommandText = "WO_ChangeGamertag";
            sqcmd.Parameters.AddWithValue("@in_IP", LastIP);
            sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);
            sqcmd.Parameters.AddWithValue("@in_GamerTag", gametag);
            sqcmd.Parameters.AddWithValue("@in_ActualExec", 0);

            if (!CallWOApi(sqcmd))
                return;
        }

        int balance = 0;
        // try to buy
        {
            SqlCommand sqcmd = new SqlCommand();
            sqcmd.CommandType = CommandType.StoredProcedure;
            sqcmd.CommandText = BuyItem3.GetBuyProcFromIdx(BuyIdx);
            sqcmd.Parameters.AddWithValue("@in_IP", LastIP);
            sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);
            sqcmd.Parameters.AddWithValue("@in_ItemId", ItemID);
            sqcmd.Parameters.AddWithValue("@in_BuyDays", BuyItem3.GetBuyDaysFromIdx(BuyIdx));

            if (!CallWOApi(sqcmd))
                return;

            reader.Read();
            balance = getInt("Balance");
        }

        // ok, change gamer tag
        {
            SqlCommand sqcmd = new SqlCommand();
            sqcmd.CommandType = CommandType.StoredProcedure;
            sqcmd.CommandText = "WO_ChangeGamertag";
            sqcmd.Parameters.AddWithValue("@in_IP", LastIP);
            sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);
            sqcmd.Parameters.AddWithValue("@in_GamerTag", gametag);
            sqcmd.Parameters.AddWithValue("@in_ActualExec", 1);

            if (!CallWOApi(sqcmd))
                return;
        }

        Response.Write("WO_0");
        Response.Write(string.Format("{0}", balance));
    }
}
