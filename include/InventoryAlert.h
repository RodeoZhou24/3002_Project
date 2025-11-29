#ifndef INVENTORY_ALERT_H
#define INVENTORY_ALERT_H

#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <ctime>
#include <iomanip>
#include <sstream>

using namespace std;

/**
 * @brief Inventory Alert System for Electronic Products
 * @author Wang Jiarui (124090612)
 * 
 * This class implements a multi-level inventory warning system that:
 * - Monitors stock levels in real-time
 * - Predicts potential stockouts
 * - Generates alerts with different urgency levels
 * - Logs all alerts for historical analysis
 */
class InventoryAlert {
public:
    // Alert levels enum for better type safety
    enum class AlertLevel {
        GREEN,      // Stock sufficient
        MEDIUM,     // Monitor closely
        HIGH,       // Replenishment needed soon
        CRITICAL    // Immediate action required
    };

    // Product categories for electronics
    enum class ProductCategory {
        SMARTPHONE,
        LAPTOP,
        GPU,
        TABLET,
        GENERAL
    };

    // Alert record structure
    struct AlertRecord {
        string timestamp;
        string productID;
        string productName;
        int currentStock;
        double forecastDemand;
        AlertLevel level;
        ProductCategory category;
        string message;

        AlertRecord() : currentStock(0), forecastDemand(0.0), 
                       level(AlertLevel::GREEN), category(ProductCategory::GENERAL) {}
    };

private:
    vector<AlertRecord> alertHistory;           // All alert records
    map<string, int> productThresholds;         // Custom thresholds per product
    map<string, int> alertCountByProduct;       // Alert frequency tracking
    mutable mutex alertMutex;                   // Thread-safe operations
    int totalAlerts;                            // Total alert counter

    // Helper functions
    string getCurrentTimestamp() const;
    string alertLevelToString(AlertLevel level) const;
    string categoryToString(ProductCategory category) const;
    double getCategoryMultiplier(ProductCategory category) const;
    bool isPromotionalPeriod() const;

public:
    // Constructor
    InventoryAlert();

    // Core alert checking functions
    bool isAlert(const string& productID, double forecast, int currentStock, 
                 ProductCategory category = ProductCategory::GENERAL);
    
    bool checkAlert(const string& productID, const string& productName,
                   double forecast, int currentStock, 
                   ProductCategory category = ProductCategory::GENERAL);

    // Alert level determination
    AlertLevel getAlertLevel(double forecast, int currentStock) const;
    
    // Threshold calculation
    int calculateThreshold(const string& productID, ProductCategory category,
                          int avgDailySales, int leadTimeDays) const;
    
    void setProductThreshold(const string& productID, int threshold);
    int getProductThreshold(const string& productID) const;

    // Alert recording and logging
    void recordAlert(const AlertRecord& alert);
    void exportAlertLog(const string& filename) const;
    void printAlert(const AlertRecord& alert) const;

    // Statistics and reporting
    int getTotalAlerts() const;
    map<string, int> getAlertsByProduct() const;
    vector<AlertRecord> getCriticalAlerts() const;
    vector<AlertRecord> getAlertsByLevel(AlertLevel level) const;
    vector<AlertRecord> getAllAlerts() const;
    
    // Clear history
    void clearAlertHistory();
    
    // Display functions
    void displayAlertSummary() const;
    void displayRecentAlerts(int count = 10) const;
};

#endif // INVENTORY_ALERT_H