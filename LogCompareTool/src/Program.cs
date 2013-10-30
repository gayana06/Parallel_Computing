using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LogCompare
{
    class Program
    {
        private Dictionary<string,string> serialList;
        private Dictionary<string, string> parallelList;
        private const char SEP_HYP = '-';
        private const string SEP_COMMA = ",";
        private const char SEP_COMMA_CHAR = ',';


        public Program()
        {
            this.serialList = new Dictionary<string, string>();
            this.parallelList = new Dictionary<string, string>();
            
        }

        static void Main(string[] args)
        {
            Program p = new Program();
            p.ReadFile("log-s.txt", ref p.serialList);
            p.ReadFile("log-p.txt", ref p.parallelList);
            Console.WriteLine("Start comparison");
            p.CompareData();
            Console.WriteLine("Done");
        }

        private void CompareData()
        {
            foreach (var vals in serialList)
            {
                foreach (var valp in parallelList)
                {
                    if (vals.Key == valp.Key)
                    {
                        string[] sarray = vals.Value.Split(SEP_COMMA_CHAR);
                        string[] parray = valp.Value.Split(SEP_COMMA_CHAR);
                        NoteDifference(vals.Key,sarray,parray);
                        break;
                    }
                }
            }
        }

        private void NoteDifference(string gen,string[] sarray, string[] parray)
        {
            StreamWriter write = new StreamWriter("compared_OK.txt", true);
            StreamWriter writef = new StreamWriter("compared_FAIL.txt", true);
            bool hasVal = false;
            write.WriteLine(string.Empty);
            for (int i = 0; i < sarray.Length; i++)
            {
                hasVal = false;
                for (int j = 0; j < parray.Length; j++)
                {
                    if (sarray[i] == parray[j])
                    {
                        write.WriteLine("Gen: "+gen+" Serial :" + sarray[i] + " compared OK");
                        hasVal = true;
                        break;
                    }                    
                }
                if (!hasVal)
                {
                    writef.WriteLine("Gen: " + gen + " Serial : " + sarray[i] + " compared FAILED");
                }
                write.Flush();
                writef.Flush();
            }
            if (sarray.Length < parray.Length)
            {
                for (int i = 0; i < parray.Length; i++)
                {
                    hasVal = false;
                    for (int j = 0; j < sarray.Length; j++)
                    {
                        if (parray[i] == sarray[j])
                        {
                            hasVal = true;
                            break;
                        }
                    }
                    if (!hasVal)
                    {
                        writef.WriteLine("Gen: " + gen + " Additional in parallel : " + parray[i]);
                    }
                }
            }
            write.Flush();
            write.Close();
            writef.Flush();
            writef.Close();
        }

        private void ReadFile(string path, ref Dictionary<string, string> data)
        {
            StreamReader read = new StreamReader(path);
            string line = string.Empty;
            List<string> tmpList = new List<string>();
            string[] tmpstr=new string[2];
            StringBuilder sb = new StringBuilder();
            int index = -1;
            while((line = read.ReadLine()) != null)
            {
                if (line.Trim() != string.Empty)
                {
                    tmpstr = line.Split(SEP_HYP);
                    if (!tmpList.Contains(tmpstr[0].Trim()))
                    {
                        index++;
                        if (index > 0)
                        {
                            string tmp = sb.ToString().Substring(0, sb.ToString().Length - 1);
                            data.Add(tmpList[index-1], tmp.Trim());
                            sb.Clear();
                        }
                        tmpList.Add(tmpstr[0].Trim());
                        sb.Append(tmpstr[1].Trim() + SEP_COMMA);
                    }
                    else
                    {
                        sb.Append(tmpstr[1].Trim() + SEP_COMMA);
                    }
                    
                }
            }
            if (index > 0)
            {
                string tmp = sb.ToString().Substring(0, sb.ToString().Length - 1);
                data.Add(tmpList[index], tmp);
                sb.Clear();
            }
            read.Close();

        }
    }
}
