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

public partial class api_GetProfile4 : WOApiWebPage
{
    void AddFPSAttachInventory(ref StringBuilder xml)
    {
        // move to next select, where data should be
        reader.NextResult();

        xml.Append("<fpsattach>\n");
        while (reader.Read())
        {
            xml.Append("<i ");
            xml.Append(xml_attr("wi", reader["WeaponID"]));
            xml.Append(xml_attr("ai", reader["AttachmentID"]));
            xml.Append(xml_attr("ml", reader["MinutesLeft"]));

            int IsEquipped = getInt("IsEquipped");
            if (IsEquipped > 0) xml.Append(xml_attr("eq", 1));

            xml.Append("/>");
        }
        xml.Append("</fpsattach>\n");
    }

    void AddLoadouts(ref StringBuilder xml)
    {
        // move to next select, where data should be
        reader.NextResult();

        xml.Append("<loadouts>\n");
        while (reader.Read())
        {
            xml.Append("<l ");
            xml.Append(xml_attr("id", reader["LoadoutID"]));
            xml.Append(xml_attr("cl", reader["Class"]));
            xml.Append(xml_attr("xp", reader["HonorPoints"]));
            xml.Append(xml_attr("tm", reader["TimePlayed"]));
            xml.Append(xml_attr("lo", reader["Loadout"]));
            xml.Append(xml_attr("sp1", reader["SpendSP1"]));
            xml.Append(xml_attr("sp2", reader["SpendSP2"]));
            xml.Append(xml_attr("sp3", reader["SpendSP3"]));
            xml.Append(xml_attr("sv", reader["Skills"].ToString().TrimEnd(" 0".ToCharArray())));

            xml.Append("/>");
        }
        xml.Append("</loadouts>\n");
    }

    void AddInventory(ref StringBuilder xml)
    {
        // move to next select, where data should be
        reader.NextResult();

        xml.Append("<inventory>\n");
        while (reader.Read())
        {
            int ItemID = Convert.ToInt32(reader["ItemID"]);
            int Quantity = Convert.ToInt32(reader["Quantity"]);

            xml.Append("<i ");
            xml.Append(xml_attr("id", ItemID));
            xml.Append(xml_attr("ml", reader["MinutesLeft"]));
            if (Quantity > 1) xml.Append(xml_attr("qt", Quantity));

            xml.Append("/>");
        }
        xml.Append("</inventory>\n");
    }

    void AddAchievements(ref StringBuilder xml)
    {
        // move to next select, where data should be
        reader.NextResult();

        xml.Append("<achievements>\n");
        while (reader.Read())
        {
            int AchID = Convert.ToInt32(reader["AchID"]);
            int AchValue = Convert.ToInt32(reader["Value"]);
            int AchUnlocked = Convert.ToInt32(reader["Unlocked"]);

            xml.Append("<a ");
            xml.Append(xml_attr("id", AchID));
            xml.Append(xml_attr("v", AchValue));
            xml.Append(xml_attr("u", AchUnlocked));
            xml.Append("/>");
        }
        xml.Append("</achievements>\n");
    }

    void AddNewItemsInStore(ref StringBuilder xml)
    {
        // move to next select, with new items in store
        reader.NextResult();

        xml.Append("<nis>\n");
        while (reader.Read())
        {
            xml.Append("<i ");
            xml.Append(xml_attr("id", reader["ItemID"]));
            xml.Append("/>");
        }
        xml.Append("</nis>\n");
    }

    void AddStatistics(ref StringBuilder xml, string xmlNodeName)
    {
        // move to next select, with statistics
        reader.NextResult();
        reader.Read();

        xml.Append("<" + xmlNodeName + " ");
        xml.Append(xml_attr("dg", reader["DailyGames"]));
        xml.Append(xml_attr("ki", reader["Kills"]));
        xml.Append(xml_attr("hs", reader["Headshots"]));
        xml.Append(xml_attr("cf", reader["CaptureFlags"]));
        xml.Append(xml_attr("mcq", reader["MatchesCQ"]));
        xml.Append(xml_attr("mdm", reader["MatchesDM"]));
        xml.Append(xml_attr("msb", reader["MatchesSB"]));
        xml.Append("/>\n");
    }

    string GetGNABalance(string CustomerID)
    {
        SqlCommand sqcmd = new SqlCommand();
        sqcmd.CommandType = CommandType.StoredProcedure;
        sqcmd.CommandText = "WO_GNAGetBalance";
        sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);
        if (!CallWOApi(sqcmd))
            throw new ApiExitException("Failed to get GNA Balance");

