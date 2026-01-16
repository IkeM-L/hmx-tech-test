using HmxLabs.TechTest.Models;
using HmxLabs.TechTest.RiskSystem;

namespace HmxLabs.TechTest.ConsoleApp
{
    public class Program
    {
        public static void Main(string[] args_)
        {
            // Prefer release compiled dll
            string assemblyPath = @"../../../../Pricers/bin/Release/net8.0/Pricers.dll";
            if(!File.Exists(assemblyPath))
                assemblyPath = @"../../../../Pricers/bin/Debug/net8.0/Pricers.dll";
            if (!File.Exists(assemblyPath))
            {
                Console.WriteLine("Please update the location of Pricers.dll");
                return;
            }
            
            
            var tradeLoader = new SerialTradeLoader();

            var allTrades = tradeLoader.LoadTrades();
            var results = new ScalarResults();
            //var pricer = new SerialPricer();
            var pricer = new ParallelPricer();
            pricer.Price(allTrades, results, assemblyPath);

            var screenPrinter = new ScreenResultPrinter();
            screenPrinter.PrintResults(results);

            Console.WriteLine("Press any key to exit..");
            Console.ReadKey();
        }
    }
}