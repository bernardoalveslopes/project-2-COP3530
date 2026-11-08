#ifndef MERGESORT_H
#define MERGESORT_H

#include <vector>
#include "stock.h"

// Merge Sort, written by hand (no std::sort, std::stable_sort, or std::merge).
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

// Merges the two sorted halves stocks[left..mid] and stocks[mid+1..right]
// into one sorted run, using temp as scratch space.
inline void mergeHalves(std::vector<Stock>& stocks, std::vector<Stock>& temp,
                        int left, int mid, int right,
                        SortField field, SortOrder order,
                        long long& comparisons) {

    int i = left;      // next record in the left half
    int j = mid + 1;   // next record in the right half
    int k = left;      // next free spot in temp

    while (i <= mid && j <= right) {
        comparisons++;

        // Take from the right half only when it strictly comes first.
        // On a tie the left record goes first, which keeps the sort stable.
        if (comesBefore(stocks[j], stocks[i], field, order)) {
            temp[k] = stocks[j];
            j++;
        } else {
            temp[k] = stocks[i];
            i++;
        }

        k++;
    }

    // One half is used up, so copy whatever is left in the other half
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

    // Copy the merged run back into stocks
    for (int m = left; m <= right; m++)
        stocks[m] = temp[m];
}

// Sorts stocks[left..right]: split it in half, sort each half,
// then merge the two sorted halves.
inline void mergeSortRange(std::vector<Stock>& stocks, std::vector<Stock>& temp,
                           int left, int right,
                           SortField field, SortOrder order,
                           long long& comparisons) {

    // 0 or 1 records is already sorted
    if (left >= right)
        return;

    // written this way instead of (left + right) / 2 so it can't overflow
    int mid = left + (right - left) / 2;

    mergeSortRange(stocks, temp, left, mid, field, order, comparisons);
    mergeSortRange(stocks, temp, mid + 1, right, field, order, comparisons);
    mergeHalves(stocks, temp, left, mid, right, field, order, comparisons);
}

// Same arguments as heapSort, so the menu can call either one.
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
