using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Data;
using System.Text;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;
using System.Data.SqlClient;

public partial class api_SrvAddUserRoundResult4 : WOApiWebPage
{
    void UpdateLoadoutStats()
    {
        string CustomerID = web.CustomerID();

        for (int i = 0; i < 999; i++)
        {
            string LoadoutID = null;
            string TimePlayed = "";
            string HonorPoints = "";
            try
            {
                LoadoutID = web.Param("lid" + i.ToString());
                TimePlayed = web.Param("ltp" + i.ToString());
                HonorPoints = web.Param("lxp" + i.ToString());
            }
            catch
            {
                break;
            }

            SqlCommand sqcmd = new SqlCommand();
            sqcmd.CommandType = CommandType.StoredProcedure;
            sqcmd.CommandText = "WO_LoadoutUpdateStats";
            sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);
            sqcmd.Parameters.AddWithValue("@in_LoadoutID", LoadoutID);
            sqcmd.Parameters.AddWithValue("@in_TimePlayed", TimePlayed);
            sqcmd.Parameters.AddWithValue("@in_HonorPoints", HonorPoints);
            if (!CallWOApi(sqcmd))
                return;
        }
    }

    protected override void Execute()
    {
        string skey1 = web.Param("skey1");
        if (skey1 != SERVER_API_KEY)
            throw new ApiExitException("bad key");

        string CustomerID = web.CustomerID();
        string GameSessionID = web.Param("GameSessionID");

        string MapID = web.Param("MapID");
        string MapType = web.Param("MapType");
        string TeamID = web.Param("TeamID");
        string GamePoints = web.Param("GamePoints");
        string GameDollars = web.Param("GameDollars");
        string HonorPoints = web.Param("HonorPoints");
        string SkillPoints = web.Param("SkillPoints");
        string Kills = web.Param("Kills");
        string Deaths = web.Param("Deaths");
        string ShotsFired = web.Param("ShotsFired");
        string ShotsHits = web.Param("ShotsHits");
        string Headshots = web.Param("Headshots");
        string AssistKills = web.Param("AssistKills");
        string Wins = web.Param("Wins");
        string Losses = web.Param("Losses");
        string CaptureNeutralPoints = web.Param("CaptureNeutralPoints");
        string CaptureEnemyPoints = web.Param("CaptureEnemyPoints");
        string TimePlayed = web.Param("TimePlayed");
        string LevelUpMin = web.Param("LevelUpMin");
        string LevelUpMax = web.Param("LevelUpMax");

        SqlCommand sqcmd = new SqlCommand();
        sqcmd.CommandType = CommandType.StoredProcedure;
        sqcmd.CommandText = "WO_SRV_AddUserRoundResult4";
        sqcmd.Parameters.AddWithValue("@in_IP", LastIP);
        sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);
        sqcmd.Parameters.AddWithValue("@in_GameSessionID", GameSessionID);

        sqcmd.Parameters.AddWithValue("@in_MapID", MapID);
        sqcmd.Parameters.AddWithValue("@in_MapType", MapType);
        sqcmd.Parameters.AddWithValue("@in_TeamID", TeamID);
        sqcmd.Parameters.AddWithValue("@in_GamePoints", GamePoints);
        sqcmd.Parameters.AddWithValue("@in_GameDollars", GameDollars);
        sqcmd.Parameters.AddWithValue("@in_HonorPoints", HonorPoints);
        sqcmd.Parameters.AddWithValue("@in_SkillPoints", SkillPoints);
        sqcmd.Parameters.AddWithValue("@in_Kills", Kills);
        sqcmd.Parameters.AddWithValue("@in_Deaths", Deaths);
        sqcmd.Parameters.AddWithValue("@in_ShotsFired", ShotsFired);
        sqcmd.Parameters.AddWithValue("@in_ShotsHits", ShotsHits);
        sqcmd.Parameters.AddWithValue("@in_Headshots", Headshots);
        sqcmd.Parameters.AddWithValue("@in_AssistKills", AssistKills);
        sqcmd.Parameters.AddWithValue("@in_Wins", Wins);
        sqcmd.Parameters.AddWithValue("@in_Losses", Losses);
        sqcmd.Parameters.AddWithValue("@in_CaptureNeutralPoints", CaptureNeutralPoints);
        sqcmd.Parameters.AddWithValue("@in_CaptureEnemyPoints", CaptureEnemyPoints);
        sqcmd.Parameters.AddWithValue("@in_TimePlayed", TimePlayed);
        sqcmd.Parameters.AddWithValue("@in_LevelUpMin", LevelUpMin);
        sqcmd.Parameters.AddWithValue("@in_LevelUpMax", LevelUpMax);

        // additional protection of sql proc
        sqcmd.Parameters.AddWithValue("@in_skey2", "ACsR4x23GsjYU*476xnDvYXK@!56");

        if (!CallWOApi(sqcmd))
            return;

        UpdateLoadoutStats();

        Response.Write("WO_0");
    }
}
