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

public partial class api_SetRSUpdateStatus : WOApiWebPage
{
    protected override void Execute()
    {
        string CustomerID = web.CustomerID();
        string Status = web.Param("Status");
        string Perc = web.Param("Perc");
        string r3dLog = web.Param("r3dLog");

        SqlCommand sqcmd = new SqlCommand();
        sqcmd.CommandType = CommandType.StoredProcedure;
        sqcmd.CommandText = "ECLIPSE_SetRSUpdateStatus";
        sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);
        sqcmd.Parameters.AddWithValue("@in_Status", Status);
        sqcmd.Parameters.AddWithValue("@in_Perc", Perc);
        sqcmd.Parameters.AddWithValue("@in_r3dLog", r3dLog);

        if (!CallWOApi(sqcmd))
            return;

        Response.Write("WO_0");
    }
}
