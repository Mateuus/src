using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Data;
using System.Text;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;
using System.Data.SqlClient;

public partial class api_LoginSessionPoller : WOApiWebPage
{
    void ReportFriendStatus(ref StringBuilder xml)
    {
        string CustomerID = web.CustomerID();

        SqlCommand sqcmd = new SqlCommand();
        sqcmd.CommandType = CommandType.StoredProcedure;
        sqcmd.CommandText = "WO_FriendGetStatus";
        sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);

        if (!CallWOApi(sqcmd))
            return;

        xml.Append("<friends>");

        xml.Append("<frstatus>");
        while (reader.Read())
        {
            xml.Append("<f ");
            xml.Append(xml_attr("ID", reader["FriendID"]));
            xml.Append(xml_attr("GT", reader["Gamertag"]));
            xml.Append(xml_attr("on", reader["Online"]));
            xml.Append(xml_attr("gsid", reader["GameSessionID"]));
            // if we have clan data
            if (reader["ClanID"] != null && reader["ClanID"].ToString() != "")
            {
                xml.Append(xml_attr("clname", reader["ClanName"]));
            }

            xml.Append("/>");
        }
        xml.Append("</frstatus>");

        reader.NextResult();
        xml.Append("<frreq>");
        while (reader.Read())
        {
            xml.Append("<f ");
            xml.Append(xml_attr("ID", reader["CustomerID"]));
            xml.Append(xml_attr("GT", reader["Gamertag"]));
            xml.Append(xml_attr("XP", reader["HonorPoints"]));
            // if we have clan data
            if (reader["ClanID"] != null && reader["ClanID"].ToString() != "")
            {
                xml.Append(xml_attr("clname", reader["ClanName"]));
            }
            xml.Append("/>");
        }
        xml.Append("</frreq>");

        xml.Append("</friends>");
    }

    void ClanGetInvites(ref StringBuilder xml)
    {
        string CustomerID = web.CustomerID();

        // report invites
        SqlCommand sqcmd = new SqlCommand();
        sqcmd.CommandType = CommandType.StoredProcedure;
        sqcmd.CommandText = "WO_ClanInviteGetInvitesForPlayer";
        sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);

        if (!CallWOApi(sqcmd))
            return;

        xml.Append("<clinvites>");
        while (reader.Read())
        {
            xml.Append("<inv ");
            // info about invite
            xml.Append(xml_attr("id", reader["ClanInviteID"]));
            xml.Append(xml_attr("gt", reader["Gamertag"]));
            // info about clan
            xml.Append(xml_attr("cname", reader["ClanName"]));
            xml.Append(xml_attr("cl", reader["ClanLevel"]));
            xml.Append(xml_attr("cem", reader["ClanEmblemID"]));
            xml.Append(xml_attr("cemc", reader["ClanEmblemColor"]));
            xml.Append(xml_attr("cm1", reader["MaxClanMembers"]));
            xml.Append(xml_attr("cm2", reader["NumClanMembers"]));
            xml.Append("/>\n");
        }
        xml.Append("</clinvites>");
    }

    void ClanGetApplications(ref StringBuilder xml)
    {
        string CustomerID = web.CustomerID();

        // report clan applications
        SqlCommand sqcmd = new SqlCommand();
        sqcmd.CommandType = CommandType.StoredProcedure;
        sqcmd.CommandText = "WO_ClanApplyGetList";
        sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);

        if (!CallWOApi(sqcmd))
            return;

        xml.Append("<clapps>");
        while (reader.Read())
        {
            xml.Append("<app ");
            xml.Append(xml_attr("id", reader["ClanApplicationID"]));
            xml.Append(xml_attr("note", reader["ApplicationText"]));
            xml.Append(xml_attr("gt", reader["Gamertag"]));
            xml.Append(xml_attr("xp", reader["HonorPoints"]));

            xml.Append(xml_attr("k", reader["Kills"]));
            xml.Append(xml_attr("d", reader["Deaths"]));
            xml.Append(xml_attr("w", reader["Wins"]));
            xml.Append(xml_attr("l", reader["Losses"]));
            xml.Append(xml_attr("tp", reader["TimePlayed"]));
            xml.Append("/>\n");
        }
        xml.Append("</clapps>");
    }

    void ReportClanStatus(ref StringBuilder xml)
    {
        string CustomerID = web.CustomerID();

        xml.Append("<clan>");

        int ClanID = 0;
        int ClanRank = 0;

        // get user clan info
        {
            SqlCommand sqcmd = new SqlCommand();
            sqcmd.CommandType = CommandType.StoredProcedure;
            sqcmd.CommandText = "WO_ClanGetPlayerData";
            sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);

            if (!CallWOApi(sqcmd))
                return;

            reader.Read();
            ClanID = getInt("ClanID");
            ClanRank = getInt("ClanRank");
        }

        if (ClanID == 0)
        {
            // if no clan yet
            xml.Append("<cldata ");
            xml.Append(xml_attr("ID", ClanID));
            xml.Append("/>\n");

            ClanGetInvites(ref xml);
        }
        else
        {
            // we have clan
            xml.Append("<cldata ");
            xml.Append(xml_attr("ID", ClanID));
            xml.Append(xml_attr("rank", ClanRank));
            xml.Append(xml_attr("cm1", reader["NumClanMembers"]));
            xml.Append(xml_attr("cm2", reader["MaxClanMembers"]));
            xml.Append("/>\n");

            // if officer, report applications
            if (ClanRank <= 1)
            {
                ClanGetApplications(ref xml);
            }
        }

        xml.Append("</clan>");
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

        if (GameSessionID != "0")
        {
            // skip friends - we're in game
            GResponse.Write("<?xml version=\"1.0\"?>\n");
            return;
        }

        StringBuilder xml = new StringBuilder();
        xml.Append("<?xml version=\"1.0\"?>\n");
        xml.Append("<ldata>");

        ReportFriendStatus(ref xml);
        ReportClanStatus(ref xml);

        xml.Append("</ldata>");
        GResponse.Write(xml.ToString());
    }
}
