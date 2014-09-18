using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Data;
using System.Text;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;
using System.Data.SqlClient;

public partial class api_UpdateLoginSession : WOApiWebPage
{
    void ReportFriendStatus()
    {
        string CustomerID = web.CustomerID();

        SqlCommand sqcmd = new SqlCommand();
        sqcmd.CommandType = CommandType.StoredProcedure;
        sqcmd.CommandText = "WO_FriendGetStatus";
        sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);

        if (!CallWOApi(sqcmd))
            return;

        StringBuilder xml = new StringBuilder();
        xml.Append("<?xml version=\"1.0\"?>\n");
        xml.Append("<frstatus>");

        xml.Append("<friends>");
        while (reader.Read())
        {
            xml.Append("<f ");
            xml.Append(xml_attr("ID", reader["FriendID"]));
            xml.Append(xml_attr("GT", reader["Gamertag"]));
            xml.Append(xml_attr("on", reader["Online"]));
            xml.Append(xml_attr("gsid", reader["GameSessionID"]));
            xml.Append("/>");
        }
        xml.Append("</friends>");

        reader.NextResult();
        xml.Append("<pending>");
        while (reader.Read())
        {
            xml.Append("<f ");
            xml.Append(xml_attr("ID", reader["CustomerID"]));
            xml.Append(xml_attr("GT", reader["Gamertag"]));
            xml.Append(xml_attr("XP", reader["HonorPoints"]));
            xml.Append("/>");
        }
        xml.Append("</pending>");

        xml.Append("</frstatus>");

        GResponse.Write(xml.ToString());
    }

    protected override void Execute()
    {
        string CustomerID = web.CustomerID();
        string SessionID = web.SessionKey();
        string GameSessionID = web.Param("GSID");

        SqlCommand sqcmd = new SqlCommand();
        sqcmd.CommandType = CommandType.StoredProcedure;
        sqcmd.CommandText = "WO_UpdateLoginSession2";
        sqcmd.Parameters.AddWithValue("@in_IP", LastIP);
        sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);
        sqcmd.Parameters.AddWithValue("@in_SessionID", SessionID);
        sqcmd.Parameters.AddWithValue("@in_GameSessionID", GameSessionID);

        if (!CallWOApi(sqcmd))
            return;

        GResponse.Write("WO_0");

        if (GameSessionID == "0")
        {
            ReportFriendStatus();
        }
        else
        {
            // skip friends - we're in game
            GResponse.Write("<?xml version=\"1.0\"?>\n");
        }
    }
}
