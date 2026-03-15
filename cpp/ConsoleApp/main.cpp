#include "../RiskSystem/SerialTradeLoader.h"
#include "../RiskSystem/StreamingTradeLoader.h"
#include "../Models/ScalarResults.h"
#include "../RiskSystem/SerialPricer.h"
#include "../RiskSystem/ParallelPricer.h"
#include "../RiskSystem/ScreenResultPrinter.h"

#include <memory>
#include <iostream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>

int getch() {
    termios oldt{}, newt{};
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    const int ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}
#endif

namespace
{
    // Set to true to use StreamingTradeLoader.
    constexpr bool UseStreamingTradeLoader = true;

    // If true, uses ParallelPricer. Otherwise, uses SerialPricer.
    constexpr bool UseParallelPricer = true;
}

int main() {
    ScalarResults results;

    if (UseStreamingTradeLoader) {
        // ReSharper disable once CppDFAUnreachableCode
        if (UseParallelPricer) {
            auto session = ParallelPricer::start(results);
            StreamingTradeLoader::streamTrades([&session](std::unique_ptr<ITrade> trade) {
                session.submit(std::move(trade));
            });
            session.finish();
        } else {
            // ReSharper disable once CppDFAUnreachableCode
            SerialPricer pricer;
            StreamingTradeLoader::streamTrades([&pricer, &results](std::unique_ptr<ITrade> trade) {
                std::vector<std::vector<std::unique_ptr<ITrade>>> tradeBatch;
                tradeBatch.emplace_back();
                tradeBatch.back().push_back(std::move(trade));
                pricer.price(tradeBatch, results);
            });
        }
    } else {
        // ReSharper disable once CppDFAUnreachableCode
        const auto allTrades = SerialTradeLoader::loadTrades();

        if (UseParallelPricer) {
            ParallelPricer::price(allTrades, results);
        } else {
            // ReSharper disable once CppDFAUnreachableCode
            SerialPricer pricer;
            pricer.price(allTrades, results);
        }
    }

    ScreenResultPrinter::printResults(results);

    std::cout << "Press any key to exit.." << std::endl;
    getch();

    return 0;
}
