using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Data;
using System.Text;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;
using System.Data.SqlClient;

public partial class api_MysteryBox : WOApiWebPage
{
    public class LootEntry
    {
        public double Chance;
        public int ItemID;
        public int ExpDaysMin;
        public int ExpDaysMax;
        public int GDMin;
        public int GDMax;
        public int GDIfHave;
    }
    int category_ = 0;
    List<LootEntry> loots_ = new List<LootEntry>();
    Random rnd_ = new Random();

    public class WinData
    {
        public double Roll = 0;
        public int ItemID = 0;
        public int ExpDays = 0;
        public int GD = 0;
    }

    void ReadLootInfo(string LootID)
    {
        SqlCommand sqcmd = new SqlCommand();
        sqcmd.CommandType = CommandType.StoredProcedure;
        sqcmd.CommandText = "WO_LootGetData";
        sqcmd.Parameters.AddWithValue("@in_LootID", LootID);
        if (!CallWOApi(sqcmd))
            return;

        // get item category
        reader.Read();
        category_ = getInt("Category");

        // get content
        reader.NextResult();
        while (reader.Read())
        {
            LootEntry le = new LootEntry();
            le.Chance = Convert.ToDouble(reader["Chance"]);
            le.ItemID = Convert.ToInt32(reader["ItemID"]);
            le.ExpDaysMin = Convert.ToInt32(reader["ExpDaysMin"]);
            le.ExpDaysMax = Convert.ToInt32(reader["ExpDaysMax"]);
            le.GDMin = Convert.ToInt32(reader["GDMin"]);
            le.GDMax = Convert.ToInt32(reader["GDMax"]);
            le.GDIfHave = Convert.ToInt32(reader["GDIfHave"]);

            loots_.Add(le);
        }

        if (loots_.Count == 0)
            throw new ApiExitException("loot box is empty");

        // renormalize
        double Sum = 0.0;
        foreach (LootEntry le in loots_)
            Sum += le.Chance;
        foreach (LootEntry le in loots_)
            le.Chance /= Sum;

        // bring to [0..1]
        Sum = loots_[0].Chance;
        for(int i=1; i<loots_.Count; i++)
        {
            double Chance = loots_[i].Chance;
            loots_[i].Chance += Sum;
            Sum += Chance;
        }
    }

    WinData GetWinning()
    {
        WinData win = new WinData();
        win.Roll = rnd_.NextDouble();
        //Response.Write(win.Roll.ToString() + " : ");

        foreach (LootEntry le in loots_)
        {
            if (win.Roll <= le.Chance)
            {
                if (le.ItemID == 0)
                {
                    double min = (double)le.GDMin;
                    double max = (double)le.GDMax + 1;
                    win.GD = (int)(min + ((max - min) * rnd_.NextDouble()));
                    win.ItemID = 0;
                    //Response.Write(win.GD.ToString() + " GD" + "<br>");
                }
                else
                {
                    double min = (double)le.ExpDaysMin;
                    double max = (double)le.ExpDaysMax + 1;
                    win.ExpDays = (int)(min + ((max - min) * rnd_.NextDouble()));
                    win.ItemID = le.ItemID;
                    win.GD = le.GDIfHave;
                    //Response.Write(win.ItemID.ToString() + " for " + win.ExpDays.ToString() + "<br>");
                }
                return win;
            }
        }

        throw new ApiExitException("no winning");
    }

    class TestWonItem
    {
        public double Nums = 1;
    };

    void TestTheRolls()
    {
        string LootID = web.Param("LootID");
        ReadLootInfo(LootID);

        /*
        Response.Write("Loot Content for " + LootID + ":<br>");
        foreach (LootEntry le in loots_)
            Response.Write(string.Format("{0:000000} : {1:0.0000}<br>", le.ItemID, le.Chance));
         * */

        Dictionary<int, TestWonItem> wons = new Dictionary<int, TestWonItem>();
        Response.Write("Rolling:<br>");
        for(int i=0; i<50; i++)
        {
            WinData win = GetWinning();
            try
            {
                wons[win.ItemID].Nums += 1;
            }
            catch
            {
                wons.Add(win.ItemID, new TestWonItem());
            }

            if (win.ItemID == 0)
            {
                Response.Write(string.Format("Roll: {0:0.0000}, GD: {1}<br>",
                    win.Roll,
                    win.GD));
            }
            else
            {
                Response.Write(string.Format("Roll: {0:0.0000}, Item: {1} for {2} days<br>",
                    win.Roll,
                    win.ItemID,
                    win.ExpDays));
            }
        }

        Response.Write("Percentages:<br>");
        double AllTotals = 0;
        foreach (TestWonItem wi in wons.Values)
            AllTotals += wi.Nums;
        foreach (TestWonItem wi in wons.Values)
            wi.Nums /= AllTotals;

        foreach (int ItemID in wons.Keys)
            Response.Write(string.Format("{0} : {1}%<br>", ItemID, wons[ItemID].Nums * 100));

        Response.Write("<br>");
    }


