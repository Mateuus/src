using System;
using System.Collections.Generic;
using System.Windows.Forms;
using System.IO;

namespace LangPackFixer
{
    static class Program
    {
        static LangPack leng_;

        static void DebugLog(string msg)
        {
            Console.Write(msg);
        }

        static void FixLangPack(string fname)
        {
            DebugLog(string.Format("{0} - ", fname));
            LangPack lin = new LangPack(fname);
            DebugLog(string.Format("{0} lines\n", lin.lines_.Count));

            LangPack lout = new LangPack();
            StreamWriter SW = new StreamWriter(fname + "_missing.txt");
            int missing = 0;

            // recreate new langpack
            foreach (LangPack.Entry e in leng_.lines_)
            {
                if (e.value == null)
                {
                    lout.lines_.Add(e);
                    continue;
                }

                LangPack.Entry e2;
                if (lin.map_.TryGetValue(e.name, out e2))
                {
                    // have translated value
                    lout.lines_.Add(e2);
                    continue;
                }
                else
                {
                    missing++;
                    // not translated line
                    e2 = new LangPack.Entry();
                    e2.name = e.name;
                    e2.value = "#" + e.value;
                    lout.lines_.Add(e2);

                    SW.WriteLine(e.name);
                }
            }

            SW.Close();
            if (missing == 0)
                File.Delete(fname + "_missing.txt");

            DebugLog(string.Format("\t{0} missing\n", missing));

            File.Delete(fname + ".bak");
            File.Move(fname, fname + ".bak");
            lout.Write(fname);
        }

        static void DoLangPacks()
        {
            leng_ = new LangPack("english.lang");
            DebugLog(string.Format("Readed engligh.lang - {0} lines\n", leng_.lines_.Count));

            FixLangPack("german.lang");
            FixLangPack("french.lang");
            FixLangPack("italian.lang");
            FixLangPack("spanish.lang");
            FixLangPack("russian.lang");
        }

        static void Main(string[] args)
        {
            if (args.Length == 0)
            {
                Console.WriteLine("usage: LangPackFixer <dir_to_langpack>");
                return;
            }

            try
            {

                System.IO.Directory.SetCurrentDirectory(args[0]);

                DoLangPacks();
            }
            catch (Exception e)
            {
                Console.WriteLine("CRASH: " + e.Message);
                //Console.WriteLine(e.ToString());
            }
        }
    }
}
