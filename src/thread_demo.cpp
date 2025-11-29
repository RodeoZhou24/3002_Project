/**
 * @file thread_demo.cpp
 * @brief ThreadManager 演示程序
 * @author Auto-generated
 * @date 2025-01-XX
 */

#include "ThreadManager.h"
#include "PricingStrategy.h"
#include <iostream>
#include <vector>
#include <string>

using namespace pricing;

int main() {
    std::cout << "═══════════════════════════════════════════════════════" << std::endl;
    std::cout << "  多线程定价系统演示程序" << std::endl;
    std::cout << "  ThreadManager Demo" << std::endl;
    std::cout << "═══════════════════════════════════════════════════════" << std::endl;
    std::cout << std::endl;
    
    // 创建输出目录
    system("mkdir -p output");
    
    // 创建 ThreadManager 实例
    ThreadManager manager("output/pricing.log");
    
    // 创建定价策略
    PricingStrategy strategy;
    
    // 定义商家和产品
    std::vector<Merchant> merchants = {
        Merchant("Apple官方店", {"iPhone-15-Pro", "iPhone-15-Pro-Max", "MacBook-Pro-14"}, 1),
        Merchant("京东自营", {"iPhone-15-Pro", "MacBook-Pro-14", "RTX-4090"}, 2),
        Merchant("天猫旗舰", {"iPhone-15-Pro-Max", "RTX-4090", "MacBook-Pro-16"}, 2),
        Merchant("苏宁易购", {"MacBook-Pro-14", "MacBook-Pro-16", "RTX-4080"}, 3),
        Merchant("拼多多", {"iPhone-15-Pro", "RTX-4080", "RTX-4090"}, 4)
    };
    
    std::cout << "📋 商家和产品列表：" << std::endl;
    for (const auto& merchant : merchants) {
        std::cout << "  • " << merchant.name << " (优先级: " << merchant.priority << ")" << std::endl;
        for (const auto& product : merchant.products) {
            std::cout << "    - " << product << std::endl;
        }
    }
    std::cout << std::endl;
    
    // 启动多线程定价
    manager.startPricing(merchants, strategy);
    
    // 等待所有线程完成
    std::cout << "\n⏳ 等待所有定价任务完成..." << std::endl;
    manager.waitAll();
    
    // 打印统计信息
    manager.printStatistics();
    
    // 导出价格趋势
    manager.exportPriceTrend("output/price_trend.csv");
    
    std::cout << "\n✨ 演示完成！" << std::endl;
    std::cout << "  日志文件: output/pricing.log" << std::endl;
    std::cout << "  价格趋势: output/price_trend.csv" << std::endl;
    std::cout << std::endl;
    
    return 0;
}

