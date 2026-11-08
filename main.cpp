#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include "stock.h"
#include "dataloader.h"
#include "heapsort.h"
#include "mergesort.h"

using namespace std;

// Enter tries the csv next to the program first, then the data folder
// (that's where it is in our repo)
const string DEFAULT_FILE_NAME = "all_stocks_5yr.csv";
const string DEFAULT_DATA_PATH = "data/all_stocks_5yr.csv";
const string RESULTS_FILE = "results.csv";
const string CHART_FILE = "chart.html";

// algorithm ids, numbered the same as the algorithm menu
const int MERGE_SORT = 1;
const int HEAP_SORT = 2;
const int COMPARE_BOTH = 3;

// how many # the slowest algorithm gets in the bar chart
const int BAR_WIDTH = 50;

// One finished sort run, so View Results can show it later.
// This is my own struct for the results screen, not a copy of Stock.
struct SortResult {
    string algorithm;
    SortField field;
    SortOrder order;
    int records;
    double timeMs;
    long long comparisons;
    double memoryMB;
    bool sortedCorrectly;
};

// ---------- input helpers ----------

// Checks if text is a whole number between low and high.
// I check every character is a digit BEFORE calling stoi, because stoi
// crashes the program on letters. That also rejects stuff like "3abc".
bool parseNumber(const string& text, int low, int high, int& number) {
    // must be 1 to 9 digits (more than 9 could be too big for an int)
    if (text.empty() || text.size() > 9)
        return false;

    for (int i = 0; i < (int)text.size(); i++) {
        if (text[i] < '0' || text[i] > '9')
            return false;
    }

    int value = stoi(text);

    if (value < low || value > high)
        return false;

    number = value;
    return true;
}

// Reads one line from the keyboard. Returns false if input got closed
// (Ctrl+Z), so the program can quit instead of looping forever.
bool readLine(string& line) {
    if (!getline(cin, line))
        return false;

    removeCarriageReturn(line);  // teammates' helper from dataloader.h
    return true;
}

// Keeps asking until the user types a number between low and high.
// I read whole lines instead of using cin >> because typing letters
// breaks cin >> and the menu would spin forever.
// Returns -1 if input is closed.
int readChoice(int low, int high) {
    string line;

    while (true) {
        cout << "Enter choice (" << low << "-" << high << "): ";

        if (!readLine(line))
            return -1;

        int choice;
        if (parseNumber(line, low, high, choice))
            return choice;

        cout << "Invalid input. Please enter a number from "
             << low << " to " << high << "." << endl;
    }
}

// Same as readChoice, but just pressing Enter gives back defaultValue.
// Used for "how many records", where Enter means all of them.
int readNumberOrDefault(int low, int high, int defaultValue) {
    string line;

    while (true) {
        cout << "Enter a number (" << low << "-" << high
             << ", Enter = " << defaultValue << "): ";

        if (!readLine(line))
            return -1;

        if (line.empty())
            return defaultValue;

        int number;
        if (parseNumber(line, low, high, number))
            return number;

        cout << "Invalid input. Please enter a number from "
             << low << " to " << high << "." << endl;
    }
}

// ---------- names for printing ----------

// Turns the SortField enum into text for the screen
string fieldName(SortField field) {
    switch (field) {
        case CLOSE:
            return "Closing Price";
        case VOLUME:
            return "Trading Volume";
        case DATE:
            return "Date";
    }

    return "Unknown";
}

// Short lowercase version of the field for results.csv
string csvFieldName(SortField field) {
    switch (field) {
        case CLOSE:
            return "close";
        case VOLUME:
            return "volume";
        case DATE:
            return "date";
    }

    return "unknown";
}

// Turns the SortOrder enum into text
string orderName(SortOrder order) {
    if (order == ASCENDING)
        return "Ascending";

    return "Descending";
}

