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
    constexpr bool UseStreamingTradeLoader = true;

    // If true, uses ParallelPricer. Otherwise uses SerialPricer.
    constexpr bool UseParallelPricer = true;
}

int main(int argc, char* argv[]) {
    ScalarResults results;

    if (UseStreamingTradeLoader) {
        StreamingTradeLoader tradeLoader;

        if (UseParallelPricer) {
            ParallelPricer pricer;
            pricer.start(&results);
            try {
                tradeLoader.streamTrades([&pricer](ITrade* trade) {
                    pricer.submit(trade);
                });
            } catch (...) {
                pricer.finish();
                throw;
            }
            pricer.finish();
        } else {
            SerialPricer pricer;
            tradeLoader.streamTrades([&pricer, &results](ITrade* trade) {
                std::unique_ptr<ITrade> tradeOwner(trade);
                pricer.price({{tradeOwner.get()}}, &results);
            });
        }
    } else {
        SerialTradeLoader tradeLoader;
        auto allTrades = tradeLoader.loadTrades();

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
