using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Data;
using System.Text;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;
using System.Data.SqlClient;

public partial class api_SrvGiveItem : WOApiWebPage
{
    protected override void Execute()
    {
        string skey1 = web.Param("skey1");
        if (skey1 != SERVER_API_KEY)
            throw new ApiExitException("bad key");

        string CustomerID = web.CustomerID();
        string ItemID = web.Param("ItemID");
        string BuyDays = web.Param("BuyDays");

        SqlCommand sqcmd = new SqlCommand();
        sqcmd.CommandType = CommandType.StoredProcedure;
        sqcmd.CommandText = "WO_SRV_GIVEITEM";
        sqcmd.Parameters.AddWithValue("@in_IP", LastIP);
        sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);
        sqcmd.Parameters.AddWithValue("@in_ItemId", ItemID);
        sqcmd.Parameters.AddWithValue("@in_BuyDays", BuyDays);

        // additional protection of sql proc
        sqcmd.Parameters.AddWithValue("@in_skey2", "ACsR4x23GsjYU*476xnDvYXK@!56");

        if (!CallWOApi(sqcmd))
            return;

        Response.Write("WO_0");
    }
}