        reader.Read();
        string GNABalance = reader["GNABalance"].ToString();
        reader.Close();

        return GNABalance;
    }

    protected override void Execute()
    {
        if (!WoCheckLoginSession())
            return;

        string CustomerID = web.CustomerID();

        // if we have "jg" parameter, it means GetProfile call was called from server
        string IsJoiningGame = "0";
        try
        {
            IsJoiningGame = web.Param("jg");
        }
        catch { }


        // in case of russian server, override balance from balance from GAMENET
        string gamePoints = "";
        if (IsJoiningGame == "0" && ConfigurationManager.AppSettings.Get("WO_Region") == "RU")
        {
            // override gamepoints with information from gamenet balance
            gamePoints = GetGNABalance(CustomerID);
        }

        SqlCommand sqcmd = new SqlCommand();
        sqcmd.CommandType = CommandType.StoredProcedure;
        sqcmd.CommandText = "WO_GetAccountInfo4";
        sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);
        sqcmd.Parameters.AddWithValue("@in_IsJoiningGame", IsJoiningGame);

        if (!CallWOApi(sqcmd))
            return;

        reader.Read();

        if (ConfigurationManager.AppSettings.Get("WO_Region") != "RU")
            gamePoints = reader["GamePoints"].ToString();

        string curTime = string.Format("{0} {1} {2} {3} {4}",
            DateTime.Now.Year, DateTime.Now.Month, DateTime.Now.Day,
            DateTime.Now.Hour, DateTime.Now.Minute);

        StringBuilder xml = new StringBuilder();
        xml.Append("<?xml version=\"1.0\"?>\n");
        xml.Append("<account ");
        xml.Append(xml_attr("CustomerID", reader["CustomerID"]));
        xml.Append(xml_attr("AccountStatus", reader["AccountStatus"]));
        xml.Append(xml_attr("gamertag", reader["Gamertag"].ToString().TrimEnd()));
        xml.Append(xml_attr("gamepoints", gamePoints));
        xml.Append(xml_attr("GameDollars", reader["GameDollars"]));
        xml.Append(xml_attr("HonorPoints", reader["HonorPoints"]));
        xml.Append(xml_attr("SkillPoints", reader["SkillPoints"]));
        xml.Append(xml_attr("ClanID", reader["ClanID"]));
        xml.Append(xml_attr("ClanRank", reader["ClanRank"]));
        xml.Append(xml_attr("ClanTag", reader["ClanTag"]));
        xml.Append(xml_attr("ClanTagColor", reader["ClanTagColor"]));
        xml.Append(xml_attr("Kills", reader["Kills"]));
        xml.Append(xml_attr("Deaths", reader["Deaths"]));
        xml.Append(xml_attr("ShotsFired", reader["ShotsFired"]));
        xml.Append(xml_attr("ShotsHits", reader["ShotsHits"]));
        xml.Append(xml_attr("Headshots", reader["Headshots"]));
        xml.Append(xml_attr("AssistKills", reader["AssistKills"]));
        xml.Append(xml_attr("Wins", reader["Wins"]));
        xml.Append(xml_attr("Losses", reader["Losses"]));
        xml.Append(xml_attr("CaptureNeutralPoints", reader["CaptureNeutralPoints"]));
        xml.Append(xml_attr("CaptureEnemyPoints", reader["CaptureEnemyPoints"]));
        xml.Append(xml_attr("TimePlayed", reader["TimePlayed"]));
        xml.Append(xml_attr("Abilities", reader["Abilities"]));
        xml.Append(xml_attr("F1S", reader["Faction1Score"]));
        xml.Append(xml_attr("F2S", reader["Faction2Score"]));
        xml.Append(xml_attr("F3S", reader["Faction3Score"]));
        xml.Append(xml_attr("F4S", reader["Faction4Score"]));
        xml.Append(xml_attr("F5S", reader["Faction5Score"]));
        xml.Append(xml_attr("time", curTime));
        xml.Append(xml_attr("IsFPSEnabled", reader["IsFPSEnabled"]));
        string IsDeveloper = reader["IsDeveloper"].ToString();
        if(IsDeveloper != "0")
            xml.Append(xml_attr("IsDev", IsDeveloper));
        xml.Append(">\n");

        AddLoadouts(ref xml);
        AddAchievements(ref xml);
        AddFPSAttachInventory(ref xml);
        AddInventory(ref xml);
        AddNewItemsInStore(ref xml);
        AddStatistics(ref xml, "sday");
        AddStatistics(ref xml, "sweek");

        xml.Append("</account>");

        GResponse.Write(xml.ToString());
    }
}
