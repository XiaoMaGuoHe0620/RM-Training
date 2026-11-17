#ifndef STATS_H
#define STATS_H

#include <vector>
#include <optional>
#include <cstddef>

// 1. 求一组长度的均值，空数组返回 std::nullopt
std::optional<double> meanLength(const std::vector<double>& lengths);

// 2. 统计一组长度中不小于给定阈值的个数
std::size_t countNotLessThan(const std::vector<double>& lengths, double threshold);

#endif // STATS_H
