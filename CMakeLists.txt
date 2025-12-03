cmake_minimum_required(VERSION 3.16)
project(DynamicPricing LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 包含头文件目录
include_directories(include)

# 显式列出需要的源文件 (剔除 thread_demo.cpp 避免冲突)
set(SOURCES
        src/main.cpp
        src/DataLoader.cpp
        src/Forecaster.cpp
        src/InventoryAlert.cpp
        src/PricingStrategy.cpp
        src/ThreadManager.cpp
        src/Visualizer.cpp
)

# 创建可执行文件
add_executable(main ${SOURCES})

# 链接线程库 (Linux/Mac 需要)
if(UNIX)
    target_link_libraries(main pthread)
endif()

# 设置调试器的工作目录为项目根目录
set_target_properties(main PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        XCODE_SCHEME_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)