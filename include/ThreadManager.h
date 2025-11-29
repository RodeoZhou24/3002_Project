/**
 * @file ThreadManager.h
 * @brief 多线程定价管理器 - 负责模拟多商家并发定价
 * @author Zhao Runtian (124090988)
 * @date 2025-11-18
 */

#ifndef THREAD_MANAGER_H
#define THREAD_MANAGER_H

#include <memory>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <vector>
#include <map>
#include <string>
#include <queue>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>

// 前向声明
namespace pricing {
    class PricingStrategy;
}

/**
 * @brief 商家信息结构
 */
struct Merchant {
    std::string name;                    // 商家名称
    std::vector<std::string> products;   // 负责的产品列表
    int priority;                        // 优先级 (1-5, 1最高)
    
    Merchant(const std::string& n, const std::vector<std::string>& p, int prio = 3)
        : name(n), products(p), priority(prio) {}
};

/**
 * @brief 定价任务结构
 */
struct PricingTask {
    std::string merchantName;
    std::string productId;
    double basePrice;
    double adjustedPrice;
    int stockLevel;
    std::chrono::system_clock::time_point timestamp;
    bool success;
    
    PricingTask() : basePrice(0), adjustedPrice(0), stockLevel(0), success(false) {}
};

/**
 * @brief 价格记录（用于持久化）
 */
struct PriceRecord {
    std::string timestamp;
    std::string merchantName;
    std::string productId;
    double originalPrice;
    double adjustedPrice;
    double adjustmentRate;
    int stockLevel;
    std::string status;  // "SUCCESS" or "FAILED"
};

/**
 * @brief 线程安全的价格表
 * 使用读写锁优化并发读性能
 */
class ThreadSafePriceTable {
private:
    std::map<std::string, double> prices;
    mutable std::shared_mutex rwMutex;  // 读写锁
    
public:
    /**
     * @brief 获取产品价格（支持多线程并发读）
     */
    double getPrice(const std::string& productId) const;
    
    /**
     * @brief 设置产品价格（独占写）
     */
    void setPrice(const std::string& productId, double price);
    
    /**
     * @brief 原子操作：仅当新价格更低时更新
     */
    bool updatePriceIfLower(const std::string& productId, double newPrice);
    
    /**
     * @brief 获取所有价格（快照）
     */
    std::map<std::string, double> getAllPrices() const;
    
    /**
     * @brief 价格表大小
     */
    size_t size() const;
};

/**
 * @brief 线程安全的日志队列
 * 使用无锁队列优化性能
 */
class ThreadSafeLogger {
private:
    std::queue<std::string> logQueue;
    mutable std::mutex queueMutex;
    std::condition_variable cv;
    std::atomic<bool> stopFlag;
    std::thread writerThread;
    std::ofstream logFile;
    
    void writerThreadFunc();
    
public:
    explicit ThreadSafeLogger(const std::string& filename);
    ~ThreadSafeLogger();
    
    /**
     * @brief 添加日志（非阻塞）
     */
    void log(const std::string& message);
    
    /**
     * @brief 停止日志写入
     */
    void stop();
};

/**
 * @brief 多线程定价管理器
 */
class ThreadManager {
private:
    // 线程管理
    std::vector<std::thread> merchantThreads;
    std::atomic<bool> stopFlag;
    
    // 数据结构
    ThreadSafePriceTable priceTable;
    std::vector<PriceRecord> priceHistory;
    
    mutable std::mutex historyMutex;
    
    
    // 日志
    std::unique_ptr<ThreadSafeLogger> logger;
    
    // 统计信息
    std::atomic<int> totalTasks;
    std::atomic<int> successTasks;
    std::atomic<int> failedTasks;
    
    // 任务队列（可选：使用任务队列模式）
    std::queue<PricingTask> taskQueue;
    std::mutex queueMutex;
    std::condition_variable queueCV;
    
    /**
     * @brief 商家定价线程函数
     */
    void merchantPricingThread(const Merchant& merchant, pricing::PricingStrategy& strategy);
    
    /**
     * @brief 执行单个定价任务
     */
    PricingTask executePricingTask(const std::string& merchantName,
                                    const std::string& productId,
                                    pricing::PricingStrategy& strategy);
    
    /**
     * @brief 记录价格变更历史
     */
    void recordPriceChange(const PricingTask& task, const std::string& merchantName);
    
    /**
     * @brief 获取当前时间字符串
     */
    std::string getCurrentTimeString() const;
    
    /**
     * @brief 模拟随机延迟（模拟网络延迟）
     */
    void simulateDelay(int minMs, int maxMs);

public:
    /**
     * @brief 构造函数
     */
    explicit ThreadManager(const std::string& logFile = "output/pricing.log");
    
    /**
     * @brief 析构函数
     */
    ~ThreadManager();
    
    /**
     * @brief 启动多商家定价（主入口）
     */
    void startPricing(const std::vector<Merchant>& merchants, pricing::PricingStrategy& strategy);
    
    /**
     * @brief 等待所有线程完成
     */
    void waitAll();
    
    /**
     * @brief 停止所有定价线程
     */
    void stopAll();
    
    /**
     * @brief 导出价格趋势CSV
     */
    void exportPriceTrend(const std::string& filename) const;
    
    /**
     * @brief 打印统计报告
     */
    void printStatistics() const;
    
    /**
     * @brief 获取当前价格表
     */
    const ThreadSafePriceTable& getPriceTable() const { return priceTable; }
    
    /**
     * @brief 任务队列模式：添加任务
     */
    void addTask(const PricingTask& task);
    
    /**
     * @brief 任务队列模式：启动工作线程
     */
    void startWorkers(int numWorkers, pricing::PricingStrategy& strategy);
};

#endif // THREAD_MANAGER_H