using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Data;
using System.Text;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;
using System.Data.SqlClient;

public partial class api_WeaponAttach : WOApiWebPage
{
    void BuyAttachment()
    {
        string CustomerID = web.CustomerID();
        string WeaponID = web.Param("WeaponID");
        string AttachID = web.Param("AttachID");
        string Slot = web.Param("Slot");
        string BuyIdx = web.Param("BuyIdx");

        // check if attachment is valid
        {
            SqlCommand sqcmd = new SqlCommand();
            sqcmd.CommandType = CommandType.StoredProcedure;
            sqcmd.CommandText = "WO_WeaponAttachCheckBuy";
            sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);
            sqcmd.Parameters.AddWithValue("@in_WeaponID", WeaponID);
            sqcmd.Parameters.AddWithValue("@in_AttachmentID", AttachID);
            sqcmd.Parameters.AddWithValue("@in_Slot", Slot);

            if (!CallWOApi(sqcmd))
                return;
        }

        int balance = 0;

        // buy attachment item for specified weapon
        {
            SqlCommand sqcmd = new SqlCommand();
            sqcmd.CommandType = CommandType.StoredProcedure;
            sqcmd.CommandText = BuyItem3.GetBuyProcFromIdx(BuyIdx);
            sqcmd.Parameters.AddWithValue("@in_IP", LastIP);
            sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);
            sqcmd.Parameters.AddWithValue("@in_ItemId", AttachID);
            sqcmd.Parameters.AddWithValue("@in_BuyDays", BuyItem3.GetBuyDaysFromIdx(BuyIdx));
            sqcmd.Parameters.AddWithValue("@in_Param1", WeaponID);

            if (!CallWOApi(sqcmd))
                return;
        }

        // return new balance
        reader.Read();
        balance = getInt("Balance");
        Response.Write("WO_0");
        Response.Write(string.Format("{0}", balance));
    }

    void EquipAttachment()
    {
        string CustomerID = web.CustomerID();
        string WeaponID = web.Param("WeaponID");
        string AttachID = web.Param("AttachID");
        string Slot = web.Param("Slot");

        SqlCommand sqcmd = new SqlCommand();
        sqcmd.CommandType = CommandType.StoredProcedure;
        sqcmd.CommandText = "WO_WeaponAttachSet";
        sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);
        sqcmd.Parameters.AddWithValue("@in_WeaponID", WeaponID);
        sqcmd.Parameters.AddWithValue("@in_AttachmentID", AttachID);
        sqcmd.Parameters.AddWithValue("@in_Slot", Slot);

        if (!CallWOApi(sqcmd))
            return;

        Response.Write("WO_0");
    }

    void FixDefaultAttachments()
    {
        string CustomerID = web.CustomerID();
        string WeaponID = web.Param("WeaponID");

        SqlCommand sqcmd = new SqlCommand();
        sqcmd.CommandType = CommandType.StoredProcedure;
        sqcmd.CommandText = "WO_WeaponAttachFixDefaults";
        sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);
        sqcmd.Parameters.AddWithValue("@in_WeaponID", WeaponID);

        if (!CallWOApi(sqcmd))
            return;

        Response.Write("WO_0");
    }

    protected override void Execute()
    {
        if (!WoCheckLoginSession())
            return;

        string func = web.Param("func");
        if (func == "buy")
            BuyAttachment();
        else if (func == "equip")
            EquipAttachment();
        else if (func == "fix")
            FixDefaultAttachments();
        else
            throw new ApiExitException("bad func");
    }
}