    void GetBoxInfo()
    {
        string LootID = web.Param("LootID");
        ReadLootInfo(LootID);

        // can get content only for storecat_MysteryBox
        //if (category_ != 3)
        //    throw new ApiExitException("can't get content for loot");

        StringBuilder xml = new StringBuilder();
        xml.Append("<?xml version=\"1.0\"?>\n");
        xml.Append("<box>");
        foreach(LootEntry le in loots_)
        {
            xml.Append("<e ");
            xml.Append(xml_attr("ID", le.ItemID.ToString()));
            if (le.ItemID == 0)
            {
                xml.Append(xml_attr("v1", le.GDMin.ToString()));
                xml.Append(xml_attr("v2", le.GDMax.ToString()));
            }
            else
            {
                xml.Append(xml_attr("v1", le.ExpDaysMin.ToString()));
                xml.Append(xml_attr("v2", le.ExpDaysMax.ToString()));
            }

            xml.Append("/>\n");
        }
        xml.Append("</box>");

        GResponse.Write(xml.ToString());
    }

    void RollBox()
    {
        string CustomerID = web.CustomerID();
        string ItemID = web.Param("ItemID");
        string BuyIdx = web.Param("BuyIdx");

        // step 1: read loot box info
        ReadLootInfo(ItemID);

        // only storecat_MysteryBox can be bought for GD
        if(category_ != 3 && BuyItem3.IsGD(BuyIdx))
            throw new ApiExitException("can't buy loot for GD");

        // step 2: see what we won
        WinData win = GetWinning();

        // step 3: actually buy item
        int balance = 0;
        {
            SqlCommand sqcmd = new SqlCommand();
            sqcmd.CommandType = CommandType.StoredProcedure;
            sqcmd.CommandText = BuyItem3.GetBuyProcFromIdx(BuyIdx);
            sqcmd.Parameters.AddWithValue("@in_IP", LastIP);
            sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);
            sqcmd.Parameters.AddWithValue("@in_ItemId", ItemID);
            sqcmd.Parameters.AddWithValue("@in_BuyDays", BuyItem3.GetBuyDaysFromIdx(BuyIdx));

            if (!CallWOApi(sqcmd))
                return;

            reader.Read();
            balance = getInt("Balance");
        }

        // step 4: add winning!
        {
            SqlCommand sqcmd = new SqlCommand();
            sqcmd.CommandType = CommandType.StoredProcedure;
            sqcmd.CommandText = "WO_LootAddReward";
            sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);
            sqcmd.Parameters.AddWithValue("@in_Roll", win.Roll);
            sqcmd.Parameters.AddWithValue("@in_LootID", ItemID);
            sqcmd.Parameters.AddWithValue("@in_ItemID", win.ItemID.ToString());
            sqcmd.Parameters.AddWithValue("@in_ExpDays", win.ExpDays.ToString());
            sqcmd.Parameters.AddWithValue("@in_GD", win.GD.ToString());

            if (!CallWOApi(sqcmd))
                return;

            // overwrite GD result from procedure
            reader.Read();
            win.GD = getInt("GD");
        }

        Response.Write("WO_0");
        Response.Write(string.Format("{0} {1} {2} {3}", win.ItemID, win.ExpDays, win.GD, balance));

        return;
    }

    // will sell LOOT box
    void SellLootBox()
    {
        string CustomerID = web.CustomerID();
        string ItemID = web.Param("ItemID");

        SqlCommand sqcmd = new SqlCommand();
        sqcmd.CommandType = CommandType.StoredProcedure;
        sqcmd.CommandText = "WO_LootSellLootBox";
        sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);
        sqcmd.Parameters.AddWithValue("@in_ItemID", ItemID);

        if (!CallWOApi(sqcmd))
            return;

        Response.Write("WO_0");
    }

    protected override void Execute()
    {
        string func = web.Param("func");
        if (func == "fEEaTest001")
        {
            TestTheRolls();
            return;
        }

        if (!WoCheckLoginSession())
            return;

        if (func == "info")
            GetBoxInfo();
        else if (func == "roll")
            RollBox();
        else if (func == "sell")
            SellLootBox();
        else
            throw new ApiExitException("bad func");
    }
}
