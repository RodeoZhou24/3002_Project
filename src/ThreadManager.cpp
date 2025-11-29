/**
 * @file ThreadManager.cpp
 * @brief 多线程定价管理器实现
 * @author Zhao Runtian (124090988)
 * @date 2025-11-18
 */

#include "ThreadManager.h"
#include "PricingStrategy.h"  // 需要定价策略模块
#include <random>
#include <algorithm>
#include <ctime>
#include <cstdlib>

// ============================================================================
// ThreadSafePriceTable 实现
// ============================================================================

double ThreadSafePriceTable::getPrice(const std::string& productId) const {
    std::shared_lock<std::shared_mutex> lock(rwMutex);  // 共享锁（读）
    auto it = prices.find(productId);
    if (it != prices.end()) {
        return it->second;
    }
    return 0.0;  // 产品不存在返回0
}

void ThreadSafePriceTable::setPrice(const std::string& productId, double price) {
    std::unique_lock<std::shared_mutex> lock(rwMutex);  // 独占锁（写）
    prices[productId] = price;
}

bool ThreadSafePriceTable::updatePriceIfLower(const std::string& productId, double newPrice) {
    std::unique_lock<std::shared_mutex> lock(rwMutex);
    
    auto it = prices.find(productId);
    if (it == prices.end() || newPrice < it->second) {
        prices[productId] = newPrice;
        return true;
    }
    return false;
}

std::map<std::string, double> ThreadSafePriceTable::getAllPrices() const {
    std::shared_lock<std::shared_mutex> lock(rwMutex);
    return prices;  // 返回副本
}

size_t ThreadSafePriceTable::size() const {
    std::shared_lock<std::shared_mutex> lock(rwMutex);
    return prices.size();
}

// ============================================================================
// ThreadSafeLogger 实现
// ============================================================================

ThreadSafeLogger::ThreadSafeLogger(const std::string& filename) 
    : stopFlag(false), logFile(filename, std::ios::app) {
    
    if (!logFile.is_open()) {
        std::cerr << "Warning: Cannot open log file: " << filename << std::endl;
    }
    
    // 启动后台写入线程
    writerThread = std::thread(&ThreadSafeLogger::writerThreadFunc, this);
}

ThreadSafeLogger::~ThreadSafeLogger() {
    stop();
    if (writerThread.joinable()) {
        writerThread.join();
    }
    if (logFile.is_open()) {
        logFile.close();
    }
}

void ThreadSafeLogger::log(const std::string& message) {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        logQueue.push(message);
    }
    cv.notify_one();  // 通知写入线程
}

void ThreadSafeLogger::stop() {
    stopFlag = true;
    cv.notify_all();
}

void ThreadSafeLogger::writerThreadFunc() {
    while (!stopFlag || !logQueue.empty()) {
        std::unique_lock<std::mutex> lock(queueMutex);
        
        // 等待队列非空或停止信号
        cv.wait(lock, [this] { return !logQueue.empty() || stopFlag.load(); });
        
        while (!logQueue.empty()) {
            std::string message = logQueue.front();
            logQueue.pop();
            
            lock.unlock();  // 解锁后写入文件（避免阻塞其他线程）
            
            if (logFile.is_open()) {
                logFile << message << std::endl;
                logFile.flush();  // 立即刷新
            }
            
            lock.lock();
        }
    }
}

// ============================================================================
// ThreadManager 实现
// ============================================================================

ThreadManager::ThreadManager(const std::string& logFile)
    : stopFlag(false), totalTasks(0), successTasks(0), failedTasks(0) {
    
    logger = std::make_unique<ThreadSafeLogger>(logFile);
    logger->log("=== Pricing System Started ===");
}

ThreadManager::~ThreadManager() {
    stopAll();
    waitAll();
}

void ThreadManager::startPricing(const std::vector<Merchant>& merchants, 
                                  pricing::PricingStrategy& strategy) {
    
    std::cout << "\n🚀 Starting multi-threaded pricing with " 
              << merchants.size() << " merchants...\n" << std::endl;
    
    stopFlag = false;
    
    // 为每个商家创建一个线程
    for (const auto& merchant : merchants) {
        merchantThreads.emplace_back(
            &ThreadManager::merchantPricingThread, 
            this, 
            std::ref(merchant), 
            std::ref(strategy)
        );
        
        std::cout << "✓ Thread started for merchant: " << merchant.name << std::endl;
    }
    
    logger->log("All merchant threads started");
}

