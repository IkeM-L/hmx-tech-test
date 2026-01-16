using System.Text;
using HmxLabs.TechTest.Models;

namespace HmxLabs.TechTest.RiskSystem
{
    public class ScreenResultPrinter
    {
        public void PrintResults(ScalarResults results)
        {
            var stringBuilder = new StringBuilder();
            
            if(results == null || !results.Any())
                return;
            
            foreach (var result in results)
            {
                // Write code here to print out the results such that we have : 
                // TradeID : Result : Error
                // If there is no result then the output should be :
                // TradeID : Error
                // If there is no error the output should be :
                // TradeID : Result
                stringBuilder.Append(result.TradeId);
                if(result.Result != null)
                {
                    stringBuilder.Append(" : ");
                    stringBuilder.Append(result.Result);
                }
                if(result.Error != null)
                {
                    stringBuilder.Append(" : ");
                    stringBuilder.Append(result.Error);
                }
                stringBuilder.AppendLine();
            }
            
            Console.WriteLine(stringBuilder.ToString());
            
        }
    }
}