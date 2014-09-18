using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Data;
using System.Text;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;
using System.Data.SqlClient;

public partial class api_ClientUpdateAchievements : WOApiWebPage
{
    protected override void Execute()
    {
        string CustomerID = web.CustomerID();
        string SteamID = "";
        int numAch = Convert.ToInt32(web.Param("NumAch"));
        for (int i = 0; i < numAch; ++i)
        {
            string AchID = web.Param(String.Format("AchID{0}", i));
            string AchValue = web.Param(String.Format("AchVal{0}", i));
            string AchUnlocked = web.Param(String.Format("AchUnl{0}", i));
            int AchievUnlocked = Convert.ToInt32(AchUnlocked);

            int AchIDNum = Convert.ToInt32(AchID);
            if(!(AchIDNum>=400 && AchIDNum<=512)) // only those IDs are allowed to be updated from client!
                 throw new ApiExitException("bad coffee");

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
                api.MarkSteamAchievementAchieved(SteamID, AchID );
            }

        }
        Response.Write("WO_0");
    }
}
