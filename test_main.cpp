
#include <iostream>
#include <vector>
#include "heapsort.h"

using namespace std;

int main() {

    vector<Stock> stocks = {
        {"2018-01-05", 100, 110, 95, 105, 5000, "AAPL"},
        {"2018-01-02", 200, 210, 190, 195, 8000, "MSFT"},
        {"2018-01-04", 50, 60, 45, 55, 3000, "AMZN"},
        {"2018-01-01", 150, 160, 140, 155, 9000, "GOOG"},
        {"2018-01-03", 75, 85, 70, 80, 6000, "FB"},
        {"2018-01-06", 300, 310, 290, 305, 2000, "TSLA"},
        {"2018-01-07", 125, 135, 120, 130, 7000, "NFLX"},
        {"2018-01-08", 250, 260, 240, 245, 4000, "IBM"},
        {"2018-01-09", 175, 185, 170, 180, 1000, "ORCL"},
        {"2018-01-10", 225, 235, 220, 230, 10000, "INTC"}
    };

    int fieldChoice;
    int orderChoice;

    cout << "Choose field to sort by:\n";
    cout << "1. Closing Price\n";
    cout << "2. Volume\n";
    cout << "3. Date\n";
    cin >> fieldChoice;

    cout << "Choose order:\n";
    cout << "1. Ascending\n";
    cout << "2. Descending\n";
    cin >> orderChoice;

    SortField field;

    if (fieldChoice == 1)
        field = CLOSE;
    else if (fieldChoice == 2)
        field = VOLUME;
    else
        field = DATE;

    SortOrder order;

    if (orderChoice == 1)
        order = ASCENDING;
    else
        order = DESCENDING;

    long long comparisons = 0;

    heapSort(stocks, field, order, comparisons);

    cout << "\nSorted Stocks:\n";

    for (const Stock& stock : stocks) {
        cout << stock.Name << " | "
             << stock.date << " | "
             << stock.close << " | "
             << stock.volume << endl;
    }

    cout << "\nComparisons: " << comparisons << endl;

    return 0;
}
