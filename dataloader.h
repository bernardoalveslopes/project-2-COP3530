#ifndef DATALOADER_H
#define DATALOADER_H

#include <cstddef>
#include <exception>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "stock.h"

// Loads the S&P 500 Stock Data set from Kaggle (data/all_stocks_5yr.csv):
// https://www.kaggle.com/datasets/camnugent/sandp500
//
// Expected header and column order:
//   date,open,high,low,close,volume,Name
//   2013-02-08,15.07,15.12,14.63,14.75,8407500,AAL
//
// A few rows have an empty open, high, or low. Those values are stored as
// 0.0 and the record is kept, because the program only sorts by close,
// volume, and date. Rows missing date, close, volume, or Name, or holding
// text that is not a number, are skipped and counted.

// Files with Windows line endings leave a '\r' at the end of each line.
inline void removeCarriageReturn(std::string& line) {
    if (!line.empty() && line.back() == '\r')
        line.pop_back();
}

// Splits one CSV line on commas. The dataset has no quoted fields,
// so a plain split is enough.
inline std::vector<std::string> splitCsvLine(const std::string& line) {
    std::vector<std::string> fields;
    std::size_t start = 0;
    std::size_t comma = line.find(',');

    while (comma != std::string::npos) {
        fields.push_back(line.substr(start, comma - start));
        start = comma + 1;
        comma = line.find(',', start);
    }

    fields.push_back(line.substr(start));

    return fields;
}

// Reads open, high, or low. An empty field means the value is missing.
inline double parseOptionalPrice(const std::string& field) {
    if (field.empty())
        return 0.0;

    return std::stod(field);
}

// Fills stock from one data row. Returns false if the row can't be used.
inline bool parseStockLine(const std::string& line, Stock& stock) {
    std::vector<std::string> fields = splitCsvLine(line);

    if (fields.size() != 7)
        return false;

    // date, close, volume, and Name are needed to sort and display a record
    if (fields[0].empty() || fields[4].empty() || fields[5].empty() || fields[6].empty())
        return false;

    // std::stod and std::stoll throw an exception on text that is not a number:
    // https://en.cppreference.com/w/cpp/string/basic_string/stof
    // https://en.cppreference.com/w/cpp/string/basic_string/stol
    try {
        stock.date = fields[0];
        stock.open = parseOptionalPrice(fields[1]);
        stock.high = parseOptionalPrice(fields[2]);
        stock.low = parseOptionalPrice(fields[3]);
        stock.close = std::stod(fields[4]);
        stock.volume = std::stoll(fields[5]);
        stock.Name = fields[6];
    } catch (const std::exception&) {
        return false;
    }

    return true;
}

// Reads every record in filePath into stocks, replacing what was there.
// skippedRows is set to the number of rows that could not be used.
// Returns false if the file can't be opened or has an unexpected header.
inline bool loadStocks(const std::string& filePath, std::vector<Stock>& stocks,
                       int& skippedRows) {

    const std::string expectedHeader = "date,open,high,low,close,volume,Name";

    stocks.clear();
    skippedRows = 0;

    std::ifstream file(filePath);

    if (!file.is_open()) {
        std::cerr << "Could not open " << filePath << std::endl;
        return false;
    }

    std::string line;

    std::getline(file, line);
    removeCarriageReturn(line);

    if (line != expectedHeader) {
        std::cerr << "Unexpected header in " << filePath << std::endl;
        std::cerr << "Expected: " << expectedHeader << std::endl;
        return false;
    }

    while (std::getline(file, line)) {
        removeCarriageReturn(line);

        if (line.empty())
            continue;

        Stock stock;

        if (parseStockLine(line, stock))
            stocks.push_back(stock);
        else
            skippedRows++;
    }

    return true;
}

#endif
