using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Data;
using System.Text;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;
using System.Data.SqlClient;

public partial class api_SkillReset : WOApiWebPage
{
    protected override void Execute()
    {
        if (!WoCheckLoginSession())
            return;

        string CustomerID = web.CustomerID();
        string LoadoutID = web.Param("LoadoutID");

        // temp developers version of skill reset, no item yet
        // todo, buy a special skill reset item
        {
        }

        // reset
        {
            // reassemble skill string
            SqlCommand sqcmd = new SqlCommand();
            sqcmd.CommandType = CommandType.StoredProcedure;
            sqcmd.CommandText = "WO_SkillReset";
            sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);
            sqcmd.Parameters.AddWithValue("@in_LoadoutID", LoadoutID);

            if (!CallWOApi(sqcmd))
                return;
        }

        Response.Write("WO_0");
    }
}
