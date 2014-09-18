using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Data;
using System.Text;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;
using System.Data.SqlClient;
using System.IO;
using System.IO.Compression;

public partial class api_SrvUploadLogFile : WOApiWebPage
{
    public string PICTURES_DIR = "C:\\inetpub\\wwwroot\\intrawo\\pictures\\";

    public static void CopyStream(Stream input, Stream output) 
    { 
        byte[] buffer = new byte[32768]; 
        int read; 
        while ((read = input.Read(buffer, 0, buffer.Length)) > 0) 
        { 
            output.Write(buffer, 0, read); 
        } 
    }
    
    void WriteAttachedImages()
    {
        for (int jpgIdx = 0; jpgIdx < 99; jpgIdx++)
        {
            string str1 = string.Format("jpgFile{0}", jpgIdx);

            HttpPostedFile jpgFile = this.Request.Files[str1];
            if(jpgFile == null)
                break;

            // fname in form GS_1cc625ebba7a005_1288443711_6497d172.jpg
            string[] parts = jpgFile.FileName.Split('_');
            int CustomerID = Convert.ToInt32(parts[2]);

            // create directory based on customer id
            string dir = string.Format("{0}{1}", PICTURES_DIR, CustomerID);
            System.IO.Directory.CreateDirectory(dir);

            // save jpg there
            string fname = string.Format("{0}\\{1}_{2}{3:00}{4:00}-{5:00}{6:00}.jpg", 
                dir, CustomerID,
                DateTime.Now.Year, DateTime.Now.Month, DateTime.Now.Day,
                DateTime.Now.Hour, DateTime.Now.Minute);

            FileStream fs = new FileStream(fname, System.IO.FileMode.Create, System.IO.FileAccess.Write);
            CopyStream(jpgFile.InputStream, fs);
            fs.Close();
        }
    }

    protected override void Execute()
    {
        string skey1 = web.Param("skey1");
        if (skey1 != SERVER_API_KEY)
            throw new ApiExitException("bad key");

        string sessionId = web.Param("sessionId");

        HttpPostedFile logFile = this.Request.Files["logFile"];
        if (logFile == null)
            throw new ApiExitException("no logFile");
        int logSize = Convert.ToInt32(web.Param("logSize"));

        GZipStream GZip = new GZipStream(logFile.InputStream, CompressionMode.Decompress);
        Byte[] logData = new Byte[logSize];
        GZip.Read(logData, 0, logSize);
        GZip.Close();

        // create log dir
        string dir = string.Format("c:\\inetpub\\logss\\{0}-{1:00}-{2:00}",
            DateTime.Now.Year, DateTime.Now.Month, DateTime.Now.Day);
        System.IO.Directory.CreateDirectory(dir);

        // Open file for writing
        string fname = string.Format("{0}\\GS_{1}.txt", dir, sessionId);
        FileStream fs = new FileStream(fname, System.IO.FileMode.Create, System.IO.FileAccess.Write);
        fs.Write(logData, 0, logData.Length);
        fs.Close();

        // check if we have crash dump file
        HttpPostedFile dmpFile = this.Request.Files["dmpFile"];
        if(dmpFile != null)
        {
            int dmpSize = Convert.ToInt32(web.Param("dmpSize"));

            GZip = new GZipStream(dmpFile.InputStream, CompressionMode.Decompress);
            Byte[] dmpData = new Byte[dmpSize];
            GZip.Read(dmpData, 0, dmpSize);
            GZip.Close();

            // create crash logs dir
            dir = string.Format("c:\\inetpub\\logss_CRASH");
            System.IO.Directory.CreateDirectory(dir);

            // save log there
            fname = string.Format("{0}\\GS_{1}.txt", dir, sessionId);
            fs = new FileStream(fname, System.IO.FileMode.Create, System.IO.FileAccess.Write);
            fs.Write(logData, 0, logData.Length);
            fs.Close();

            // save dump there
            fname = string.Format("{0}\\GS_{1}.dmp", dir, sessionId);
            fs = new FileStream(fname, System.IO.FileMode.Create, System.IO.FileAccess.Write);
            fs.Write(dmpData, 0, dmpData.Length);
            fs.Close();
        }

        WriteAttachedImages();

        Response.Write("WO_0");
    }
}
