using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Data;
using System.Text;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;
using System.Data.SqlClient;

public partial class api_SrvSetCreateGameKey2 : WOApiWebPage
{
    protected override void Execute()
    {
        string skey1 = web.Param("skey1");
        if (skey1 != SERVER_API_KEY)
            throw new ApiExitException("bad key");

        string ServerID = web.Param("ServerID");
        string CreateGameKey = web.Param("CreateGameKey");

        SqlCommand sqcmd = new SqlCommand();
        sqcmd.CommandType = CommandType.StoredProcedure;
        sqcmd.CommandText = "WO_SRV_SetCreateGameKey2";
        sqcmd.Parameters.AddWithValue("@in_IP", LastIP);
        sqcmd.Parameters.AddWithValue("@in_ServerID", ServerID);
        sqcmd.Parameters.AddWithValue("@in_CreateGameKey", CreateGameKey);

        if (!CallWOApi(sqcmd))
            return;

        Response.Write("WO_0");
    }
}
