#ifndef MERGESORT_H
#define MERGESORT_H

#include <vector>
#include "stock.h"

//
//
// References:
//   Cormen, Leiserson, Rivest, and Stein, "Introduction to Algorithms",
//   3rd ed., MIT Press, 2009, Section 2.3.1 (the MERGE and MERGE-SORT
//   procedures this code follows).
//   Sedgewick and Wayne, "Algorithms", 4th ed., Section 2.2 "Mergesort":
//   https://algs4.cs.princeton.edu/22mergesort/
//   (the idea of making one helper array up front and reusing it in every
//   merge, instead of making new arrays at each step).
//
// Time: O(n log n) in every case.
// Extra memory: one helper array of n records, plus O(log n) for recursion.
// Stable: records with equal keys keep their original order.

inline void mergeHalves(std::vector<Stock>& stocks, std::vector<Stock>& temp,
                        int left, int mid, int right,
                        SortField field, SortOrder order,
                        long long& comparisons) {

    int i = left;     
    int j = mid + 1;  
    int k = left;    

    while (i <= mid && j <= right) {
        comparisons++;


        if (comesBefore(stocks[j], stocks[i], field, order)) {
            temp[k] = stocks[j];
            j++;
        } else {
            temp[k] = stocks[i];
            i++;
        }

        k++;
    }


    while (i <= mid) {
        temp[k] = stocks[i];
        i++;
        k++;
    }

    while (j <= right) {
        temp[k] = stocks[j];
        j++;
        k++;
    }


    for (int m = left; m <= right; m++)
        stocks[m] = temp[m];
}


inline void mergeSortRange(std::vector<Stock>& stocks, std::vector<Stock>& temp,
                           int left, int right,
                           SortField field, SortOrder order,
                           long long& comparisons) {

    // 0 or 1 records is already sorted
    if (left >= right)
        return;


    int mid = left + (right - left) / 2;

    mergeSortRange(stocks, temp, left, mid, field, order, comparisons);
    mergeSortRange(stocks, temp, mid + 1, right, field, order, comparisons);
    mergeHalves(stocks, temp, left, mid, right, field, order, comparisons);
}

inline void mergeSort(std::vector<Stock>& stocks, SortField field,
                      SortOrder order, long long& comparisons) {

    int size = static_cast<int>(stocks.size());

    comparisons = 0;

    if (size < 2)
        return;

    // The helper array. This is the O(n) extra memory merge sort needs.
    std::vector<Stock> temp(size);

    mergeSortRange(stocks, temp, 0, size - 1, field, order, comparisons);
}

#endif
