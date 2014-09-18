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

public partial class api_GNAGetBalance : WOApiWebPage
{
    protected override void Execute()
    {
        if (ConfigurationManager.AppSettings.Get("WO_Region") != "RU")
            throw new ApiExitException("bad region");

        if (!WoCheckLoginSession())
            return;

        string CustomerID = web.CustomerID();

        SqlCommand sqcmd = new SqlCommand();
        sqcmd.CommandType = CommandType.StoredProcedure;
        sqcmd.CommandText = "WO_GNAGetBalance";
        sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);
        if (!CallWOApi(sqcmd))
            throw new ApiExitException("Failed to get GNA Balance");

        reader.Read();
        string GNABalance = reader["GNABalance"].ToString();

        Response.Write("WO_0");
        Response.Write(string.Format("{0}", GNABalance));
    }
}