// Turns an algorithm id into its name
string algorithmName(int algorithm) {
    if (algorithm == MERGE_SORT)
        return "Merge Sort";

    return "Heap Sort";
}

// Figures out how many records to sort. recordLimit 0 means "all of them".
// Also guards against a limit bigger than what's loaded (after a reload).
int recordsToUse(const vector<Stock>& stocks, int recordLimit) {
    int n = (int)stocks.size();

    if (recordLimit > 0 && recordLimit < n)
        n = recordLimit;

    return n;
}

// ---------- screens ----------

// Prints the main menu plus the current settings, since option 3
// reuses whatever was picked last in option 2
void showMainMenu(const vector<Stock>& stocks, SortField field, SortOrder order,
                  int recordLimit) {
    cout << "\n========== MarketSort ==========" << endl;
    cout << "Records loaded: " << stocks.size() << endl;

    if (!stocks.empty()) {
        cout << "Current settings: " << fieldName(field) << ", " << orderName(order)
             << ", " << recordsToUse(stocks, recordLimit) << " records" << endl;
    }

    cout << "1. Load Stock Data" << endl;
    cout << "2. Sort and Analyze" << endl;
    cout << "3. Compare Algorithms" << endl;
    cout << "4. View Results" << endl;
    cout << "5. Exit" << endl;
}

// Prints the first few stocks as a small table (Symbol, Date, Close, Volume)
void printStockRows(const vector<Stock>& stocks, int count) {
    if ((int)stocks.size() < count)
        count = (int)stocks.size();

    cout << left << setw(4) << "#" << setw(8) << "Symbol" << setw(13) << "Date"
         << right << setw(10) << "Close" << setw(14) << "Volume" << endl;

    for (int i = 0; i < count; i++) {
        cout << left << setw(4) << i + 1
             << setw(8) << stocks[i].Name  // the loader keeps the symbol in Name
             << setw(13) << stocks[i].date
             << right << setw(10) << stocks[i].close
             << setw(14) << stocks[i].volume << endl;
    }

    cout << left;
}

// Checks if a file exists by trying to open it
bool fileExists(const string& path) {
    ifstream file(path);
    return file.is_open();
}

// Option 1: asks for the csv path and loads it with the teammates'
// loadStocks(). Returns false only if input got closed.
bool loadDataScreen(vector<Stock>& stocks) {
    cout << "\nEnter CSV path (press Enter for " << DEFAULT_FILE_NAME << "): ";

    string path;
    if (!readLine(path))
        return false;

    if (path.empty()) {
        if (fileExists(DEFAULT_FILE_NAME))
            path = DEFAULT_FILE_NAME;
        else
            path = DEFAULT_DATA_PATH;
    }

    cout << "Loading " << path << "... (takes a few seconds)" << endl;

    int skippedRows = 0;

    // loadStocks prints its own "Could not open" message if the file is missing
    if (!loadStocks(path, stocks, skippedRows)) {
        cout << "Load failed. No data is loaded." << endl;
        return true;
    }

    cout << "File loaded successfully!" << endl;
    cout << "Records: " << stocks.size() << " (skipped " << skippedRows << " bad rows)" << endl;
    cout << "Columns: Symbol, Date, Open, High, Low, Close, Volume" << endl;
    cout << "\nFirst 5 rows:" << endl;
    printStockRows(stocks, 5);

    return true;
}

// Walks through a sorted vector and checks every pair of neighbors.
// If the next one should have come BEFORE the current one, it's out of order.
// Uses comesBefore from stock.h so it matches the sort's own rule.
// (these checks are not added to the comparison count)
bool isSorted(const vector<Stock>& stocks, SortField field, SortOrder order) {
    for (int i = 1; i < (int)stocks.size(); i++) {
        if (comesBefore(stocks[i], stocks[i - 1], field, order))
            return false;
    }

    return true;
}

