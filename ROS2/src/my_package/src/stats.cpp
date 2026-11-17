#include "stats.h"

// 实现求均值函数
std::optional<double> meanLength(const std::vector<double>& lengths) {
    if (lengths.empty()) {
        return std::nullopt; // 空数组返回 nullopt
    }
    
    double sum = 0.0;
    for (double len : lengths) {
        sum += len;
    }
    return sum / lengths.size();
}

// 实现统计不小于阈值的个数
std::size_t countNotLessThan(const std::vector<double>& lengths, double threshold) {
    std::size_t count = 0;
    for (double len : lengths) {
        if (len >= threshold) {
            ++count;
        }
    }
    return count;
}
