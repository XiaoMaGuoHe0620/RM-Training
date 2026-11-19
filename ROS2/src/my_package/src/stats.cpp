#include "stats.h"

// 实现求均值函数
std::optional<double> meanLength(const std::vector<double>& lengths) {
    // empty() 是vector的成员函数，判断这个向量是不是空（里面没有元素）
    if (lengths.empty()) {
        return std::nullopt; // 空数组返回 nullopt
    }
    
    double sum = 0.0;
    // for循环，依次把lengths里面每一个元素拿出来，放到变量len里，等价于遍历数组每一个长度
    for (double len : lengths) {
        sum += len;
    }
    // lengths.size() 返回vector里面元素个数，类型是size_t
    // 总和除以数量，算出平均值，自动包装成 std::optional<double> 返回
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