// Adds one line to results.csv for a finished run, so we have numbers
// for graphs in the report. Writes the header first if the file is new.
void appendToResultsFile(const SortResult& result) {
    bool newFile = !fileExists(RESULTS_FILE);

    ofstream out(RESULTS_FILE, ios::app);  // app = add to the end, don't erase

    if (!out.is_open()) {
        cout << "Could not write to " << RESULTS_FILE << endl;
        return;
    }

    if (newFile)
        out << "algorithm,field,records,time_ms,comparisons,memory_mb" << endl;

    out << result.algorithm << "," << csvFieldName(result.field) << ","
        << result.records << "," << fixed << setprecision(3) << result.timeMs << ","
        << result.comparisons << "," << setprecision(2) << result.memoryMB << endl;
}

// Runs one algorithm on its own copy of the first n records and fills in
// result. Returns false if the algorithm id is unknown.
bool runOneSort(const vector<Stock>& stocks, int n, SortField field, SortOrder order,
                int algorithm, SortResult& result) {
    // Copy the first n records. Each algorithm gets a fresh copy of the same
    // unsorted data, otherwise the second sort would get already-sorted input
    // and the comparison wouldn't be fair.
    vector<Stock> copy;
    for (int i = 0; i < n; i++)
        copy.push_back(stocks[i]);

    long long comparisons = 0;

    cout << "Running " << algorithmName(algorithm) << " on " << n << " records..." << endl;

    // only the sort itself is timed, not the copy above or the check below
    chrono::steady_clock::time_point start = chrono::steady_clock::now();

    switch (algorithm) {
        case HEAP_SORT:
            heapSort(copy, field, order, comparisons);
            break;
        case MERGE_SORT:
            mergeSort(copy, field, order, comparisons);
            break;
        default:
            cout << "Unknown algorithm." << endl;
            return false;
    }

    chrono::steady_clock::time_point end = chrono::steady_clock::now();

    result.algorithm = algorithmName(algorithm);
    result.field = field;
    result.order = order;
    result.records = n;
    // measure in microseconds, then divide by 1000 to get ms with decimals
    result.timeMs = chrono::duration_cast<chrono::microseconds>(end - start).count() / 1000.0;
    result.comparisons = comparisons;

    // Memory is an ESTIMATE, not measured. Merge sort needs a temp array as
    // big as the input to merge into, so about n * sizeof(Stock) extra bytes.
    // Heap sort just swaps inside the same vector, so it needs ~0 extra.
    // (sizeof(Stock) counts the string objects but not text stored outside
    // them. Our dates and symbols are short enough to fit inside the string
    // object, so it's pretty close.) Divide by 1024*1024 to get MB.
    if (algorithm == MERGE_SORT)
        result.memoryMB = (double)n * sizeof(Stock) / (1024.0 * 1024.0);
    else
        result.memoryMB = 0.0;

    result.sortedCorrectly = isSorted(copy, field, order);

    cout << result.algorithm << " done: " << result.timeMs << " ms, "
         << result.comparisons << " comparisons" << endl;

    appendToResultsFile(result);

    return true;
}

// Picks the gap between y axis gridlines from a list of round numbers,
// so the axis labels look nice. It takes the first step where 5 gridlines
// are enough to reach the slowest time.
double niceGridStep(double slowest) {
    double steps[] = {1, 2, 5, 10, 20, 50, 100, 200, 250, 500, 1000, 2000, 5000, 10000};
    int numSteps = 14;

    for (int i = 0; i < numSteps; i++) {
        if (steps[i] * 5 >= slowest)
            return steps[i];
    }

    // only happens if a sort takes over 50 seconds
    return 10000;
}

// Blue for merge sort, orange for heap sort
string barColor(const string& algorithm) {
    if (algorithm == "Merge Sort")
        return "#4a90e2";

    return "#f5a623";
}

