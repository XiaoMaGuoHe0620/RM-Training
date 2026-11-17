#include <iostream>
#include <vector>
#include "stats.h"

int main() {
    // ================= 测试用例 1：空数组 =================
    std::vector<double> empty_lengths;
    
    // 测试 meanLength
    auto mean_res = meanLength(empty_lengths);
    if (mean_res.has_value()) {
        std::cerr << "[Error] 空数组的均值应返回 std::nullopt" << std::endl;
        return 1; // 结果不对返回非零值
    }
    
    // 测试 countNotLessThan
    if (countNotLessThan(empty_lengths, 5.0) != 0) {
        std::cerr << "[Error] 空数组的统计个数应为 0" << std::endl;
        return 1;
    }

    // ================= 测试用例 2：正常数组，且包含恰好等于阈值的情况 =================
    std::vector<double> lengths = {10.0, 20.0, 30.0, 40.0};
    
    // 测试 meanLength (10+20+30+40)/4 = 25.0
    auto mean_res2 = meanLength(lengths);
    if (!mean_res2.has_value() || *mean_res2 != 25.0) {
        std::cerr << "[Error] 数组的均值计算错误" << std::endl;
        return 1;
    }

    // 测试 countNotLessThan，阈值设为 30.0 (数组中有 30.0 和 40.0 满足)
    double threshold = 30.0; 
    std::size_t count = countNotLessThan(lengths, threshold);
    if (count != 2) {
        std::cerr << "[Error] 不小于 " << threshold << " 的个数计算错误，期望 2，实际 " << count << std::endl;
        return 1;
    }

    // ================= 测试用例 3：单一元素恰好等于阈值 =================
    std::vector<double> single_length = {15.0};
    if (countNotLessThan(single_length, 15.0) != 1) {
        std::cerr << "[Error] 恰好等于阈值的测试失败" << std::endl;
        return 1;
    }

    std::cout << "所有测试用例通过！(All tests passed!)" << std::endl;
    return 0; // 成功返回 0
}
