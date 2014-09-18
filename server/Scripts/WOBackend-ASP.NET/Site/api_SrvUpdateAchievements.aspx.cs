using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Data;
using System.Text;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;
using System.Data.SqlClient;

public partial class api_SrvUpdateAchievements : WOApiWebPage
{
    protected override void Execute()
    {
        string skey1 = web.Param("skey1");
        if (skey1 != SERVER_API_KEY)
            throw new ApiExitException("bad key");

        string CustomerID = web.CustomerID();
        string SteamID = "";
        int numAch = Convert.ToInt32(web.Param("NumAch"));
        for (int i = 0; i < numAch; ++i)
        {
            string AchID = web.Param(String.Format("AchID{0}", i));
            string AchValue = web.Param(String.Format("AchVal{0}", i));
            string AchUnlocked = web.Param(String.Format("AchUnl{0}", i));
            int AchievUnlocked = Convert.ToInt32(AchUnlocked);

            SqlCommand sqcmd = new SqlCommand();
            sqcmd.CommandType = CommandType.StoredProcedure;
            sqcmd.CommandText = "WO_UpdateAchievementStatus";
            sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);
            sqcmd.Parameters.AddWithValue("@in_AchID", AchID);
            sqcmd.Parameters.AddWithValue("@in_AchValue", AchValue);
            sqcmd.Parameters.AddWithValue("@in_AchUnlocked", AchUnlocked);

            if (!CallWOApi(sqcmd))
                return;

            reader.Read();
            SteamID = getString("SteamID");

            // do steam unlock of this achievement
            if (SteamID != "0" && AchievUnlocked > 0)
            {
                SteamApi api = new SteamApi();
                api.MarkSteamAchievementAchieved(SteamID, AchID);

            }
        }

        Response.Write("WO_0");
    }
}
