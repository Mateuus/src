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

public partial class api_SteamBuyGP : WOApiWebPage
{
    void GetShopData()
    {
        SqlCommand sqcmd = new SqlCommand();
        sqcmd.CommandType = CommandType.StoredProcedure;
        sqcmd.CommandText = "WO_SteamGetGPShop";
        if (!CallWOApi(sqcmd))
            return;

        StringBuilder xml = new StringBuilder();
        Response.ContentType = "text/xml";
        xml.Append("<?xml version=\"1.0\"?>\n");
        xml.Append("<SteamGPShop>");

        // steam gp items
        while (reader.Read())
        {
            xml.Append("<i ");
            xml.Append(xml_attr("ID", reader["SteamGPItemID"]));
            xml.Append(xml_attr("GP", reader["GP"]));
            xml.Append(xml_attr("BonusGP", reader["BonusGP"]));
            xml.Append(xml_attr("Price", reader["PriceCentsUSD"]));
            xml.Append("/>");
        }

        xml.Append("</SteamGPShop>");
        Response.Write(xml.ToString());
    }

    bool ParseResponse(SteamXML.MicroTnxResponse resp)
    {
        if (resp.error != null)
            throw new ApiExitException(string.Format("resp.error: {0} {1}", resp.error.errorcode, resp.error.errordesc));
        
        if (resp.params_ == null)
            throw new ApiExitException("no resp.params");

        if (resp.result != "OK")
            throw new ApiExitException("bad resp.result " + resp.params_.status);

        return true;
    }

    void AuthTransaction()
    {
        string CustomerID = web.CustomerID();
        string steamId = web.Param("steamId");
        string gpItemId = web.Param("gpItemId");
        string country = web.Param("country");

        SqlCommand sqcmd = new SqlCommand();
        sqcmd.CommandType = CommandType.StoredProcedure;
        sqcmd.CommandText = "WO_SteamStartOrder";
        sqcmd.Parameters.AddWithValue("@in_IP", LastIP);
        sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);
        sqcmd.Parameters.AddWithValue("@in_SteamID", steamId);
        sqcmd.Parameters.AddWithValue("@in_SteamGPItemId", gpItemId);
        if (!CallWOApi(sqcmd))
            return;

        reader.Read();
        string GP = getString("GP");
        string PriceUSDCents = getString("PriceUSD");
        string OrderID = getString("OrderID");
        string ItemName = getString("ItemName");
        if (ItemName == null || ItemName == "")
            ItemName = string.Format("{0} GP", GP);

        SteamApi api = new SteamApi();
        SteamXML.MicroTnxResponse resp = api.InitTxn(
            OrderID, steamId, 
            "USD", PriceUSDCents, 
            gpItemId, ItemName, 
            "EN");
        //System.Diagnostics.Debug.WriteLine("@@@InitTxn: " + api.lastData_);
        if (!ParseResponse(resp))
            return;

        Response.Write("WO_0");
    }

    void FinalizeTransaction()
    {
        string orderId = web.Param("orderId");
        string CustomerID = web.CustomerID();

        // check that transaction was approved and get it transactionID
        SteamApi api = new SteamApi();
        SteamXML.MicroTnxResponse resp = api.QueryTxn(orderId);
        //System.Diagnostics.Debug.WriteLine("@@@QueryTxn: " + api.lastData_);
        if (!ParseResponse(resp))
            return;
        if (resp.params_.status != "Approved")
        {
            Response.Write("WO_5");
            Response.Write(string.Format("bad status: {0}", resp.params_.status));
            return;
        }

        // finalize transaction in steam. NOTE: fucking steam give http error instead of valid answer here
        resp = api.FinalizeTxn(orderId);
        //System.Diagnostics.Debug.WriteLine("@@@FinalizeTxn: " + api.lastData_);
        if (!ParseResponse(resp))
            return;

        // note: we can't pass transaction id as bigint here, because stream using *unsigned* int64.
        string transid = resp.params_.transid;

        // finalize transaction in DB and get new balance
        SqlCommand sqcmd = new SqlCommand();
        sqcmd.CommandType = CommandType.StoredProcedure;
        sqcmd.CommandText = "WO_SteamFinishOrder";
        sqcmd.Parameters.AddWithValue("@in_IP", LastIP);
        sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);
        sqcmd.Parameters.AddWithValue("@in_OrderID", orderId);
        sqcmd.Parameters.AddWithValue("@in_transid", transid);
        if (!CallWOApi(sqcmd))
            return;

        // return new GP balance
        reader.Read();
        int balance = getInt("Balance");
        Response.Write("WO_0");
        Response.Write(balance.ToString());
    }

    protected override void Execute()
    {
        if (ConfigurationManager.AppSettings.Get("WO_Region") == "RU")
            throw new ApiExitException("bad region");

        if (!WoCheckLoginSession())
            return;

        string func = web.Param("func");
        if (func == "shop")
            GetShopData();
        else if (func == "auth")
            AuthTransaction();
        else if (func == "fin")
            FinalizeTransaction();
        else
            throw new ApiExitException("bad func");
    }
}
