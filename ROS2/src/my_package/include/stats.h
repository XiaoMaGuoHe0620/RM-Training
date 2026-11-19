// 预处理指令 if not defined 如果STATS_H没被定义，就继续往下编译，否则跳过整个文件
// 目的是头文件保护，防止同一头文件被多次 #include 造成重复定义编译报错
#ifndef STATS_H 
#define STATS_H

// 引入标准库头文件<vector>，因为函数参数用到了 std::vector<double> （double）类型动态数组
#include <vector> 
// 引入 std::optional<T>  std::optional<double>表示可能有一个double值，也可能没有值（空）
#include <optional> 
// 引入<csddef>，提供 std::size_t 类型， std::size_t 是无符号整数类型，专门用来表示数组下标，容器大小，计数
#include <cstddef> 

// 1. 求一组长度的均值，空数组返回 std::nullopt
// 函数返回值：std::optional<double> 如果向量非空：里面存平均值double 如果向量是空：返回std::nullopt（代表不存在有效值）
// std::vector<double>：存放长度的数组  const：函数内部不会修改这个向量  &：引用传递，避免拷贝整个vector,提升效率
std::optional<double> meanLength(const std::vector<double>& lengths);

// 2. 统计一组长度中不小于给定阈值的个数
// 返回类型：std::size_t 返回满足条件的元素个数（无符号整数，符合容器计数习惯）
// const std::vector<double>& lengths：待统计的长度数组  double threshold：阈值
std::size_t countNotLessThan(const std::vector<double>& lengths, double threshold);

// 和最开头 #ifndef STATS_H 配对，结束头文件保护块
// STATS_H 
#endif