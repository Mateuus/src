using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Data;
using System.Text;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;
using System.Data.SqlClient;

public partial class api_SkillLearn : WOApiWebPage
{
    // from C++ UserSkills.h
    int NUM_TIERS = 3;
    int NUM_RANKS = 5;
    int NUM_SKILLS_PER_TIER = 10;
    int SKILL_CLASS_MULT = 100;	// multiplier used in SkillID to detect skill class

    int TIER2_UNLOCK = 20;
    int TIER3_UNLOCK = 20;

    protected override void Execute()
    {
        if (!WoCheckLoginSession())
            return;

        string CustomerID = web.CustomerID();
        string LoadoutID = web.Param("LoadoutID");
        int SkillID = Convert.ToInt32(web.Param("SkillID"));
        int SkillLevel = Convert.ToInt32(web.Param("SkillLevel"));

        // skill class and skill index inside loadout
        int SkillClass = SkillID / SKILL_CLASS_MULT;
        int SkillOff = SkillID % SKILL_CLASS_MULT;
        int SkillTier = (SkillID % SKILL_CLASS_MULT) / NUM_SKILLS_PER_TIER;
        if (SkillOff >= NUM_SKILLS_PER_TIER * NUM_TIERS)
            throw new ApiExitException("bad skillId");

        int[] SkillData = new int[NUM_SKILLS_PER_TIER * NUM_TIERS];
        int[] SkillPrices = new int[NUM_RANKS];
        int PlayerSP = 0;
        int SpendSP1 = 0;
        int SpendSP2 = 0;
        int SpendSP3 = 0;

        // 1st step, get info about player/loadout/skill
        {
            SqlCommand sqcmd = new SqlCommand();
            sqcmd.CommandType = CommandType.StoredProcedure;
            sqcmd.CommandText = "WO_SkillLearnPrepare";
            sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);
            sqcmd.Parameters.AddWithValue("@in_LoadoutID", LoadoutID);
            sqcmd.Parameters.AddWithValue("@in_SkillID", SkillID);

            if (!CallWOApi(sqcmd))
                return;

            reader.Read();
            PlayerSP = getInt("PlayerSP");
            SpendSP1 = getInt("SpendSP1");
            SpendSP2 = getInt("SpendSP2");
            SpendSP3 = getInt("SpendSP3");

            // validate player class vs skill
            int LoadoutClass = getInt("Class");
            if (LoadoutClass != SkillClass)
                throw new ApiExitException("wrong skill class");

            // parse skill levels
            string SkillString = reader["Skills"].ToString();
            int StringLen = Math.Min(SkillString.Length, NUM_SKILLS_PER_TIER * NUM_TIERS);
            for (int i = 0; i < StringLen; i++)
            {
                if (SkillString[i] >= '0' && SkillString[i] <= '9')
                    SkillData[i] = SkillString[i] - '0';
                else
                    SkillData[i] = 0;
            }

            // check level and correct advancement
            if (SkillLevel < 1 || SkillLevel > NUM_RANKS)
                throw new ApiExitException("bad skill level");
            if (SkillData[SkillOff] + 1 != SkillLevel)
                throw new ApiExitException("bad skill advance");

            // read skill prices
            reader.NextResult();
            reader.Read();
            if(!reader.HasRows)
                throw new ApiExitException("bad skillid data");

            // skill price can be negative to hide it from public build
            SkillPrices[0] = Math.Abs(getInt("Lv1"));
            SkillPrices[1] = Math.Abs(getInt("Lv2"));
            SkillPrices[2] = Math.Abs(getInt("Lv3"));
            SkillPrices[3] = Math.Abs(getInt("Lv4"));
            SkillPrices[4] = Math.Abs(getInt("Lv5"));

            if (SkillPrices[SkillLevel - 1] == 0)
                throw new ApiExitException("skill level price is not set");
        }

        // tier check
        switch (SkillTier)
        {
            default:
                throw new ApiExitException("bad skill tier");
            case 0:
                SpendSP1 += SkillPrices[SkillLevel - 1];
                break;
            case 1:
                if (SpendSP1 < TIER2_UNLOCK)
                    throw new ApiExitException("not enough sp for tier2");
                SpendSP2 += SkillPrices[SkillLevel - 1];
                break;
            case 2:
                if (SpendSP2 < TIER3_UNLOCK)
                    throw new ApiExitException("not enough sp for tier3");
                SpendSP3 += SkillPrices[SkillLevel - 1];
                break;
        }

        if ((SpendSP1 + SpendSP2 + SpendSP3) > PlayerSP)
            throw new ApiExitException("not enough SP");

        // ok, advance skill one level
        SkillData[SkillOff]++;

        // update loadout
        {
            // reassemble skill string
            string SkillString = "";
            for(int i=0; i<NUM_SKILLS_PER_TIER * NUM_TIERS; i++)
                SkillString += Convert.ToChar(SkillData[i] + '0');
            SkillString = SkillString.TrimEnd(" 0".ToCharArray());

            SqlCommand sqcmd = new SqlCommand();
            sqcmd.CommandType = CommandType.StoredProcedure;
            sqcmd.CommandText = "WO_SkillLearnSetLoadout";
            sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);
            sqcmd.Parameters.AddWithValue("@in_LoadoutID", LoadoutID);
            sqcmd.Parameters.AddWithValue("@in_SpendSP1", SpendSP1);
            sqcmd.Parameters.AddWithValue("@in_SpendSP2", SpendSP2);
            sqcmd.Parameters.AddWithValue("@in_SpendSP3", SpendSP3);
            sqcmd.Parameters.AddWithValue("@in_Skills", SkillString);

            if (!CallWOApi(sqcmd))
                return;
        }

        Response.Write("WO_0");
        Response.Write(string.Format("{0} {1} {2}", SpendSP1, SpendSP2, SpendSP3));
    }
}
