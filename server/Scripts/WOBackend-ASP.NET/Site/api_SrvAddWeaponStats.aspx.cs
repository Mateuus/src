using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Data;
using System.Text;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;
using System.Data.SqlClient;

public partial class api_SrvAddWeaponStats : WOApiWebPage
{
    protected override void Execute()
    {
        string skey1 = web.Param("skey1");
        if (skey1 != SERVER_API_KEY)
            throw new ApiExitException("bad key");

        string MapType = web.Param("MapType");

        for (int i = 0; i < 999; i++)
        {
            string wstr = null;
            try
            {
                 wstr = web.Param("w" + i.ToString());
            }
            catch
            {
                break;
            }

            string[] wstat = wstr.Split(" ".ToCharArray());
            string KillsCQ = (MapType == "0" ? wstat[3] : "0");   // MAPT_Conquest
            string KillsDM = (MapType == "1" ? wstat[3] : "0");   // MAPT_Deathmatch
            string KillsSB = (MapType == "3" ? wstat[3] : "0");   // MAPT_Bomb

            SqlCommand sqcmd = new SqlCommand();
            sqcmd.CommandType = CommandType.StoredProcedure;
            sqcmd.CommandText = "WO_SRV_AddWeaponStats";
            sqcmd.Parameters.AddWithValue("@in_ItemID", wstat[0]);
            sqcmd.Parameters.AddWithValue("@in_ShotsFired", wstat[1]);
            sqcmd.Parameters.AddWithValue("@in_ShotsHits", wstat[2]);
            sqcmd.Parameters.AddWithValue("@in_KillsCQ", KillsCQ);
            sqcmd.Parameters.AddWithValue("@in_KillsDM", KillsDM);
            sqcmd.Parameters.AddWithValue("@in_KillsSB", KillsSB);
            if (!CallWOApi(sqcmd))
                return;
        }

        Response.Write("WO_0");
    }
}
