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

public partial class api_GetShop5 : WOApiWebPage
{
    void OutShopItem(MemoryStream ms)
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
            return;

        Int32 priceBits = 0;
        if (p1 > 0) priceBits = priceBits | 1;
        if (p2 > 0) priceBits = priceBits | 2;
        if (p3 > 0) priceBits = priceBits | 4;
        if (p4 > 0) priceBits = priceBits | 8;
        if (p5 > 0) priceBits = priceBits | 16;
        if (p6 > 0) priceBits = priceBits | 32;
        if (p7 > 0) priceBits = priceBits | 64;
        if (p8 > 0) priceBits = priceBits | 128;

        Int32 ItemId = getInt("ItemId");
        Int32 itemCat = getInt("Category");

        msWriteInt(ms, ItemId);
        msWriteByte(ms, priceBits);
        msWriteByte(ms, itemCat);
        if (p1 > 0) msWriteInt(ms, p1);
        if (p2 > 0) msWriteInt(ms, p2);
        if (p3 > 0) msWriteInt(ms, p3);
        if (p4 > 0) msWriteInt(ms, p4);
        if (p5 > 0) msWriteInt(ms, p5);
        if (p6 > 0) msWriteInt(ms, p6);
        if (p7 > 0) msWriteInt(ms, p7);
        if (p8 > 0) msWriteInt(ms, p8);
    }

    void OutSkillPrices2(MemoryStream ms)
    {
        while (reader.Read())
        {
            int SkillID = getInt("SkillID");
            int lv1 = getInt("Lv1");
            int lv2 = getInt("Lv2");
            int lv3 = getInt("Lv3");
            int lv4 = getInt("Lv4");
            int lv5 = getInt("Lv5");

            msWriteShort(ms, SkillID);
            msWriteByte(ms, lv1);
            msWriteByte(ms, lv2);
            msWriteByte(ms, lv3);
            msWriteByte(ms, lv4);
            msWriteByte(ms, lv5);
        }
        msWriteShort(ms, 0xFFFF);
    }

    protected override void Execute()
    {
        SqlCommand sqcmd = new SqlCommand();
        sqcmd.CommandType = CommandType.StoredProcedure;
        sqcmd.CommandText = "WO_GetShopInfo5";

        if (!CallWOApi(sqcmd))
            return;

        // "SHO3"
        byte[] itemHdr = new byte[4] { 83, 72, 79, 51 };

        MemoryStream ms = new MemoryStream();
        ms.Write(itemHdr, 0, 4);

        OutSkillPrices2(ms);
        ms.Write(itemHdr, 0, 4);

        // common items
        reader.NextResult();
        while(reader.Read())
        {
            OutShopItem(ms);
        }

        // packages - must be SAME as item for now
        reader.NextResult();
        while (reader.Read())
        {
            // package can be disabled
            if (getInt("IsEnabled") == 0)
                continue;
            OutShopItem(ms);
        }

        ms.Write(itemHdr, 0, 4);

        GResponse.Write("WO_0");
        GResponse.BinaryWrite(ms.ToArray());
    }               
}
