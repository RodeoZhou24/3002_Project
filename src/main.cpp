/**
 * @file main.cpp
 * @brief 动态定价系统完整演示程序
 * @brief 整合 DataLoader, Forecaster, InventoryAlert, PricingStrategy, ThreadManager 模块
 */

#include "DataLoader.h"
#include "Forecaster.h"
#include "InventoryAlert.h"
#include "PricingStrategy.h"
#include "ThreadManager.h"
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <random>

using namespace std;
using namespace pricing;

// 辅助函数：根据产品ID提取历史数据
struct ProductHistory {
    vector<string> dates;
    vector<double> salesHistory;
    vector<double> priceHistory;
    vector<int> stockHistory;
    double latestPrice;
    int latestStock;
    string productId;
};

ProductHistory extractProductHistory(const vector<Sale>& allSales, const string& productId) {
    ProductHistory history;
    history.productId = productId;
    
    for (const auto& sale : allSales) {
        if (sale.productId == productId) {
            history.dates.push_back(sale.date);
            history.salesHistory.push_back(static_cast<double>(sale.sales));
            history.priceHistory.push_back(sale.price);
            history.stockHistory.push_back(sale.stock);
        }
    }
    
    if (!history.dates.empty()) {
        history.latestPrice = history.priceHistory.back();
        history.latestStock = history.stockHistory.back();
    }
    
    return history;
}

// 辅助函数：将产品ID映射到类别
InventoryAlert::ProductCategory getProductCategory(const string& productId) {
    // 简单映射逻辑，可根据实际情况调整
    if (productId.find("P1") != string::npos) {
        return InventoryAlert::ProductCategory::SMARTPHONE;
    } else if (productId.find("P2") != string::npos) {
        return InventoryAlert::ProductCategory::LAPTOP;
    } else {
        return InventoryAlert::ProductCategory::GENERAL;
    }
}

// 辅助函数：将告警级别转换为字符串
string alertLevelToString(InventoryAlert::AlertLevel level) {
    switch(level) {
        case InventoryAlert::AlertLevel::GREEN:
            return "GREEN";
        case InventoryAlert::AlertLevel::MEDIUM:
            return "MEDIUM";
        case InventoryAlert::AlertLevel::HIGH:
            return "HIGH";
        case InventoryAlert::AlertLevel::CRITICAL:
            return "CRITICAL";
        default:
            return "UNKNOWN";
    }
}

// 辅助函数：查找数据文件（智能查找，支持在构建目录或项目根目录运行）
string findDataFile(const string& filename) {
    // 方法1：尝试当前目录
    ifstream test1(filename);
    if (test1.good()) {
        test1.close();
        return filename;
    }
    
    // 方法2：尝试上一级目录（如果从 cmake-build-debug 运行）
    string parentPath = "../" + filename;
    ifstream test2(parentPath);
    if (test2.good()) {
        test2.close();
        return parentPath;
    }
    
    // 方法3：尝试上两级目录
    string grandParentPath = "../../" + filename;
    ifstream test3(grandParentPath);
    if (test3.good()) {
        test3.close();
        return grandParentPath;
    }
    
    // 如果都找不到，返回原始文件名（让 DataLoader 报错）
    return filename;
}

