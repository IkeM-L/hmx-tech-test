#ifndef SCREENRESULTPRINTER_H
#define SCREENRESULTPRINTER_H

#include "../Models/ScalarResults.h"

class ScreenResultPrinter {
public:
    /// Prints one output line per trade result.
    static void printResults(const ScalarResults& results);
};

#endif // SCREENRESULTPRINTER_H
