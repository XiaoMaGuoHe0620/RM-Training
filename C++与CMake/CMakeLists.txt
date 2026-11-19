# 声明这个项目要求CMake的版本至少是3.10，否则直接报错，不能继续构建
cmake_minimum_required(VERSION 3.10)
# 定义项目名字：StatsProject
project(StatsProject)

# 设置 C++ 标准为 C++17 (必须，因为使用了 std::optional)
set(CMAKE_CXX_STANDARD 17)
# 强制要求必须使用上面指定的C++版本，如果编译器不支持C++17，CMake直接报错
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 包含头文件目录
# 告诉编译器，去include/文件夹里面找.h头文件
include_directories(include)

# 1. 将 src 下的源文件编译成一个库 (STATIC 表示静态库)
# add_library:生成库文件 stats_lib:库的名字（自己命名，后面链接要用）
# STATIC:静态库（.a在Linux/.lib在Windows）
# src/stats.cpp:用来编译这个库的源码，就是我们写的stats实现代码
add_library(stats_lib STATIC src/stats.cpp)

# 2. 编译带 main 的测试程序
# add_executable：生成可执行程序
# main_text：最终生成的程序名（Linux下./main_test,Windows main_test.exe)
# main.cpp：包含int main()的测试入口文件
add_executable(main_test main.cpp)

# 3. 将测试程序链接到刚才生成的库
# 把可执行文件main_test和静态库stats_lib链接在一起
# PRIVATE：链接关系是私有的，只给main_test使用，别的目标不会继承这个链接
# 只有链接上库，main.cpp里调用mealength,coutNotLessThan才不会报“未定义引用”
target_link_libraries(main_test PRIVATE stats_lib)
