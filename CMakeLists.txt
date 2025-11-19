cmake_minimum_required(VERSION 3.16)
project(DynamicPricing LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(pricing_strategy STATIC
    PricingStrategy.cpp
    PricingStrategy.h
)
target_include_directories(pricing_strategy PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(pricing_demo PricingStrategy.cpp)
target_compile_definitions(pricing_demo PRIVATE TEST_PRICING)
target_link_libraries(pricing_demo PRIVATE pricing_strategy)



