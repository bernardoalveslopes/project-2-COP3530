// Tests for mergesort.h, plus loading the real dataset with dataloader.h.
// Build: g++ -std=c++14 -O2 test_mergesort.cpp -o test_mergesort
// Run it from the repo folder so it finds data/all_stocks_5yr.csv.
// Like test_main.cpp, this has its own main(), so don't compile it with main.cpp.

#include <iostream>
#include <string>
#include <vector>
#include "dataloader.h"
#include "mergesort.h"

using namespace std;

int failures = 0;

void check(bool passed, const string& name) {
    if (passed) {
        cout << "  PASS  " << name << endl;
    } else {
        cout << "  FAIL  " << name << endl;
        failures++;
    }
}

// No record should come before the record in front of it
bool isSorted(const vector<Stock>& stocks, SortField field, SortOrder order) {
    for (int i = 1; i < (int)stocks.size(); i++) {
        if (comesBefore(stocks[i], stocks[i - 1], field, order))
            return false;
    }

    return true;
}

// Before sorting, each record's original position is stored in open
// (open is never a sort field, so it just rides along with the record).
void tagPositions(vector<Stock>& stocks) {
    for (int i = 0; i < (int)stocks.size(); i++)
        stocks[i].open = i;
}

// Every original position must show up exactly once after sorting,
// so no record was lost or copied twice
bool keptEveryRecord(const vector<Stock>& stocks) {
    vector<bool> seen(stocks.size(), false);

    for (int i = 0; i < (int)stocks.size(); i++) {
        int position = (int)stocks[i].open;

        if (position < 0 || position >= (int)stocks.size() || seen[position])
            return false;

        seen[position] = true;
    }

    return true;
}

// Neighbors with equal keys must still be in their original order
bool isStable(const vector<Stock>& stocks, SortField field, SortOrder order) {
    for (int i = 1; i < (int)stocks.size(); i++) {
        bool equalKeys = !comesBefore(stocks[i - 1], stocks[i], field, order) &&
                         !comesBefore(stocks[i], stocks[i - 1], field, order);

        if (equalKeys && stocks[i].open < stocks[i - 1].open)
            return false;
    }

    return true;
}

Stock makeStock(const string& name, const string& date, double close, long long volume) {
    Stock stock;
    stock.date = date;
    stock.open = 0;
    stock.high = 0;
    stock.low = 0;
    stock.close = close;
    stock.volume = volume;
    stock.Name = name;
    return stock;
}

const SortField FIELDS[] = {CLOSE, VOLUME, DATE};
const SortOrder ORDERS[] = {ASCENDING, DESCENDING};
const string FIELD_NAMES[] = {"close", "volume", "date"};
const string ORDER_NAMES[] = {"ascending", "descending"};

// Sorts a copy of stocks every possible way and checks each result
void checkAllWays(const vector<Stock>& stocks, const string& label) {
    for (int f = 0; f < 3; f++) {
        for (int o = 0; o < 2; o++) {
            vector<Stock> copy = stocks;
            tagPositions(copy);

            long long comparisons = 0;
            mergeSort(copy, FIELDS[f], ORDERS[o], comparisons);

            bool passed = isSorted(copy, FIELDS[f], ORDERS[o]) &&
                          keptEveryRecord(copy) &&
                          isStable(copy, FIELDS[f], ORDERS[o]);

            check(passed, label + " by " + FIELD_NAMES[f] + " " + ORDER_NAMES[o] +
                          " (" + to_string(comparisons) + " comparisons)");
        }
    }
}

int main() {
    cout << "Small cases:" << endl;

    long long comparisons = -1;
    vector<Stock> empty;
    mergeSort(empty, CLOSE, ASCENDING, comparisons);
    check(empty.empty() && comparisons == 0, "empty vector");

    vector<Stock> one = {makeStock("AAPL", "2018-01-02", 170.5, 2500)};
    mergeSort(one, VOLUME, DESCENDING, comparisons);
    check(one.size() == 1 && one[0].Name == "AAPL" && comparisons == 0, "one record");

    // equal closes must stay in the order they started in (A, B, C, D)
    vector<Stock> ties = {
        makeStock("A", "2018-01-04", 50, 10),
        makeStock("B", "2018-01-03", 50, 20),
        makeStock("C", "2018-01-02", 50, 30),
        makeStock("D", "2018-01-01", 50, 40)
    };
    mergeSort(ties, CLOSE, DESCENDING, comparisons);
    check(ties[0].Name == "A" && ties[1].Name == "B" && ties[2].Name == "C" &&
          ties[3].Name == "D", "all keys equal keeps original order");

    vector<Stock> small = {
        makeStock("AAPL", "2018-01-05", 105, 5000),
        makeStock("MSFT", "2018-01-02", 195, 8000),
        makeStock("AMZN", "2018-01-04", 55, 3000),
        makeStock("GOOG", "2018-01-01", 155, 9000),
        makeStock("FB",   "2018-01-03", 80, 6000),
        makeStock("TSLA", "2018-01-02", 305, 2000),
        makeStock("NFLX", "2018-01-07", 105, 7000),
        makeStock("IBM",  "2018-01-08", 245, 3000),
        makeStock("ORCL", "2018-01-09", 180, 1000),
        makeStock("INTC", "2018-01-05", 230, 10000)
    };
    checkAllWays(small, "10 records");

    cout << "\nFull dataset:" << endl;

    vector<Stock> data;
    int skippedRows = 0;

    if (!loadStocks("data/all_stocks_5yr.csv", data, skippedRows)) {
        check(false, "load data/all_stocks_5yr.csv");
        return 1;
    }

    check(data.size() == 619040 && skippedRows == 0, "loaded all 619040 records, 0 skipped");
    checkAllWays(data, "619040 records");

    if (failures == 0)
        cout << "\nAll tests passed." << endl;
    else
        cout << "\n" << failures << " test(s) FAILED." << endl;

    return failures == 0 ? 0 : 1;
}