int main() {
    cout << "═══════════════════════════════════════════════════════" << endl;
    cout << "  动态定价系统完整演示程序" << endl;
    cout << "  Dynamic Pricing System Demo" << endl;
    cout << "═══════════════════════════════════════════════════════" << endl;
    cout << endl;
    
    // ========================================================================
    // 1. 数据加载
    // ========================================================================
    cout << "【步骤 1】数据加载" << endl;
    cout << string(60, '-') << endl;
    
    // 智能查找数据文件（支持在构建目录或项目根目录运行）
    string dataFile = findDataFile("sales_history.txt");
    DataLoader loader(dataFile);
    if (!loader.loadData()) {
        cerr << "❌ 错误：无法加载销售数据文件: " << dataFile << endl;
        cerr << "   提示：请确保 sales_history.txt 文件在项目根目录或当前目录" << endl;
        return 1;
    }
    
    const vector<Sale>& allSales = loader.getSalesData();
    if (allSales.empty()) {
        cerr << "❌ 错误：销售数据为空" << endl;
        return 1;
    }
    
    cout << "✅ 成功加载 " << allSales.size() << " 条销售记录" << endl;
    loader.displayData();
    cout << endl;
    
    // ========================================================================
    // 2. 选取示例产品并提取历史数据
    // ========================================================================
    cout << "【步骤 2】产品数据提取" << endl;
    cout << string(60, '-') << endl;
    
    vector<string> exampleProducts = {"P1001", "P1002"};
    map<string, ProductHistory> productHistories;
    
    for (const auto& productId : exampleProducts) {
        ProductHistory history = extractProductHistory(allSales, productId);
        
        if (history.dates.empty()) {
            cout << "⚠️  警告：产品 " << productId << " 没有历史数据，跳过" << endl;
            continue;
        }
        
        productHistories[productId] = history;
        cout << "✅ 产品 " << productId << ":" << endl;
        cout << "   - 记录数: " << history.dates.size() << endl;
        cout << "   - 最新价格: ¥" << fixed << setprecision(2) << history.latestPrice << endl;
        cout << "   - 当前库存: " << history.latestStock << endl;
    }
    
    if (productHistories.empty()) {
        cerr << "❌ 错误：没有可用的产品数据" << endl;
        return 1;
    }
    
    cout << endl;
    
    // ========================================================================
    // 3. 销量预测（Forecaster）
    // ========================================================================
    cout << "【步骤 3】销量预测" << endl;
    cout << string(60, '-') << endl;
    
    map<string, double> nextPredictions;  // 存储每个产品的下一天预测值
    
    for (auto& [productId, history] : productHistories) {
        cout << "\n--- 产品 " << productId << " ---" << endl;
        
        // 使用移动平均法（窗口=3）进行预测
        vector<double> forecast = Forecaster::movingAverage(history.salesHistory, 3);
        
        if (!forecast.empty()) {
            // 显示预测结果
            Forecaster::displayForecast(history.salesHistory, forecast, history.dates);
            
            // 预测下一天的需求
            double nextPrediction = Forecaster::predictNext(history.salesHistory, 3);
            nextPredictions[productId] = nextPrediction;
            cout << "📊 下一天预测销量: " << fixed << setprecision(2) << nextPrediction << " 单位" << endl;
        } else {
            cout << "⚠️  警告：数据不足，无法进行预测" << endl;
            nextPredictions[productId] = 0.0;
        }
    }
    
    cout << endl;
    
    // ========================================================================
    // 4. 库存预警（InventoryAlert）
    // ========================================================================
    cout << "【步骤 4】库存预警" << endl;
    cout << string(60, '-') << endl;
    
    InventoryAlert alertSystem;
    map<string, InventoryAlert::AlertLevel> alertLevels;
    
    for (const auto& [productId, history] : productHistories) {
        double forecast = nextPredictions[productId];
        int currentStock = history.latestStock;
        InventoryAlert::ProductCategory category = getProductCategory(productId);
        
        if (forecast <= 0 || currentStock <= 0) {
            cout << "⚠️  产品 " << productId << " 数据无效，跳过库存检查" << endl;
            continue;
        }
        
        // 触发库存检查并记录告警
        string productName = "Product " + productId;
        bool hasAlert = alertSystem.checkAlert(productId, productName, forecast, currentStock, category);
        
        // 获取告警级别
        InventoryAlert::AlertLevel level = alertSystem.getAlertLevel(forecast, currentStock);
        alertLevels[productId] = level;
        
        cout << "📦 产品 " << productId << ":" << endl;
        cout << "   - 预测需求: " << fixed << setprecision(2) << forecast << " 单位" << endl;
        cout << "   - 当前库存: " << currentStock << " 单位" << endl;
        cout << "   - 告警级别: " << alertLevelToString(level) << endl;
        
        if (hasAlert) {
            cout << "   ⚠️  触发库存告警！" << endl;
        } else {
            cout << "   ✅ 库存充足" << endl;
        }
    }
    
    // 显示告警汇总
    cout << "\n--- 告警汇总 ---" << endl;
    alertSystem.displayAlertSummary();
    alertSystem.displayRecentAlerts(5);
    cout << endl;
    
    // ========================================================================
    // 5. 单商品动态定价（PricingStrategy）
    // ========================================================================
    cout << "【步骤 5】单商品动态定价" << endl;
    cout << string(60, '-') << endl;
    
    PricingStrategy strategy;
    map<string, PricingResult> pricingResults;
    
    for (const auto& [productId, history] : productHistories) {
        cout << "\n--- 产品 " << productId << " ---" << endl;
        
        // 构造 Product 结构
        Product product;
        product.id = productId;
        product.name = "Product " + productId;
        product.basePrice = history.latestPrice;
        product.stock = history.latestStock;
        product.category = "electronics";
        product.isNewModel = false;
        product.series = "Series-" + productId.substr(0, 2);
        
        // 构造 MarketContext
        MarketContext context;
        context.competitorPrice = history.latestPrice * 0.95;  // 竞争对手降价 5%
        context.demandForecast = nextPredictions[productId];
        context.isPeakSeason = false;  // 可根据实际日期判断
        context.viewCount = static_cast<int>(history.salesHistory.back() * 10);  // 假设浏览量是销量的10倍
        context.cartCount = static_cast<int>(history.salesHistory.back() * 2);   // 假设加购数是销量的2倍
        context.purchaseCount = static_cast<int>(history.salesHistory.back());
        
        // 设置当前时间
        time_t now = time(nullptr);
        context.currentTime = *localtime(&now);
        context.newerModelInSeriesAvailable = false;
        
        // 调用定价策略
        PricingResult result = strategy.calculatePrice(product, context);
        pricingResults[productId] = result;
        
        // 显示定价结果
        cout << "💰 定价结果:" << endl;
        cout << "   - 原价: ¥" << fixed << setprecision(2) << product.basePrice << endl;
        cout << "   - 新价格: ¥" << fixed << setprecision(2) << result.newPrice << endl;
        cout << "   - 价格调整: " << fixed << setprecision(2) 
             << (result.adjustment * 100) << "%" << endl;
        cout << "   - 说明: " << result.strategyExplanation << endl;
    }
    
    cout << endl;
    
    // ========================================================================
    // 6. 多线程定价模拟（ThreadManager）
    // ========================================================================
    cout << "【步骤 6】多线程定价模拟" << endl;
    cout << string(60, '-') << endl;
    
    // 确保输出目录存在
    system("mkdir -p output");
    
    ThreadManager manager("output/pricing.log");
    
    // 为每个产品创建多个商家进行并发定价
    vector<Merchant> merchants;
    
    for (const auto& [productId, history] : productHistories) {
        // 为每个产品创建2-3个商家
        merchants.push_back(Merchant(
            "商家A-" + productId,
            {productId},
            1
        ));
        merchants.push_back(Merchant(
            "商家B-" + productId,
            {productId},
            2
        ));
    }
    
    cout << "🚀 启动多线程定价系统，共 " << merchants.size() << " 个商家线程..." << endl;
    manager.startPricing(merchants, strategy);
    
    // 等待所有线程完成
    cout << "⏳ 等待所有定价任务完成..." << endl;
    manager.waitAll();
    
    // 显示统计信息
    manager.printStatistics();
    
    // 获取最终价格表
    const ThreadSafePriceTable& priceTable = manager.getPriceTable();
    auto allPrices = priceTable.getAllPrices();
    
    cout << "\n--- 最终价格表 ---" << endl;
    for (const auto& [productId, price] : allPrices) {
        cout << "产品 " << productId << ": ¥" << fixed << setprecision(2) << price << endl;
    }
    
    // 导出价格趋势（ThreadManager 内部已实现）
    manager.exportPriceTrend("output/price_trend.csv");
    cout << endl;
    
    // ========================================================================
    // 7. 输出可视化数据（导出 CSV）
    // ========================================================================
    cout << "【步骤 7】导出可视化数据" << endl;
    cout << string(60, '-') << endl;
    
    ofstream csvFile("output/price_trend_detailed.csv");
    if (!csvFile.is_open()) {
        cerr << "❌ 错误：无法创建 CSV 文件" << endl;
        return 1;
    }
    
    // 写入 CSV 表头
    csvFile << "date,productId,basePrice,finalPrice,stock,alertLevel,sales,predictedDemand" << endl;
    
    // 为每个产品写入历史数据
    for (const auto& [productId, history] : productHistories) {
        // 获取最终价格（优先使用 ThreadManager 的结果，否则使用 PricingStrategy 的结果）
        double finalPrice = history.latestPrice;
        if (allPrices.find(productId) != allPrices.end()) {
            finalPrice = allPrices.at(productId);
        } else if (pricingResults.find(productId) != pricingResults.end()) {
            finalPrice = pricingResults.at(productId).newPrice;
        }
        
        // 获取告警级别字符串
        string alertLevelStr = "GREEN";
        if (alertLevels.find(productId) != alertLevels.end()) {
            switch (alertLevels.at(productId)) {
                case InventoryAlert::AlertLevel::GREEN:
                    alertLevelStr = "GREEN";
                    break;
                case InventoryAlert::AlertLevel::MEDIUM:
                    alertLevelStr = "MEDIUM";
                    break;
                case InventoryAlert::AlertLevel::HIGH:
                    alertLevelStr = "HIGH";
                    break;
                case InventoryAlert::AlertLevel::CRITICAL:
                    alertLevelStr = "CRITICAL";
                    break;
            }
        }
        
        // 获取预测需求
        double predictedDemand = nextPredictions[productId];
        
        // 写入每条历史记录
        for (size_t i = 0; i < history.dates.size(); ++i) {
            csvFile << fixed << setprecision(2);
            csvFile << history.dates[i] << ","
                    << productId << ","
                    << history.priceHistory[i] << ","
                    << (i == history.dates.size() - 1 ? finalPrice : history.priceHistory[i]) << ","
                    << history.stockHistory[i] << ","
                    << (i == history.dates.size() - 1 ? alertLevelStr : "GREEN") << ","
                    << static_cast<int>(history.salesHistory[i]) << ","
                    << (i == history.dates.size() - 1 ? predictedDemand : 0.0) << endl;
        }
        
        // 添加下一天的预测行（使用最新日期+1天，这里简化处理）
        if (!history.dates.empty()) {
            string lastDate = history.dates.back();
            // 简单日期增量（实际应该用日期库）
            csvFile << lastDate << "_next," << productId << ","
                    << history.latestPrice << "," << finalPrice << ","
                    << history.latestStock << "," << alertLevelStr << ","
                    << 0 << "," << predictedDemand << endl;
        }
    }
    
    csvFile.close();
    cout << "✅ 详细价格趋势数据已导出到: output/price_trend_detailed.csv" << endl;
    cout << "✅ ThreadManager 价格趋势已导出到: output/price_trend.csv" << endl;
    cout << endl;
    
    // ========================================================================
    // 程序总结
    // ========================================================================
    cout << "═══════════════════════════════════════════════════════" << endl;
    cout << "✨ 演示程序执行完成！" << endl;
    cout << "═══════════════════════════════════════════════════════" << endl;
    cout << "\n生成的文件:" << endl;
    cout << "  - output/pricing.log - 定价系统日志" << endl;
    cout << "  - output/price_trend.csv - ThreadManager 价格趋势" << endl;
    cout << "  - output/price_trend_detailed.csv - 详细可视化数据" << endl;
    cout << endl;
    
    return 0;
}