// Writes chart.html: a dark page with a vertical bar chart of the sort times.
// The chart is plain SVG (just shapes written as text), so it opens in any
// browser with no internet or libraries. I write it straight into the file.
void writeChartHtml(const vector<SortResult>& results) {
    ofstream out(CHART_FILE);

    if (!out.is_open()) {
        cout << "Could not write " << CHART_FILE << endl;
        return;
    }

    // chart size and where the plot area sits inside it, in pixels
    const int WIDTH = 640;
    const int HEIGHT = 400;
    const int LEFT = 80;          // room on the left for the y axis numbers
    const int TOP = 50;           // room on top for the legend
    const int PLOT_WIDTH = 520;
    const int PLOT_HEIGHT = 290;
    const int BAR_PIXELS = 110;   // how wide each bar is
    const int BOTTOM = TOP + PLOT_HEIGHT;  // y of the x axis line

    double slowest = 0.0;
    bool hasMerge = false;

    for (size_t i = 0; i < results.size(); i++) {
        if (results[i].timeMs > slowest)
            slowest = results[i].timeMs;
        if (results[i].algorithm == "Merge Sort")
            hasMerge = true;
    }

    // the top of the y axis is 5 gridlines up, which is always >= slowest
    double step = niceGridStep(slowest);
    double axisMax = step * 5;

    out << fixed;

    out << "<!DOCTYPE html>\n<html>\n<head>\n<meta charset=\"utf-8\">\n"
        << "<title>Performance Comparison</title>\n<style>\n"
        << "body { background: #1e1e2e; color: #e0e0e0; font-family: Arial, sans-serif; margin: 30px; }\n"
        << "h1 { font-size: 26px; margin-bottom: 16px; }\n"
        << ".box { background: #2a2a3c; border-radius: 8px; padding: 20px; display: inline-block; }\n"
        << "table { margin-top: 12px; border-collapse: collapse; }\n"
        << "td { padding: 4px 24px 4px 0; }\n"
        << ".label { color: #9a9ab0; }\n"
        << ".note { color: #9a9ab0; font-size: 13px; margin: 10px 0 0 0; }\n"
        << "</style>\n</head>\n<body>\n"
        << "<h1>Performance Comparison</h1>\n<div class=\"box\">\n";

    out << "<svg width=\"" << WIDTH << "\" height=\"" << HEIGHT << "\" "
        << "xmlns=\"http://www.w3.org/2000/svg\" font-family=\"Arial, sans-serif\">\n";

    // gridlines and y axis numbers, one every "step" ms
    for (int i = 0; i <= 5; i++) {
        // SVG y starts at the TOP of the picture and grows downward,
        // so going up the axis means subtracting from BOTTOM
        int y = BOTTOM - i * PLOT_HEIGHT / 5;

        out << "<line x1=\"" << LEFT << "\" y1=\"" << y << "\" x2=\"" << LEFT + PLOT_WIDTH
            << "\" y2=\"" << y << "\" stroke=\"#44445a\" />\n";
        out << "<text x=\"" << LEFT - 10 << "\" y=\"" << y + 4 << "\" text-anchor=\"end\" "
            << "fill=\"#b0b0c0\" font-size=\"12\">" << setprecision(0) << step * i << "</text>\n";
    }

    // the two axis lines
    out << "<line x1=\"" << LEFT << "\" y1=\"" << TOP << "\" x2=\"" << LEFT << "\" y2=\"" << BOTTOM
        << "\" stroke=\"#c0c0d0\" />\n";
    out << "<line x1=\"" << LEFT << "\" y1=\"" << BOTTOM << "\" x2=\"" << LEFT + PLOT_WIDTH
        << "\" y2=\"" << BOTTOM << "\" stroke=\"#c0c0d0\" />\n";

    // y axis title, rotated to run up the side
    out << "<text x=\"20\" y=\"" << TOP + PLOT_HEIGHT / 2 << "\" fill=\"#e0e0e0\" font-size=\"13\" "
        << "text-anchor=\"middle\" transform=\"rotate(-90 20 " << TOP + PLOT_HEIGHT / 2 << ")\">"
        << "Execution Time (ms)</text>\n";

    // each algorithm gets an equal slice of the plot width, bar centered in it
    int slotWidth = PLOT_WIDTH / (int)results.size();

    for (size_t i = 0; i < results.size(); i++) {
        // Bar height = this time as a fraction of the axis top, times the plot
        // height. Ex: 935 ms on a 0-1000 axis fills 93.5% of the 290 px plot.
        // +0.5 rounds to the nearest pixel.
        int barHeight = (int)(results[i].timeMs / axisMax * PLOT_HEIGHT + 0.5);

        // a super fast sort still gets a sliver so you can see it
        if (barHeight < 1)
            barHeight = 1;

        int x = LEFT + slotWidth * (int)i + (slotWidth - BAR_PIXELS) / 2;
        int y = BOTTOM - barHeight;  // the bar's top edge, since y grows downward
        int middle = x + BAR_PIXELS / 2;

        out << "<rect x=\"" << x << "\" y=\"" << y << "\" width=\"" << BAR_PIXELS
            << "\" height=\"" << barHeight << "\" rx=\"3\" fill=\"" << barColor(results[i].algorithm)
            << "\" />\n";

        // value just above the bar
        out << "<text x=\"" << middle << "\" y=\"" << y - 8 << "\" text-anchor=\"middle\" "
            << "fill=\"#ffffff\" font-size=\"14\" font-weight=\"bold\">"
            << setprecision(2) << results[i].timeMs << " ms</text>\n";

        // algorithm name under the x axis
        out << "<text x=\"" << middle << "\" y=\"" << BOTTOM + 22 << "\" text-anchor=\"middle\" "
            << "fill=\"#e0e0e0\" font-size=\"13\">" << results[i].algorithm << "</text>\n";
    }

    // legend in the top left: a colored square and the name for each bar
    for (size_t i = 0; i < results.size(); i++) {
        int legendX = LEFT + 140 * (int)i;

        out << "<rect x=\"" << legendX << "\" y=\"14\" width=\"14\" height=\"14\" fill=\""
            << barColor(results[i].algorithm) << "\" />\n";
        out << "<text x=\"" << legendX + 20 << "\" y=\"26\" fill=\"#e0e0e0\" font-size=\"13\">"
            << results[i].algorithm << "</text>\n";
    }

    out << "</svg>\n";

    // the run details under the chart
    out << "<table>\n"
        << "<tr><td class=\"label\">Dataset Size</td><td>" << results[0].records << " records</td></tr>\n"
        << "<tr><td class=\"label\">Sort Field</td><td>" << fieldName(results[0].field) << "</td></tr>\n"
        << "<tr><td class=\"label\">Order</td><td>" << orderName(results[0].order) << "</td></tr>\n";

    for (size_t i = 0; i < results.size(); i++) {
        out << "<tr><td class=\"label\">" << results[i].algorithm << " Comparisons</td><td>"
            << results[i].comparisons << "</td></tr>\n";
    }

    out << "</table>\n";

    if (!hasMerge)
        out << "<p class=\"note\">Merge Sort: not run this time.</p>\n";


    out << "<p class=\"note\">Times vary by system (CPU, other programs running, compiler settings).</p>\n"
        << "</div>\n</body>\n</html>\n";
}

