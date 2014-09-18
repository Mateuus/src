using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Data;
using System.Text;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;
using System.Data.SqlClient;

public partial class api_LoadoutModify : WOApiWebPage
{
    protected override void Execute()
    {
        if (!WoCheckLoginSession())
            return;

        string CustomerID = web.CustomerID();
        string LoadoutID = web.Param("LoadoutID");
        string i1 = web.Param("i1");
        string i2 = web.Param("i2");
        string i3 = web.Param("i3");
        string i4 = web.Param("i4");
        string i5 = web.Param("i5");
        string i6 = web.Param("i6");
        string i7 = web.Param("i7");
        string i8 = web.Param("i8");
        string i9 = web.Param("i9");
        string i10 = web.Param("i10");
        string i11 = web.Param("i11");
        string i12 = web.Param("i12");
        string i13 = web.Param("i13");

        SqlCommand sqcmd = new SqlCommand();
        sqcmd.CommandType = CommandType.StoredProcedure;
        sqcmd.CommandText = "WO_LoadoutModify";
        sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);
        sqcmd.Parameters.AddWithValue("@in_LoadoutID", LoadoutID);
        sqcmd.Parameters.AddWithValue("@i1", i1);
        sqcmd.Parameters.AddWithValue("@i2", i2);
        sqcmd.Parameters.AddWithValue("@i3", i3);
        sqcmd.Parameters.AddWithValue("@i4", i4);
        sqcmd.Parameters.AddWithValue("@i5", i5);
        sqcmd.Parameters.AddWithValue("@i6", i6);
        sqcmd.Parameters.AddWithValue("@i7", i7);
        sqcmd.Parameters.AddWithValue("@i8", i8);
        sqcmd.Parameters.AddWithValue("@i9", i9);
        sqcmd.Parameters.AddWithValue("@i10", i10);
        sqcmd.Parameters.AddWithValue("@i11", i11);
        sqcmd.Parameters.AddWithValue("@i12", i12);
        sqcmd.Parameters.AddWithValue("@i13", i13);

        if (!CallWOApi(sqcmd))
            return;

        reader.Read();
        string loadout = getString("Loadout");

        Response.Write("WO_0");
        Response.Write(string.Format("{0}", loadout));
    }
}