void ThreadManager::merchantPricingThread(const Merchant& merchant, 
                                           pricing::PricingStrategy& strategy) {
    
    std::string threadLog = "[Thread-" + merchant.name + "] Started";
    logger->log(threadLog);
    
    // 随机数生成器（线程安全）
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> delayDist(50, 200);  // 50-200ms
    
    // 处理该商家负责的所有产品
    for (const auto& productId : merchant.products) {
        
        if (stopFlag) {
            logger->log("[Thread-" + merchant.name + "] Stopped by signal");
            break;
        }
        
        // 执行定价任务
        PricingTask task = executePricingTask(merchant.name, productId, strategy);
        
        // 记录结果
        recordPriceChange(task, merchant.name);
        
        // 统计
        totalTasks++;
        if (task.success) {
            successTasks++;
        } else {
            failedTasks++;
        }
        
        // 模拟网络延迟
        std::this_thread::sleep_for(std::chrono::milliseconds(delayDist(gen)));
    }
    
    threadLog = "[Thread-" + merchant.name + "] Completed: " 
                + std::to_string(merchant.products.size()) + " products";
    logger->log(threadLog);
}

PricingTask ThreadManager::executePricingTask(const std::string& merchantName,
                                               const std::string& productId,
                                               pricing::PricingStrategy& strategy) {
    PricingTask task;
    task.merchantName = merchantName;
    task.productId = productId;
    task.timestamp = std::chrono::system_clock::now();
    
    try {
        // 1. 获取当前价格（如果存在）
        double currentPrice = priceTable.getPrice(productId);
        
        // 2. 如果是首次定价，生成基础价格
        if (currentPrice == 0.0) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<> priceDist(5000.0, 15000.0);
            currentPrice = priceDist(gen);
        }
        
        task.basePrice = currentPrice;
        
        // 3. 创建产品和市场上下文（实际项目中应从数据模块获取）
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> stockDist(50, 500);
        std::uniform_int_distribution<> viewDist(100, 2000);
        std::uniform_int_distribution<> cartDist(20, 400);
        std::uniform_int_distribution<> purchaseDist(5, 80);
        std::uniform_real_distribution<> demandDist(50.0, 250.0);
        std::uniform_real_distribution<> competitorPriceDist(0.85, 1.15);
        
        // 确定产品类别
        std::string category = "other";
        if (productId.find("iPhone") != std::string::npos) {
            category = "smartphone";
        } else if (productId.find("MacBook") != std::string::npos) {
            category = "laptop";
        } else if (productId.find("RTX") != std::string::npos) {
            category = "gpu";
        }
        
        pricing::Product product;
        product.id = productId;
        product.name = productId;
        product.category = category;
        product.basePrice = currentPrice;
        product.stock = stockDist(gen);
        product.isNewModel = (productId.find("New") != std::string::npos);
        product.series = category;
        
        pricing::MarketContext context;
        context.competitorPrice = currentPrice * competitorPriceDist(gen);
        context.demandForecast = demandDist(gen);
        context.isPeakSeason = (std::rand() % 10 < 3);  // 30% 概率是旺季
        context.viewCount = viewDist(gen);
        context.cartCount = cartDist(gen);
        context.purchaseCount = purchaseDist(gen);
        std::time_t now = std::time(nullptr);
        context.currentTime = *std::localtime(&now);
        context.newerModelInSeriesAvailable = (std::rand() % 10 < 2);  // 20% 概率有新款
        
        // 4. 调用定价策略计算新价格
        pricing::PricingResult result = strategy.calculatePrice(product, context);
        double newPrice = result.newPrice;
        task.adjustedPrice = newPrice;
        task.stockLevel = product.stock;
        
        // 5. 更新价格表
        priceTable.setPrice(productId, newPrice);
        task.success = true;
        
        // 6. 输出日志
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2);
        ss << "[" << merchantName << "] " << productId 
           << ": ¥" << currentPrice << " → ¥" << newPrice
           << " (" << std::showpos << ((newPrice / currentPrice - 1) * 100) 
           << std::noshowpos << "%)";
        
        std::cout << ss.str() << std::endl;
        logger->log(ss.str());
        
    } catch (const std::exception& e) {
        task.success = false;
        task.adjustedPrice = task.basePrice;
        
        std::string errorLog = "[ERROR] " + merchantName + " - " 
                               + productId + ": " + e.what();
        std::cerr << errorLog << std::endl;
        logger->log(errorLog);
    }
    
    return task;
}

