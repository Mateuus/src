using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Data;
using System.Text;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;
using System.Data.SqlClient;
using System.IO;

//
// return UI parameters for all shop items, used for client to display current weapon stats
//
public partial class api_GetItemsInfo : WOApiWebPage
{
    bool HaveShopPrice()
    {
        int p1 = getInt("Price1");
        int p2 = getInt("Price7");
        int p3 = getInt("Price30");
        int p4 = getInt("PriceP");
        int p5 = getInt("GPrice1");
        int p6 = getInt("GPrice7");
        int p7 = getInt("GPrice30");
        int p8 = getInt("GPriceP");
        if (p1 == 0 && p2 == 0 && p3 == 0 && p4 == 0 && p5 == 0 && p6 == 0 && p7 == 0 && p8 == 0)
            return false;

        return true;
    }


    void OutGearData(StringBuilder xml)
    {
        if (!HaveShopPrice())
            return;

        xml.Append("<g ");
        xml.Append(xml_attr("ID", reader["ItemID"]));
        xml.Append(xml_attr("lv", reader["LevelRequired"]));
        xml.Append(xml_attr("wg", reader["Weight"]));
        xml.Append(xml_attr("dp", reader["DamagePerc"]));
        xml.Append(xml_attr("dm", reader["DamageMax"]));
        xml.Append("/>\n");
    }

    void OutWeaponData(StringBuilder xml)
    {
        if (!HaveShopPrice())
            return;

        xml.Append("<w ");
        xml.Append(xml_attr("ID", reader["ItemID"]));
        xml.Append(xml_attr("lv", reader["LevelRequired"]));
        xml.Append(xml_attr("d1", reader["Damage"]));
        xml.Append(xml_attr("d2", reader["DamageDecay"]));
        xml.Append(xml_attr("c1", reader["NumClips"]));
        xml.Append(xml_attr("c2", reader["ClipSize"]));
        xml.Append(xml_attr("rf", reader["RateOfFire"]));
        xml.Append(xml_attr("sp", reader["Spread"]));
        xml.Append(xml_attr("rc", reader["Recoil"]));
        xml.Append("/>\n");
    }

    void OutPackageData(StringBuilder xml)
    {
        if (!HaveShopPrice())
            return;

        xml.Append("<p ");
        xml.Append(xml_attr("ID", reader["ItemID"]));
        xml.Append(xml_attr("name", reader["Name"]));
        xml.Append(xml_attr("fname", reader["FNAME"]));
        xml.Append(xml_attr("desc", reader["Description"]));
        xml.Append(xml_attr("gd", reader["AddGP"]));
        xml.Append(xml_attr("sp", reader["AddSP"]));
        xml.Append(xml_attr("i1i", reader["Item1_ID"]));
        xml.Append(xml_attr("i1e", reader["Item1_Exp"]));
        xml.Append(xml_attr("i2i", reader["Item2_ID"]));
        xml.Append(xml_attr("i2e", reader["Item2_Exp"]));
        xml.Append(xml_attr("i3i", reader["Item3_ID"]));
        xml.Append(xml_attr("i3e", reader["Item3_Exp"]));
        xml.Append(xml_attr("i4i", reader["Item4_ID"]));
        xml.Append(xml_attr("i4e", reader["Item4_Exp"]));
        xml.Append(xml_attr("i5i", reader["Item5_ID"]));
        xml.Append(xml_attr("i5e", reader["Item5_Exp"]));
        xml.Append(xml_attr("i6i", reader["Item6_ID"]));
        xml.Append(xml_attr("i6e", reader["Item6_Exp"]));
        xml.Append("/>\n");
    }

    void OutGenericsData(StringBuilder xml)
    {
        if (!HaveShopPrice())
            return;

        // output name/fname/desc for loot boxes and mystery crates
        string Category = reader["Category"].ToString();
        if (Category != "7" && Category != "3") // storecat_LootBox & storecat_MysteryBox
            return;

        xml.Append("<b ");
        xml.Append(xml_attr("ID", reader["ItemID"]));
        xml.Append(xml_attr("name", reader["Name"]));
        xml.Append(xml_attr("fname", reader["FNAME"]));
        xml.Append(xml_attr("desc", reader["Description"]));
        xml.Append("/>\n");
    }

    protected override void Execute()
    {
        SqlCommand sqcmd = new SqlCommand();
        sqcmd.CommandType = CommandType.StoredProcedure;
        sqcmd.CommandText = "WO_GetItemsData";

        if (!CallWOApi(sqcmd))
            return;

        StringBuilder xml = new StringBuilder();
        xml.Append("<?xml version=\"1.0\"?>\n");
        xml.Append("<items>");

        // gears
        xml.Append("<gears>");
        while (reader.Read())
        {
            OutGearData(xml);
        }
        xml.Append("</gears>");

        // weapons
        reader.NextResult();
        xml.Append("<weapons>");
        while (reader.Read())
        {
            OutWeaponData(xml);
        }
        xml.Append("</weapons>");

        // generics. right now - mystery crates/loot boxes
        reader.NextResult();
        xml.Append("<generics>");
        while (reader.Read())
        {
            OutGenericsData(xml);
        }
        xml.Append("</generics>");

        // packages
        reader.NextResult();
        xml.Append("<packages>");
        while (reader.Read())
        {
            OutPackageData(xml);
        }
        xml.Append("</packages>");

        xml.Append("</items>");
        GResponse.Write(xml.ToString());
    }               
}
