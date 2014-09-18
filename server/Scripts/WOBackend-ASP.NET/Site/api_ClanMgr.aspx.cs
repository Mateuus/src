using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Data;
using System.Text;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;
using System.Data.SqlClient;

public partial class api_ClanMgr : WOApiWebPage
{
    void ClanLeave()
    {
        string CustomerID = web.CustomerID();

        SqlCommand sqcmd = new SqlCommand();
        sqcmd.CommandType = CommandType.StoredProcedure;
        sqcmd.CommandText = "WO_ClanLeave";
        sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);

        if (!CallWOApi(sqcmd))
            return;

        Response.Write("WO_0");
    }

    void ClanKick()
    {
        string CustomerID = web.CustomerID();
        string MemberID = web.Param("MemberID");

        SqlCommand sqcmd = new SqlCommand();
        sqcmd.CommandType = CommandType.StoredProcedure;
        sqcmd.CommandText = "WO_ClanKickMember";
        sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);
        sqcmd.Parameters.AddWithValue("@in_MemberID", MemberID);

        if (!CallWOApi(sqcmd))
            return;

        Response.Write("WO_0");
    }

    void ClanSetRank()
    {
        string CustomerID = web.CustomerID();
        string MemberID = web.Param("MemberID");
        string Rank = web.Param("Rank");

        SqlCommand sqcmd = new SqlCommand();
        sqcmd.CommandType = CommandType.StoredProcedure;
        sqcmd.CommandText = "WO_ClanSetMemberRank";
        sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);
        sqcmd.Parameters.AddWithValue("@in_MemberID", MemberID);
        sqcmd.Parameters.AddWithValue("@in_Rank", Rank);

        if (!CallWOApi(sqcmd))
            return;

        Response.Write("WO_0");
    }

    void ClanSetLore()
    {
        string CustomerID = web.CustomerID();
        string Lore = web.Param("Lore");

        SqlCommand sqcmd = new SqlCommand();
        sqcmd.CommandType = CommandType.StoredProcedure;
        sqcmd.CommandText = "WO_ClanSetLore";
        sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);
        sqcmd.Parameters.AddWithValue("@in_Lore", Lore);

        if (!CallWOApi(sqcmd))
            return;

        Response.Write("WO_0");
    }

    void ClanDonateToClan()
    {
        string CustomerID = web.CustomerID();
        string GP = web.Param("GP");

        SqlCommand sqcmd = new SqlCommand();
        sqcmd.CommandType = CommandType.StoredProcedure;
        sqcmd.CommandText = "WO_ClanDonateToClanGP";
        sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);
        sqcmd.Parameters.AddWithValue("@in_GP", GP);

        if (!CallWOApi(sqcmd))
            return;

        Response.Write("WO_0");
    }

    void ClanDonateToMember()
    {
        string CustomerID = web.CustomerID();
        string GP = web.Param("GP");
        string MemberID = web.Param("MemberID");

        SqlCommand sqcmd = new SqlCommand();
        sqcmd.CommandType = CommandType.StoredProcedure;
        sqcmd.CommandText = "WO_ClanDonateToMemberGP";
        sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);
        sqcmd.Parameters.AddWithValue("@in_GP", GP);
        sqcmd.Parameters.AddWithValue("@in_MemberID", MemberID);

        if (!CallWOApi(sqcmd))
            return;

        Response.Write("WO_0");
    }

    protected override void Execute()
    {
        if (!WoCheckLoginSession())
            return;

        string func = web.Param("func");
        if (func == "leave")
            ClanLeave();
        else if (func == "kick")
            ClanKick();
        else if (func == "setrank")
            ClanSetRank();
        else if (func == "setlore")
            ClanSetLore();
        else if (func == "gpmember")
            ClanDonateToMember();
        else if (func == "gpclan")
            ClanDonateToClan();
        else
            throw new ApiExitException("bad func");
    }
}