void ThreadManager::recordPriceChange(const PricingTask& task, 
                                       const std::string& merchantName) {
    std::lock_guard<std::mutex> lock(historyMutex);
    
    PriceRecord record;
    record.timestamp = getCurrentTimeString();
    record.merchantName = merchantName;
    record.productId = task.productId;
    record.originalPrice = task.basePrice;
    record.adjustedPrice = task.adjustedPrice;
    record.adjustmentRate = (task.adjustedPrice / task.basePrice - 1) * 100;
    record.stockLevel = task.stockLevel;
    record.status = task.success ? "SUCCESS" : "FAILED";
    
    priceHistory.push_back(record);
}

void ThreadManager::waitAll() {
    for (auto& thread : merchantThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    merchantThreads.clear();
    
    std::cout << "\n✅ All merchant threads completed.\n" << std::endl;
}

void ThreadManager::stopAll() {
    stopFlag = true;
    queueCV.notify_all();  // 唤醒所有等待的线程
}

void ThreadManager::exportPriceTrend(const std::string& filename) const {
    std::ofstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Error: Cannot create file " << filename << std::endl;
        return;
    }
    
    // 写入表头
    file << "timestamp,merchant,product,original_price,adjusted_price,"
         << "adjustment_rate,stock_level,status\n";
    
    // 写入数据
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(historyMutex));
    for (const auto& record : priceHistory) {
        file << std::fixed << std::setprecision(2);
        file << record.timestamp << ","
             << record.merchantName << ","
             << record.productId << ","
             << record.originalPrice << ","
             << record.adjustedPrice << ","
             << record.adjustmentRate << "%,"
             << record.stockLevel << ","
             << record.status << "\n";
    }
    
    file.close();
    std::cout << "💾 Price trend exported to: " << filename << std::endl;
}

void ThreadManager::printStatistics() const {
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "📊 PRICING STATISTICS" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    
    std::cout << "Total tasks:     " << totalTasks << std::endl;
    std::cout << "Successful:      " << successTasks 
              << " (" << (totalTasks > 0 ? successTasks * 100.0 / totalTasks : 0) 
              << "%)" << std::endl;
    std::cout << "Failed:          " << failedTasks 
              << " (" << (totalTasks > 0 ? failedTasks * 100.0 / totalTasks : 0) 
              << "%)" << std::endl;
    std::cout << "Unique products: " << priceTable.size() << std::endl;
    
    std::cout << std::string(60, '=') << std::endl;
    
    // 显示价格范围
    auto prices = priceTable.getAllPrices();
    if (!prices.empty()) {
        auto minMax = std::minmax_element(
            prices.begin(), prices.end(),
            [](const auto& a, const auto& b) { return a.second < b.second; }
        );
        
        std::cout << "Price range:     ¥" << std::fixed << std::setprecision(2)
                  << minMax.first->second << " - ¥" << minMax.second->second << std::endl;
    }
    
    std::cout << std::string(60, '=') << "\n" << std::endl;
}

std::string ThreadManager::getCurrentTimeString() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

void ThreadManager::simulateDelay(int minMs, int maxMs) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(minMs, maxMs);
    
    int delay = dist(gen);
    std::this_thread::sleep_for(std::chrono::milliseconds(delay));
}

// ============================================================================
// 任务队列模式实现（可选功能）
// ============================================================================

void ThreadManager::addTask(const PricingTask& task) {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        taskQueue.push(task);
    }
    queueCV.notify_one();  // 唤醒一个工作线程
}

void ThreadManager::startWorkers(int numWorkers, pricing::PricingStrategy& strategy) {
    std::cout << "\n🔧 Starting " << numWorkers << " worker threads...\n" << std::endl;
    
    stopFlag = false;
    
    for (int i = 0; i < numWorkers; i++) {
        merchantThreads.emplace_back([this, i, &strategy]() {
            std::string workerName = "Worker-" + std::to_string(i);
            logger->log("[" + workerName + "] Started");
            
            while (!stopFlag) {
                PricingTask task;
                
                {
                    std::unique_lock<std::mutex> lock(queueMutex);
                    queueCV.wait(lock, [this] { 
                        return !taskQueue.empty() || stopFlag.load(); 
                    });
                    
                    if (stopFlag && taskQueue.empty()) {
                        break;
                    }
                    
                    if (!taskQueue.empty()) {
                        task = taskQueue.front();
                        taskQueue.pop();
                    } else {
                        continue;
                    }
                }
                
                // 处理任务
                task = executePricingTask(task.merchantName, task.productId, strategy);
                recordPriceChange(task, task.merchantName);
                
                totalTasks++;
                if (task.success) {
                    successTasks++;
                } else {
                    failedTasks++;
                }
            }
            
            logger->log("[" + workerName + "] Stopped");
        });
    }
}