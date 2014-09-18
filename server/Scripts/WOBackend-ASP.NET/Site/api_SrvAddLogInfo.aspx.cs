using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Data;
using System.Text;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;
using System.Data.SqlClient;

public partial class api_SrvAddLogInfo : WOApiWebPage
{
    protected override void Execute()
    {
        string skey1 = web.Param("skey1");
        if (skey1 != SERVER_API_KEY)
            throw new ApiExitException("bad key");

        string CustomerID = web.CustomerID();
        string GameSessionID = web.Param("GameSessionID");
        string CheatID = web.Param("CheatID");
        string Msg = web.Param("Msg");
        string Data = web.Param("Data");
        string IPStr = web.Param("IP");
        System.Net.IPAddress IpAddr = new System.Net.IPAddress(Convert.ToInt64(IPStr));

        SqlCommand sqcmd = new SqlCommand();
        sqcmd.CommandType = CommandType.StoredProcedure;
        sqcmd.CommandText = "WO_SRV_AddLogInfo";
        sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);
        sqcmd.Parameters.AddWithValue("@in_GameSessionID", GameSessionID);
        sqcmd.Parameters.AddWithValue("@in_CustomerIP", IpAddr.ToString());
        sqcmd.Parameters.AddWithValue("@in_CheatID", CheatID);
        sqcmd.Parameters.AddWithValue("@in_Msg", Msg);
        sqcmd.Parameters.AddWithValue("@in_Data", Data);

        if (!CallWOApi(sqcmd))
            return;

        Response.Write("WO_0");
    }
}
