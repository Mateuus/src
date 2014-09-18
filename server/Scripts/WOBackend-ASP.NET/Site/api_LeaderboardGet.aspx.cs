using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Data;
using System.Text;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;
using System.Data.SqlClient;

public partial class api_LeaderboardGet : WOApiWebPage
{
    protected override void Execute()
    {
        //this API is public
        //if (!WoCheckLoginSession())
        //    return;

        string CustomerID = web.CustomerID();
        string StartPos = web.Param("pos");
        string TableID = web.Param("t");

        SqlCommand sqcmd = new SqlCommand();
        sqcmd.CommandType = CommandType.StoredProcedure;
        sqcmd.CommandText = "WO_LeaderboardGet";
        sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);
        sqcmd.Parameters.AddWithValue("@in_TableID", TableID);
        sqcmd.Parameters.AddWithValue("@in_StartPos", StartPos);

        if (!CallWOApi(sqcmd))
            return;

        reader.Read();
        int CurPos = getInt("StartPos");
        int Size = getInt("Size");

        // report page of leaderboard
        StringBuilder xml = new StringBuilder();
        xml.Append("<?xml version=\"1.0\"?>\n");
        xml.Append(string.Format("<leaderboard pos=\"{0}\" size=\"{1}\">", CurPos, Size));

        reader.NextResult();
        while (reader.Read())
        {
            xml.Append("<f ");
            xml.Append(xml_attr("GT", reader["Gamertag"]));
            xml.Append(xml_attr("XP", reader["HonorPoints"]));
            xml.Append(xml_attr("k", reader["Kills"]));
            xml.Append(xml_attr("d", reader["Deaths"]));
            xml.Append(xml_attr("w", reader["Wins"]));
            xml.Append(xml_attr("l", reader["Losses"]));
            xml.Append(xml_attr("f", reader["ShotsFired"]));
            xml.Append(xml_attr("h", reader["ShotsHit"]));
            xml.Append(xml_attr("t", reader["TimePlayed"]));
            xml.Append(xml_attr("p", reader["HavePremium"]));
            xml.Append("/>");
        }
        xml.Append("</leaderboard>");

        GResponse.Write(xml.ToString());
    }
}
