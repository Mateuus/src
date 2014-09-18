using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Data;
using System.Text;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;
using System.Data.SqlClient;

public partial class api_CheckLoginSession : WOApiWebPage
{
    protected override void Execute()
    {
        string CustomerID = web.CustomerID();
        string SessionID = web.SessionKey();

        SqlCommand sqcmd = new SqlCommand();
        sqcmd.CommandType = CommandType.StoredProcedure;
        sqcmd.CommandText = "WO_UPDATELOGINSESSION";
        sqcmd.Parameters.AddWithValue("@in_IP", LastIP);
        sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);
        sqcmd.Parameters.AddWithValue("@in_SessionID", SessionID);

        if (!CallWOApi(sqcmd))
            return;

        Response.Write("WO_0");
    }
}
