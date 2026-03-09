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
#include <fcntl.h>

int _getch() {
    struct termios oldt, newt;
    int ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}
#endif

namespace
{
    // Set to true to use StreamingTradeLoader.
    constexpr bool UseStreamingTradeLoader = false;

    // If true, uses ParallelPricer. Otherwise, uses SerialPricer.
    constexpr bool UseParallelPricer = true;
}

int main(int argc, char* argv[]) {
    ScalarResults results;

    if (UseStreamingTradeLoader) {
        if (UseParallelPricer) {
            ParallelPricer pricer;
            pricer.start(&results);
            try {
                StreamingTradeLoader::streamTrades([&pricer](std::unique_ptr<ITrade> trade) {
                    pricer.submit(std::move(trade));
                });
            } catch (...) {
                pricer.finish();
                throw;
            }
            pricer.finish();
        } else {
            SerialPricer pricer;
            StreamingTradeLoader::streamTrades([&pricer, &results](std::unique_ptr<ITrade> trade) {
                std::vector<std::vector<std::unique_ptr<ITrade>>> tradeBatch;
                tradeBatch.emplace_back();
                tradeBatch.back().push_back(std::move(trade));
                pricer.price(tradeBatch, &results);
            });
        }
    } else {
        SerialTradeLoader tradeLoader;
        const auto allTrades = SerialTradeLoader::loadTrades();

        if (UseParallelPricer) {
            ParallelPricer pricer;
            pricer.price(allTrades, &results);
        } else {
            SerialPricer pricer;
            pricer.price(allTrades, &results);
        }
    }

    ScreenResultPrinter::printResults(results);

    std::cout << "Press any key to exit.." << std::endl;
    _getch();

    return 0;
}
