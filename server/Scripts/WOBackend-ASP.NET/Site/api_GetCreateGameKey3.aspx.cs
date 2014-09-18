using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Data;
using System.Text;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;
using System.Data.SqlClient;

public partial class api_GetCreateGameKey3 : WOApiWebPage
{
    protected override void Execute()
    {
        if (!WoCheckLoginSession())
            return;

        string CustomerID = web.CustomerID();
        string ServerID = web.Param("ServerID");
        string isBasicGame = web.Param("BasicGame");

        SqlCommand sqcmd = new SqlCommand();
        sqcmd.CommandType = CommandType.StoredProcedure;
        sqcmd.CommandText = "WO_GetCreateGameKey3";
        sqcmd.Parameters.AddWithValue("@in_IP", LastIP);
        sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);
        sqcmd.Parameters.AddWithValue("@in_ServerID", ServerID);
        sqcmd.Parameters.AddWithValue("@in_BasicGame", isBasicGame);

        if (!CallWOApi(sqcmd))
            return;

        reader.Read();
        int v1 = getInt("CreateGameKey");

        Response.Write("WO_0");
        Response.Write(string.Format("{0}",
            v1));
    }
}
