using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Data;
using System.Text;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;
using System.Data.SqlClient;

public partial class api_ReportHWInfo : WOApiWebPage
{
    protected override void Execute()
    {
        string r00 = web.Param("r00");
        string r10 = web.Param("r10");
        string r11 = web.Param("r11");
        string r12 = web.Param("r12");
        string r13 = web.Param("r13");
        string r20 = web.Param("r20");
        string r21 = web.Param("r21");
        string r22 = web.Param("r22");
        string r23 = web.Param("r23");
        string r24 = web.Param("r24");
        string r25 = web.Param("r25");
        string r30 = web.Param("r30");

        SqlCommand sqcmd = new SqlCommand();
        sqcmd.CommandType = CommandType.StoredProcedure;
        sqcmd.CommandText = "ECLIPSE_ReportHWInfo";
        sqcmd.Parameters.AddWithValue("@in_IP", LastIP);
        sqcmd.Parameters.AddWithValue("@r00", r00);
        sqcmd.Parameters.AddWithValue("@r10", r10);
        sqcmd.Parameters.AddWithValue("@r11", r11);
        sqcmd.Parameters.AddWithValue("@r12", r12);
        sqcmd.Parameters.AddWithValue("@r13", r13);
        sqcmd.Parameters.AddWithValue("@r20", r20);
        sqcmd.Parameters.AddWithValue("@r21", r21);
        sqcmd.Parameters.AddWithValue("@r22", r22);
        sqcmd.Parameters.AddWithValue("@r23", r23);
        sqcmd.Parameters.AddWithValue("@r24", r24);
        sqcmd.Parameters.AddWithValue("@r25", r25);
        sqcmd.Parameters.AddWithValue("@r30", r30);

        if (!CallWOApi(sqcmd))
            return;

        Response.Write("WO_0");
    }
}
