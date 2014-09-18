using System;
using System.Collections.Generic;
using System.Web;
using System.Configuration;

/// <summary>
/// Summary description for BuyItem3
/// </summary>
public class BuyItem3
{
    static public bool IsGD(string BuyIdxStr)
    {
        int BuyIdx = Convert.ToInt32(BuyIdxStr);
        if (BuyIdx >= 5 && BuyIdx <= 8)
            return true;
        return false;
    }

    static public string GetBuyProcFromIdx(string BuyIdxStr)
    {
        int BuyIdx = Convert.ToInt32(BuyIdxStr);
        switch (BuyIdx)
        {
            // common buys - GD
            case 5:
            case 6:
            case 7:
            case 8:
                return "WO_BuyItem_GD";

            // GP
            case 1:
            case 2:
            case 3:
            case 4:
                if (ConfigurationManager.AppSettings.Get("WO_Region") == "RU")
                    throw new ApiExitException("GP Buy in russian region");

                return "WO_BuyItem_GP";

            // GameNet currency
            case 9:
            case 10:
            case 11:
            case 12:
                if (ConfigurationManager.AppSettings.Get("WO_Region") != "RU")
                    throw new ApiExitException("Gamenet Buy not in russian region");

                return "WO_BuyItem_GNA";
        }

        throw new ApiExitException("bad BuyIdx");
    }

    static public string GetBuyDaysFromIdx(string BuyIdxStr)
    {
        int BuyIdx = Convert.ToInt32(BuyIdxStr);
        switch (BuyIdx)
        {
            case 1: return "1";
            case 2: return "7";
            case 3: return "30";
            case 4: return "2000";
            case 5: return "1";
            case 6: return "7";
            case 7: return "30";
            case 8: return "2000";
            case 9: return "1";
            case 10: return "7";
            case 11: return "30";
            case 12: return "2000";
        }

        throw new ApiExitException("bad BuyIdx");
    }
}
