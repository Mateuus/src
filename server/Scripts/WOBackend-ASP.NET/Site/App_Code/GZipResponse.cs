using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Text;
using System.IO;
using System.IO.Compression;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;

/// <summary>
/// Summary description for WebHelper
/// </summary>
public class GZIPResponse
{
    MemoryStream output = new MemoryStream();
    bool useGZip_ = true;

    public Int64 Length
    {
        get
        {
            return output.Length;
        }
    }

    public void UseCompression(bool use)
    {
        useGZip_ = use;
    }

    public void Write(string str)
    {
        byte[] data = Encoding.UTF8.GetBytes(str.ToString());
        output.Write(data, 0, data.Length);
    }

    public void BinaryWrite(byte[] data)
    {
        output.Write(data, 0, data.Length);
    }

    public void Flush(System.Web.HttpResponse resp)
    {
        byte[] data1 = output.ToArray();

        if (!useGZip_ || data1.Length < 200)
        {
            resp.BinaryWrite(data1);
            return;
        }

        // compress
        MemoryStream ms = new MemoryStream();
        GZipStream GZip = new GZipStream(ms, CompressionMode.Compress);
        GZip.Write(data1, 0, data1.Length);
        GZip.Close();

        // get compressed bytes
        byte[] data2 = ms.ToArray();
        ms.Close();

        // if compression failed (more data)
        if (data2.Length >= data1.Length)
        {
            resp.BinaryWrite(data1);
            return;
        }

        resp.BufferOutput = true;
        resp.ContentType = "application/octet-stream";
        resp.Write("$1");
        //resp.Write(string.Format("SIZE: {0} vs {1}<br>", data1.Length, data2.Length));

        resp.BinaryWrite(data2);
        resp.Flush();
    }
}
