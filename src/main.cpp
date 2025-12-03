/**
 * @file main.cpp
 * @brief 动态定价系统 - 集成可视化版本 (修复版)
 */

#include "DataLoader.h"
#include "Forecaster.h"
#include "InventoryAlert.h"
#include "PricingStrategy.h"
#include "../include/Visualizer.h"
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <iomanip>

using namespace std;
using namespace pricing;

// 简单的辅助结构，用于在内存中整理历史数据
struct ProductHistory {
    vector<string> dates;
    vector<double> prices;
    vector<int> stocks;
    vector<double> sales; // 修复：类型改为 double，匹配 Forecaster 接口
    double lastPrice;
    int lastStock;
};

int main() {
    cout << "=== Intelligent Pricing System Initiated ===" << endl;

    // 1. 数据加载
    DataLoader loader("sales_history.txt");
    if (!loader.loadData()) {
        loader = DataLoader("../sales_history.txt");
        if (!loader.loadData()) {
            cerr << "❌ Error: Cannot open sales_history.txt" << endl;
            return 1;
        }
    }
    const vector<Sale>& allSales = loader.getSalesData();
    cout << "✅ Loaded " << allSales.size() << " records." << endl;

    // 2. 整理数据 (按产品分组)
    map<string, ProductHistory> histories;
    for (const auto& s : allSales) {
        histories[s.productId].dates.push_back(s.date);
        histories[s.productId].prices.push_back(s.price);
        histories[s.productId].stocks.push_back(s.stock);
        // 修复：显式转换为 double
        histories[s.productId].sales.push_back(static_cast<double>(s.sales));
        histories[s.productId].lastPrice = s.price;
        histories[s.productId].lastStock = s.stock;
    }

    // 3. 运行核心逻辑
    PricingStrategy strategy;
    InventoryAlert alert;

    system("mkdir -p output");
    string csvPath = "output/price_trend_detailed.csv";
    ofstream csvFile(csvPath);

    if (csvFile.is_open()) {
        csvFile << "date,productId,basePrice,finalPrice,stock,alertLevel,sales,predictedDemand" << endl;

        for (auto& [pid, h] : histories) {
            // A. 预测
            vector<double> forecast = Forecaster::movingAverage(h.sales, 3);
            double nextDemand = forecast.empty() ? 0.0 : Forecaster::predictNext(h.sales, 3);

            // B. 预警 (修复：添加 productName 参数)
            alert.checkAlert(pid, "Product " + pid, nextDemand, h.lastStock);

            // C. 定价
            Product p;
            p.id = pid;
            p.basePrice = h.lastPrice;
            p.stock = h.lastStock;

            MarketContext ctx;
            ctx.demandForecast = nextDemand;
            ctx.competitorPrice = h.lastPrice * 0.98;

            PricingResult res = strategy.calculatePrice(p, ctx);

            cout << "Product " << pid << ": New Price -> " << res.newPrice << endl;

            // D. 写入 CSV
            for (size_t i = 0; i < h.dates.size(); ++i) {
                double finalP = (i == h.dates.size() - 1) ? res.newPrice : h.prices[i];
                double demand = (i == h.dates.size() - 1) ? nextDemand : 0.0;

                csvFile << h.dates[i] << "," << pid << ","
                        << h.prices[i] << "," << finalP << ","
                        << h.stocks[i] << ",GREEN,"
                        << h.sales[i] << "," << demand << endl;
            }
        }
        csvFile.close();
        cout << "✅ Logic complete. Data exported to CSV." << endl;
    } else {
        cerr << "❌ Error: Cannot create output CSV." << endl;
        return 1;
    }

    // 4. 可视化
    Visualizer::generateDashboard(csvPath, "output/dashboard.html");

    return 0;
}