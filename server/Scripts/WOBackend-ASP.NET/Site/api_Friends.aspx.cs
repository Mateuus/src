using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Data;
using System.Text;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;
using System.Data.SqlClient;
using System.Configuration;

public partial class api_Friends : WOApiWebPage
{
    void FriendAddReq()
    {
        string CustomerID = web.CustomerID();
        string FriendGamerTag = web.Param("name");

        SqlCommand sqcmd = new SqlCommand();
        sqcmd.CommandType = CommandType.StoredProcedure;
        sqcmd.CommandText = "WO_FriendAddReq";
        sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);
        sqcmd.Parameters.AddWithValue("@in_FriendGamerTag", FriendGamerTag);
        if (!CallWOApi(sqcmd))
            return;

        reader.Read();
        int FriendStatus = getInt("FriendStatus");

        Response.Write("WO_0");
        Response.Write(string.Format("{0}", FriendStatus));
    }

    void FriendAddAns()
    {
        string CustomerID = web.CustomerID();
        string FriendID = web.Param("FriendID");
        string Allow = web.Param("Allow");

        SqlCommand sqcmd = new SqlCommand();
        sqcmd.CommandType = CommandType.StoredProcedure;
        sqcmd.CommandText = "WO_FriendAddAns";
        sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);
        sqcmd.Parameters.AddWithValue("@in_FriendID", FriendID);
        sqcmd.Parameters.AddWithValue("@in_Allow", Allow);
        if (!CallWOApi(sqcmd))
            return;

        Response.Write("WO_0");
    }

    void FriendRemove()
    {
        string CustomerID = web.CustomerID();
        string FriendID = web.Param("FriendID");

        SqlCommand sqcmd = new SqlCommand();
        sqcmd.CommandType = CommandType.StoredProcedure;
        sqcmd.CommandText = "WO_FriendRemove";
        sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);
        sqcmd.Parameters.AddWithValue("@in_FriendID", FriendID);
        if (!CallWOApi(sqcmd))
            return;

        Response.Write("WO_0");
    }

    void FriendGetInfo()
    {
        string CustomerID = web.CustomerID();
        string FriendID = web.Param("FriendID");

        SqlCommand sqcmd = new SqlCommand();
        sqcmd.CommandType = CommandType.StoredProcedure;
        sqcmd.CommandText = "WO_FriendGetStats";
        sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);
        sqcmd.Parameters.AddWithValue("@in_FriendID", FriendID);
        if (!CallWOApi(sqcmd))
            return;

        StringBuilder xml = new StringBuilder();
        xml.Append("<?xml version=\"1.0\"?>\n");
        xml.Append("<friends>");
        while (reader.Read())
        {
            xml.Append("<f ");
            xml.Append(xml_attr("ID", reader["FriendID"]));
            xml.Append(xml_attr("XP", reader["HonorPoints"]));
            xml.Append(xml_attr("k", reader["Kills"]));
            xml.Append(xml_attr("d", reader["Deaths"]));
            xml.Append(xml_attr("w", reader["Wins"]));
            xml.Append(xml_attr("l", reader["Losses"]));
            xml.Append(xml_attr("t", reader["TimePlayed"]));
            xml.Append("/>");
        }
        xml.Append("</friends>");

        GResponse.Write(xml.ToString());
    }

    protected override void Execute()
    {
        if (!WoCheckLoginSession())
            return;

        string func = web.Param("func");
        if (func == "addReq")
            FriendAddReq();
        else if (func == "addAns")
            FriendAddAns();
        else if (func == "remove")
            FriendRemove();
        else if (func == "stats")
            FriendGetInfo();
        else
            throw new ApiExitException("bad func");
    }
}