// Opens chart.html in the default browser. "start" is a Windows command
// (on a Mac it would be "open" instead).
void openChart() {
    if (!fileExists(CHART_FILE)) {
        cout << "No chart yet. Run a sort first." << endl;
        return;
    }

    cout << "Opening " << CHART_FILE << " in your browser..." << endl;
    system("start chart.html");
}

// Runs merge, heap or both, and saves what finished into results.
// If nothing could run, the old results are kept.
void runAlgorithms(const vector<Stock>& stocks, int n, SortField field, SortOrder order,
                   int algorithm, vector<SortResult>& results) {
    vector<SortResult> newResults;
    SortResult result;

    if (algorithm == MERGE_SORT || algorithm == COMPARE_BOTH) {
        if (runOneSort(stocks, n, field, order, MERGE_SORT, result))
            newResults.push_back(result);
    }

    if (algorithm == HEAP_SORT || algorithm == COMPARE_BOTH) {
        if (runOneSort(stocks, n, field, order, HEAP_SORT, result))
            newResults.push_back(result);
    }

    if (newResults.empty())
        return;

    results = newResults;
    writeChartHtml(results);
    cout << "Saved to " << RESULTS_FILE << " and " << CHART_FILE
         << ". Pick 4 (View Results) to see the table and chart." << endl;
}

