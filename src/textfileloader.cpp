#define NOMINMAX

#include "textfileloader.h"

#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>

// =================================================================================================

TableDimensions TextFileLoader::ReadLines(const char * fileName, List<String>& fileLines, tLineFilter filter) {
    std::ifstream f(fileName);
    if (not f.is_open())
        return -1;
    std::string line;
    int rows = 0;
    int cols = 0;
    while (std::getline(f, line)) {
        String s(line);
        if (filter(s)) {
            rows++;
            cols = std::max(cols, int(line.length()));
            fileLines.Append(std::move(s));
        }
    }
    return TableDimensions (cols, rows);
}

// =================================================================================================
