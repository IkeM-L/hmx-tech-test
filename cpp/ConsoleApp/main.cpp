#include "../RiskSystem/SerialTradeLoader.h"
#include "../RiskSystem/StreamingTradeLoader.h"
#include "../Models/ScalarResults.h"
#include "../RiskSystem/SerialPricer.h"
#include "../RiskSystem/ParallelPricer.h"
#include "../RiskSystem/ScreenResultPrinter.h"

#include <iostream>
#include <string>

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
    // Note: with the current class design, the streaming loader performs pricing internally.
    constexpr bool UseStreamingTradeLoader = false;

    // Only used when UseStreamingTradeLoader == false.
    // If true, uses ParallelPricer. Otherwise uses SerialPricer.
    constexpr bool UseParallelPricer = true;
}

int main(int argc, char* argv[]) {
    ScalarResults results;

    if (UseStreamingTradeLoader) {
        StreamingTradeLoader tradeLoader;
        tradeLoader.loadAndPrice(&results);
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

    ScreenResultPrinter screenPrinter;
    screenPrinter.printResults(results);

    std::cout << "Press any key to exit.." << std::endl;
    _getch();

    return 0;
}