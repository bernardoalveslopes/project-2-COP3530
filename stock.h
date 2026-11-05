#ifndef STOCK_H
#define STOCK_H

#include <string>

// Shared by heapsort.h, mergesort.h, and dataloader.h so every part of the
// program uses the same record type and the same ordering rule.

struct Stock {
  std::string date;
  double open;
  double high;
  double low;
  double close;
  long long volume;
  std::string Name;
};

enum SortField{
  CLOSE,
  VOLUME,
  DATE
};

enum SortOrder {
  ASCENDING,
  DESCENDING,
};

inline bool comesBefore(const Stock&a, const Stock&b, SortField field, SortOrder order) {
  if (field == CLOSE) {
        if (order == ASCENDING)
            return a.close < b.close;

        return a.close > b.close;
    }

    if (field == VOLUME) {
        if (order == ASCENDING)
            return a.volume < b.volume;

        return a.volume > b.volume;
    }

    if (order == ASCENDING)
        return a.date < b.date;

    return a.date > b.date;
}

#endif
