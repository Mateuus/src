using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Data;
using System.Text;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;
using System.Data.SqlClient;

public partial class api_WelcomePackage4 : WOApiWebPage
{
    void CheckReferralLinks()
    {
        // disabled in russian
        if (System.Configuration.ConfigurationManager.AppSettings.Get("WO_Region") == "RU")
            return;

        string CustomerID = web.CustomerID();

        string ReferralID = "";
        string reg_sid = "";
        string pixelUrl = "";

        // get pixel info
        {
            SqlCommand sqcmd = new SqlCommand();
            sqcmd.CommandType = CommandType.StoredProcedure;
            sqcmd.CommandText = "ECLIPSE_ReferralsGetUserInfo";
            sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);

            if (!CallWOApi(sqcmd))
                return;

            reader.Read();
            ReferralID = reader["ReferralID"].ToString();
            reg_sid = reader["reg_sid"].ToString();
        }

        if (ReferralID == "1288929113")  // REFERID_radone1
        {
            pixelUrl = "http://panel.gwallet.com/network-node/postback/earthlink?sid=" + reg_sid;
        }

        if (ReferralID == "1288987160")	// REFERID_tokenads
        {
            pixelUrl = string.Format(
                "http://offers.tokenads.com/pixel/401?remote_id={0}&tid={1}&amount=1&cur=USD",
                CustomerID, reg_sid);
        }

        if (ReferralID == "1288932284")  // REFERID_cpmstar
        {
            pixelUrl = string.Format(
                "https://server.cpmstar.com/action.aspx?advertiserid=2133&gif=1");
        }

        if (pixelUrl == "")
            return;

        // call pixel
        string ErrorMsg = "";
        try
        {
            System.Net.WebClient client = new System.Net.WebClient();
            client.DownloadString(pixelUrl);
        }
        catch (Exception e)
        {
            ErrorMsg = e.Message;
        }

        /*
        //log to our server for test
        try
        {
            //client.DownloadString("https://api1.thewarinc.com/php/qq2.php?id=" + pixelUrl);
        }
        catch { }
        */

        // log pixel call result
        {
            SqlCommand sqcmd = new SqlCommand();
            sqcmd.CommandType = CommandType.StoredProcedure;
            sqcmd.CommandText = "ECLIPSE_ReferralsLogPixel";
            sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);
            sqcmd.Parameters.AddWithValue("@in_ReferralID", ReferralID);
            sqcmd.Parameters.AddWithValue("@in_PixelURL", pixelUrl);
            sqcmd.Parameters.AddWithValue("@in_ErrorMsg", ErrorMsg);

            if (!CallWOApi(sqcmd))
                return;
        }
    }

    protected override void Execute()
    {
        if (!WoCheckLoginSession())
            return;

        // here we'll see if user need to fire server-to-server registration pixel
        try
        {
            CheckReferralLinks();
        }
        catch { }

        string CustomerID = web.CustomerID();
        string Class = web.Param("specID");

        SqlCommand sqcmd = new SqlCommand();
        sqcmd.CommandType = CommandType.StoredProcedure;
        sqcmd.CommandText = "WO_WelcomePackage4";
        sqcmd.Parameters.AddWithValue("@in_IP", LastIP);
        sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);
        sqcmd.Parameters.AddWithValue("@in_Class", Class);

        if (!CallWOApi(sqcmd))
            return;

        Response.Write("WO_0");
    }
}
