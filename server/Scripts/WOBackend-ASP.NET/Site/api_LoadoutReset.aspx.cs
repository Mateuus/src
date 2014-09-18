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

public partial class api_LoadoutReset : WOApiWebPage
{
    protected override void Execute()
    {
        if (!WoCheckLoginSession())
            return;

        string CustomerID = web.CustomerID();
        string LoadoutID = web.Param("LoadoutID");
        string Class = web.Param("Class");

        int balance = 0;
        // try to buy
        {
            int LOADOUT_RESET_ITEMID = 301150;    // special item for loadout reset

            // permanent real $ buying
            string BuyIdx = "4";
            if (ConfigurationManager.AppSettings.Get("WO_Region") == "RU")
                BuyIdx = "12";

            SqlCommand sqcmd = new SqlCommand();
            sqcmd.CommandType = CommandType.StoredProcedure;
            sqcmd.CommandText = BuyItem3.GetBuyProcFromIdx(BuyIdx);
            sqcmd.Parameters.AddWithValue("@in_IP", LastIP);
            sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);
            sqcmd.Parameters.AddWithValue("@in_ItemId", LOADOUT_RESET_ITEMID);
            sqcmd.Parameters.AddWithValue("@in_BuyDays", BuyItem3.GetBuyDaysFromIdx(BuyIdx));

            if (!CallWOApi(sqcmd))
                return;

            reader.Read();
            balance = getInt("Balance");
        }

        // ok, reset loadout
        {
            SqlCommand sqcmd = new SqlCommand();
            sqcmd.CommandType = CommandType.StoredProcedure;
            sqcmd.CommandText = "WO_LoadoutResetClass";
            sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);
            sqcmd.Parameters.AddWithValue("@in_LoadoutID", LoadoutID);
            sqcmd.Parameters.AddWithValue("@in_Class", Class);

            if (!CallWOApi(sqcmd))
                return;
        }

        Response.Write("WO_0");
    }
}