// Option 2: pick the field, order, algorithm and how many records, then sort
void sortScreen(const vector<Stock>& stocks, SortField& field, SortOrder& order,
                int& recordLimit, vector<SortResult>& results) {
    if (stocks.empty()) {
        cout << "No data loaded yet. Load the CSV first (option 1)." << endl;
        return;
    }

    cout << "\nSort by:" << endl;
    cout << "1. Closing Price" << endl;
    cout << "2. Trading Volume" << endl;
    cout << "3. Date" << endl;
    int fieldChoice = readChoice(1, 3);
    if (fieldChoice == -1)
        return;

    cout << "\nOrder:" << endl;
    cout << "1. Ascending" << endl;
    cout << "2. Descending" << endl;
    int orderChoice = readChoice(1, 2);
    if (orderChoice == -1)
        return;

    cout << "\nAlgorithm:" << endl;
    cout << "1. Merge Sort" << endl;
    cout << "2. Heap Sort" << endl;
    cout << "3. Compare Both" << endl;
    int algorithm = readChoice(1, 3);
    if (algorithm == -1)
        return;

    int total = (int)stocks.size();
    cout << "\nHow many records to sort? (Enter = all " << total << ")" << endl;
    int n = readNumberOrDefault(1, total, total);
    if (n == -1)
        return;

    switch (fieldChoice) {
        case 1:
            field = CLOSE;
            break;
        case 2:
            field = VOLUME;
            break;
        default:
            field = DATE;
            break;
    }

    if (orderChoice == 1)
        order = ASCENDING;
    else
        order = DESCENDING;

    // 0 means "all", so a reload with a different size still sorts everything
    if (n == total)
        recordLimit = 0;
    else
        recordLimit = n;

    cout << endl;
    runAlgorithms(stocks, n, field, order, algorithm, results);
}

// Option 3: runs both algorithms with the current settings from option 2
// (or the defaults: closing price, ascending, all records)
void compareScreen(const vector<Stock>& stocks, SortField field, SortOrder order,
                   int recordLimit, vector<SortResult>& results) {
    if (stocks.empty()) {
        cout << "No data loaded yet. Load the CSV first (option 1)." << endl;
        return;
    }

    int n = recordsToUse(stocks, recordLimit);

    cout << "\nComparing both algorithms: " << fieldName(field) << ", "
         << orderName(order) << ", " << n << " records" << endl;

    runAlgorithms(stocks, n, field, order, COMPARE_BOTH, results);
}

// Prints the results table: dataset size, sort settings, then one row
// per algorithm with time, comparisons and estimated memory
void printResultsTable(const vector<SortResult>& results) {
    cout << "\n---------------- Results ----------------" << endl;
    cout << "Dataset Size: " << results[0].records << " records" << endl;
    cout << "Sort By: " << fieldName(results[0].field) << " ("
         << orderName(results[0].order) << ")" << endl << endl;

    cout << left << setw(12) << "Algorithm"
         << right << setw(12) << "Time (ms)" << setw(15) << "Comparisons"
         << setw(20) << "Memory (MB, est.)" << endl;
    cout << string(59, '-') << endl;

    for (size_t i = 0; i < results.size(); i++) {
        cout << left << setw(12) << results[i].algorithm
             << right << setw(12) << results[i].timeMs
             << setw(15) << results[i].comparisons
             << setw(20) << results[i].memoryMB << endl;
    }

    cout << left;
}

