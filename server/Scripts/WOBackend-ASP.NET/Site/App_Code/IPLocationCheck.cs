using System;
using System.Collections.Generic;
using System.Web;
using System.Configuration;

/// <summary>
/// Summary description for IPLocationCheck
/// </summary>
public class IPLocationCheck
{
    static string DB_PATH = null;
    static string LIC_PATH = null;

    static public string GetCountryCode(string strIPAddress)
    {
        if (DB_PATH == null)
        {
            string path = HttpContext.Current.Server.MapPath("~/App_Data");
            DB_PATH = path + "/IP-COUNTRY.BIN";
            LIC_PATH = path + "/License.key";
        }

        IP2Location.IPResult oIPResult = new IP2Location.IPResult();
        IP2Location.Component oIP2Location = new IP2Location.Component();
        try
        {
            //Set Database Paths
            oIP2Location.IPDatabasePath = DB_PATH;
            oIP2Location.IPLicensePath = LIC_PATH;

            oIPResult = oIP2Location.IPQuery(strIPAddress);
            switch (oIPResult.Status.ToString())
            {
                case "OK":
                    return oIPResult.CountryShort;
                default:
                    return null;
            }
        }
        catch (Exception ex)
        {
            throw new ApiExitException(ex.Message);
        }
        finally
        {
            oIPResult = null;
            oIP2Location = null;
        }
    }

    static public bool IsRegionLocked(string strIpAddress, int CustomerID)
    {
        // only US-EN have region lock
        if (ConfigurationManager.AppSettings.Get("WO_Region") != "EN")
            return false;

        // exclude IP from checks
        if (strIpAddress.StartsWith("31.25.224."))  // Syncopate servers 31.25.224.0/22
            return false;
        else if (strIpAddress == "81.177.30.7")     
            return false;
        else if (strIpAddress == "194.190.17.173")  // Syncopate office
            return false;

        // exclude users
        if (CustomerID == 1282052887 || // denis
            CustomerID == 1288049460 ) // rmarchenko
            return false;

        string code = GetCountryCode(strIpAddress);
        switch (code)
        {
            case "UA": //UKRAINE    
            case "RU": //RUSSIA
            case "BY": //BELARUS
            case "KZ": //KAZAKHSTAN
                return true;
            default:
                return false;
        }
    }
}
