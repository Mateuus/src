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

public partial class api_verificarban : WOApiWebPage
{
    protected override void Execute()
    {
        SqlCommand sqcmd = new SqlCommand();
        sqcmd.CommandType = CommandType.StoredProcedure;
        sqcmd.CommandText = "ECLIPSE_VerificaHWInfo";

        if (!CallWOApi(sqcmd))
            return;

        reader.Read();
        int ComputerID = getInt("ComputerID"); ;


        Response.Write("WO_0");
        Response.Write(string.Format("{0}",
        ComputerID));
    }
}
