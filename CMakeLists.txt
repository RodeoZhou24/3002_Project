cmake_minimum_required(VERSION 3.16)
project(DynamicPricing LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 设置包含目录
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# PricingStrategy library
add_library(pricing_strategy STATIC
    ${SOURCE_DIR}/PricingStrategy.cpp
)
target_include_directories(pricing_strategy 
    PUBLIC ${INCLUDE_DIR}
)

# ThreadManager library
add_library(thread_manager STATIC
    ${SOURCE_DIR}/ThreadManager.cpp
)
target_include_directories(thread_manager 
    PUBLIC ${INCLUDE_DIR}
)
target_link_libraries(thread_manager 
    PUBLIC pricing_strategy
)

# DataLoader library (if needed as a separate library)
add_library(data_loader STATIC
    ${SOURCE_DIR}/DataLoader.cpp
)
target_include_directories(data_loader 
    PUBLIC ${INCLUDE_DIR}
)

# Forecaster library (if needed as a separate library)
add_library(forecaster STATIC
    ${SOURCE_DIR}/Forecaster.cpp
)
target_include_directories(forecaster 
    PUBLIC ${INCLUDE_DIR}
)

# InventoryAlert library (if needed as a separate library)
add_library(inventory_alert STATIC
    ${SOURCE_DIR}/InventoryAlert.cpp
)
target_include_directories(inventory_alert 
    PUBLIC ${INCLUDE_DIR}
)

# Pricing strategy demo (original)
# Create a separate source file for the demo that includes PricingStrategy.cpp with TEST_PRICING defined
add_executable(pricing_demo ${SOURCE_DIR}/PricingStrategy.cpp)
target_compile_definitions(pricing_demo PRIVATE TEST_PRICING)
target_include_directories(pricing_demo PRIVATE ${INCLUDE_DIR})
# Note: pricing_demo directly compiles PricingStrategy.cpp, so we don't link the library
# to avoid duplicate symbols

# ThreadManager demo
add_executable(thread_demo ${SOURCE_DIR}/thread_demo.cpp)
target_include_directories(thread_demo PRIVATE ${INCLUDE_DIR})
target_link_libraries(thread_demo 
    PRIVATE pricing_strategy 
    thread_manager
)

# Main demo program - 整合所有模块的完整演示程序
add_executable(main ${SOURCE_DIR}/main.cpp)
target_include_directories(main PRIVATE ${INCLUDE_DIR})
target_link_libraries(main
    PRIVATE pricing_strategy
    thread_manager
    data_loader
    forecaster
    inventory_alert
    pthread
)

# 设置可执行文件的工作目录为项目根目录（便于 CLion 等 IDE 运行）
# 这样程序可以从项目根目录找到 sales_history.txt 等数据文件
set_target_properties(main PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    XCODE_SCHEME_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    VS_DEBUGGER_ENVIRONMENT "PATH=${CMAKE_CURRENT_SOURCE_DIR};$ENV{PATH}"
)
