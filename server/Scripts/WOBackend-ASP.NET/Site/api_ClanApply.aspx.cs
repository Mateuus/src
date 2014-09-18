using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Data;
using System.Text;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;
using System.Data.SqlClient;

public partial class api_ClanApply : WOApiWebPage
{
    void ApplyToJoin()
    {
        string CustomerID = web.CustomerID();
        string ClanID = web.Param("ClanID");
        string Note = web.Param("Note");

        SqlCommand sqcmd = new SqlCommand();
        sqcmd.CommandType = CommandType.StoredProcedure;
        sqcmd.CommandText = "WO_ClanApplyToJoin";
        sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);
        sqcmd.Parameters.AddWithValue("@in_ClanID", ClanID);
        sqcmd.Parameters.AddWithValue("@in_ApplicationText", Note);

        if (!CallWOApi(sqcmd))
            return;

        Response.Write("WO_0");
    }

    void AnswerApplication()
    {
        string CustomerID = web.CustomerID();
        string ApplID = web.Param("ApplID");
        string Answer = web.Param("Answer");

        SqlCommand sqcmd = new SqlCommand();
        sqcmd.CommandType = CommandType.StoredProcedure;
        sqcmd.CommandText = "WO_ClanApplyAnswer";
        sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);
        sqcmd.Parameters.AddWithValue("@in_ClanApplicationID", ApplID);
        sqcmd.Parameters.AddWithValue("@in_Answer", Answer);

        if (!CallWOApi(sqcmd))
            return;

        Response.Write("WO_0");
    }

    void GetApplications()
    {
        // not used now, we'll get applications in login session poller in client
        throw new ApiExitException("use api_LoginSessionPoller::ClanGetApplications");
    }

    protected override void Execute()
    {
        if (!WoCheckLoginSession())
            return;

        string func = web.Param("func");
        if (func == "get")
            GetApplications();
        else if (func == "answer")
            AnswerApplication();
        else if (func == "apply")
            ApplyToJoin();
        else
            throw new ApiExitException("bad func");
    }
}
