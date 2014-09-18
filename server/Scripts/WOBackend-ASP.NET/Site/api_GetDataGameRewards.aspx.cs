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

public partial class api_GetDataGameRewards : WOApiWebPage
{
    protected override void Execute()
    {
        SqlCommand sqcmd = new SqlCommand();
        sqcmd.CommandType = CommandType.StoredProcedure;
        sqcmd.CommandText = "WO_GetDataGameRewards";

        if (!CallWOApi(sqcmd))
            return;

        StringBuilder xml = new StringBuilder();
        xml.Append("<?xml version=\"1.0\"?>\n");
        xml.Append("<rewards>");

        while (reader.Read())
        {
            // strip name of RWD_
            string name = reader["RewardName"].ToString();
            name = name.Substring(4);

            xml.Append("<rwd ");
            xml.Append(xml_attr("ID", reader["RewardID"]));
            xml.Append(xml_attr("Name", name));
            xml.Append(xml_attr("GD_CQ", reader["GD_CQ"]));
            xml.Append(xml_attr("HP_CQ", reader["HP_CQ"]));
            xml.Append(xml_attr("GD_DM", reader["GD_DM"]));
            xml.Append(xml_attr("HP_DM", reader["HP_DM"]));
            xml.Append(xml_attr("GD_SB", reader["GD_SB"]));
            xml.Append(xml_attr("HP_SB", reader["HP_SB"]));
            xml.Append("/>\n");
        }

        xml.Append("</rewards>");
        GResponse.Write(xml.ToString());
    }               
}
