#ifndef HEAPSORT_H
#define HEAPSORT_H

#include <vector>
#include <string>
#include <algorithm>
#include "stock.h"

inline void heapify(std::vector<Stock>& stocks, int size, int root,
                    SortField field, SortOrder order,
                    long long& comparisons) {

    int largest = root;
    int left = 2 * root + 1;
    int right = 2 * root + 2;

    if (left < size) {
        comparisons++;

        if (comesBefore(stocks[largest], stocks[left], field, order))
            largest = left;
    }

    if (right < size) {
        comparisons++;

        if (comesBefore(stocks[largest], stocks[right], field, order))
            largest = right;
    }

    if (largest != root) {
        std::swap(stocks[root], stocks[largest]);

        heapify(stocks, size, largest, field, order, comparisons);
    }
}

inline void heapSort(std::vector<Stock>& stocks, SortField field,
                     SortOrder order, long long& comparisons) {

    int size = static_cast<int>(stocks.size());

    comparisons = 0;

    for (int i = size / 2 - 1; i >= 0; i--) {
        heapify(stocks, size, i, field, order, comparisons);
    }

    for (int i = size - 1; i > 0; i--) {
        std::swap(stocks[0], stocks[i]);

        heapify(stocks, i, 0, field, order, comparisons);
    }
}

#endif
