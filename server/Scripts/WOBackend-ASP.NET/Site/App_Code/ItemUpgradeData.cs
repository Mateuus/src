using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Web;
using System.Data;
using System.Data.SqlClient;
using System.Diagnostics;
using System.Configuration;
using System.Net;
using System.IO;
using System.Text;

public class ItemUpgradeData
{
    public const int MAX_UPGRADE_SLOTS = 5;
    public const int MAX_UPGRADE_LEVEL = 99;

    public enum EUpgradeType
    {
        UP_Damage = 0,
        UP_Spread,
        UP_Recoil,
        UP_Firerate,
        UP_ClipSize,
        UP_MAX_UPGRADES,
    };

    public class UpgradeItemID
    {
        // ItemID in form of 6CCULL 
        public int ItemID;
        public int Category;
        public int Type;
        public int Level;

        public UpgradeItemID(int in_ItemID)
        {
            // ItemID in form of 6CCULL 
            ItemID = in_ItemID;
            Category = Convert.ToInt32(ItemID.ToString().Substring(1, 2));
            Type = Convert.ToInt32(ItemID.ToString().Substring(3, 1));
            Level = Convert.ToInt32(ItemID.ToString().Substring(4, 2));

            if (Type < 0 || Type >= (int)EUpgradeType.UP_MAX_UPGRADES)
                throw new ApiExitException("bad Type ItemID: " + ItemID);

            if (Level < 0 || Level >= MAX_UPGRADE_LEVEL)
                throw new ApiExitException("bad Level ItemID: " + ItemID);
        }

        public int ToItemID()
        {
            // ItemID in form of 6CCULL 
            return 600000 + Category * 1000 + Type * 100 + Level;
        }
    };

    public class UpgradeData
    {
        public UpgradeItemID uid = null;
        public double Value = 0;

        public int GPPrice = 0;
        public double GPChance = 0;
        public int GDPrice = 0;
        public double GDChance = 0;

        public UpgradeData(int in_ItemID)
        {
            uid = new UpgradeItemID(in_ItemID);
        }

        public bool IsValid()
        {
            return GPPrice > 0 || GDPrice > 0;
        }
    };

    public class TypeUpgrades
    {
        public UpgradeData[] levels = new UpgradeData[MAX_UPGRADE_LEVEL];
    };

    public class CategoryUpgrades
    {
        public TypeUpgrades[] types = new TypeUpgrades[(int)EUpgradeType.UP_MAX_UPGRADES];

        public CategoryUpgrades()
        {
            for (int i = 0; i < (int)EUpgradeType.UP_MAX_UPGRADES; i++)
            {
                types[i] = new TypeUpgrades();
            }
        }
    };

    public const int MAX_CATEGORIES = 99;
    CategoryUpgrades[] categories_ = new CategoryUpgrades[MAX_CATEGORIES];
    Dictionary<int, UpgradeData> items_ = new Dictionary<int, UpgradeData>();

    public ItemUpgradeData(WOApiWebPage in_form)
    {
        for (int i = 0; i < MAX_CATEGORIES; i++)
            categories_[i] = new CategoryUpgrades();

        // get all upgrade items 
        SqlCommand sqcmd = new SqlCommand();
        sqcmd.CommandType = CommandType.StoredProcedure;
        sqcmd.CommandText = "WO_ItemUpgradeGetUpgrades";
        if (!in_form.CallWOApi(sqcmd))
            throw new ApiExitException("UpgradeGetUpgrades failed");

        SqlDataReader reader = in_form.reader;
        while (reader.Read())
        {
            UpgradeData ud = new UpgradeData(Convert.ToInt32(reader["ItemID"]));
            ud.Value = Convert.ToDouble(reader["Value"]);
            ud.GPPrice = Convert.ToInt32(reader["PriceP"]);
            ud.GPChance = Convert.ToDouble(reader["GPChance"]) / 100.0;
            ud.GDPrice = Convert.ToInt32(reader["GPriceP"]);
            ud.GDChance = Convert.ToDouble(reader["GDChance"]) / 100.0;

            // decode and put it
            categories_[ud.uid.Category].types[ud.uid.Type].levels[ud.uid.Level] = ud;
            items_.Add(ud.uid.ItemID, ud);
        }
        reader = null;
    }

    public List<UpgradeData> GetZeroLevelUpgrades(int Category)
    {
        List<UpgradeData> upg = new List<UpgradeData>();

        TypeUpgrades[] types = categories_[Category].types;
        for (int i = 0; i < (int)EUpgradeType.UP_MAX_UPGRADES; i++)
        {
            if (types[i].levels[0] != null && types[i].levels[0].IsValid())
                upg.Add(types[i].levels[0]);
        }
        return upg;
    }

    public UpgradeData GetNextUpgrade(int UpgradeID)
    {
        // last 2 digits is UpgradeLevel
        if ((UpgradeID % 100) >= MAX_UPGRADE_LEVEL)
            return null;

        UpgradeData ud;
        if (items_.TryGetValue(UpgradeID + 1, out ud))
        {
            return ud;
        }

        return null;
    }

    public UpgradeData GetUpgrade(int ItemID)
    {
        UpgradeData ud;
        if (items_.TryGetValue(ItemID, out ud))
        {
            return ud;
        }

        return null;
    }

    public double GetValue(int UpgradeId)
    {
        UpgradeData ud;
        if (items_.TryGetValue(UpgradeId, out ud))
        {
            return ud.Value;
        }

        return 0.0f; // return value to indicate that upgrade is NOT exist
    }
}
