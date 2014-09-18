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

public partial class api_BuyItem3 : WOApiWebPage
{
    protected override void Execute()
    {
        if (!WoCheckLoginSession())
            return;

        string CustomerID = web.CustomerID();
        string ItemID = web.Param("ItemID");
        string BuyIdx = web.Param("BuyIdx");

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
        int balance = getInt("Balance");

        Response.Write("WO_0");
        Response.Write(string.Format("{0}", balance));
    }
}