// Prints whether each algorithm's output was really in order
void printCorrectnessCheck(const vector<SortResult>& results) {
    cout << "\nCorrectness check:" << endl;

    bool allCorrect = true;

    for (size_t i = 0; i < results.size(); i++) {
        if (results[i].sortedCorrectly) {
            cout << "  " << results[i].algorithm << ": sorted correctly" << endl;
        } else {
            cout << "  " << results[i].algorithm << ": NOT sorted correctly!" << endl;
            allCorrect = false;
        }
    }

    if (allCorrect && results.size() == 2)
        cout << "Both algorithms sorted the data correctly" << endl;
    else if (allCorrect)
        cout << results[0].algorithm << " sorted the data correctly" << endl;
    else
        cout << "Something went wrong, check the sort code." << endl;
}

// Draws a sideways bar chart of the times with #.
// Every bar is scaled against the slowest time, so the slowest one gets
// BAR_WIDTH #'s and a sort that took half as long gets half as many.
void drawBarChart(const vector<SortResult>& results) {
    double slowest = 0.0;

    for (size_t i = 0; i < results.size(); i++) {
        if (results[i].timeMs > slowest)
            slowest = results[i].timeMs;
    }

    cout << "\nTime chart (longer bar = slower):" << endl;

    for (size_t i = 0; i < results.size(); i++) {
        int length = 0;

        // +0.5 rounds to the nearest whole # instead of always rounding down
        if (slowest > 0)
            length = (int)(results[i].timeMs / slowest * BAR_WIDTH + 0.5);

        // a really fast sort still gets 1 # so it doesn't look like it's missing
        if (length < 1)
            length = 1;

        cout << left << setw(12) << results[i].algorithm << "| ";

        for (int j = 0; j < length; j++)
            cout << '#';

        cout << " " << results[i].timeMs << " ms" << endl;
    }
}

// Option 4: shows the table, the correctness check and the # bar chart
// for the last run, then offers to open the real chart in the browser
void viewResultsScreen(const vector<SortResult>& results) {
    if (results.empty()) {
        cout << "No results yet. Run a sort first (option 2 or 3)." << endl;
        return;
    }

    printResultsTable(results);
    printCorrectnessCheck(results);
    drawBarChart(results);
    cout << "\n(Every run is also saved in " << RESULTS_FILE << ")" << endl;

    cout << "\n1. Open chart in browser" << endl;
    cout << "2. Back to main menu" << endl;

    int choice = readChoice(1, 2);

    if (choice == 1)
        openChart();
}

int main() {
    vector<Stock> stocks;
    vector<SortResult> results;

    // defaults, so option 3 works without going through option 2 first
    SortField field = CLOSE;
    SortOrder order = ASCENDING;
    int recordLimit = 0;  // 0 = all records

    // all decimals on screen show 2 places (prices, ms, MB)
    cout << fixed << setprecision(2);

    bool running = true;

    while (running) {
        showMainMenu(stocks, field, order, recordLimit);

        int choice = readChoice(1, 5);

        switch (choice) {
            case 1:
                if (loadDataScreen(stocks)) {
                    // old results belong to the old data, so start fresh
                    results.clear();
                    recordLimit = 0;
                }
                break;
            case 2:
                sortScreen(stocks, field, order, recordLimit, results);
                break;
            case 3:
                compareScreen(stocks, field, order, recordLimit, results);
                break;
            case 4:
                viewResultsScreen(results);
                break;
            case 5:
            case -1:  // input was closed, treat it like exit
                running = false;
                break;
        }
    }

    cout << "Goodbye!" << endl;

    return 0;
}
