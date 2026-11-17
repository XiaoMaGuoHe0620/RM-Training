cmake_minimum_required(VERSION 3.10)
project(StatsProject)

# 设置 C++ 标准为 C++17 (必须，因为使用了 std::optional)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 包含头文件目录
include_directories(include)

# 1. 将 src 下的源文件编译成一个库 (STATIC 表示静态库)
add_library(stats_lib STATIC src/stats.cpp)

# 2. 编译带 main 的测试程序
add_executable(main_test main.cpp)

# 3. 将测试程序链接到刚才生成的库
target_link_libraries(main_test PRIVATE stats_lib)
