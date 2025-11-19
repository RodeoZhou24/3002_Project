#include "InventoryAlert.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <iomanip>

using namespace std;

// Constructor
InventoryAlert::InventoryAlert() : totalAlerts(0) {
    // Initialize with default thresholds if needed
}

// Get current timestamp in formatted string
string InventoryAlert::getCurrentTimestamp() const {
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
    return string(buffer);
}

// Convert AlertLevel enum to string
string InventoryAlert::alertLevelToString(AlertLevel level) const {
    switch(level) {
        case AlertLevel::GREEN: return "GREEN";
        case AlertLevel::MEDIUM: return "MEDIUM";
        case AlertLevel::HIGH: return "HIGH";
        case AlertLevel::CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

// Convert ProductCategory enum to string
string InventoryAlert::categoryToString(ProductCategory category) const {
    switch(category) {
        case ProductCategory::SMARTPHONE: return "Smartphone";
        case ProductCategory::LAPTOP: return "Laptop";
        case ProductCategory::GPU: return "GPU";
        case ProductCategory::TABLET: return "Tablet";
        case ProductCategory::GENERAL: return "General";
        default: return "Unknown";
    }
}

// Get category-specific multiplier for threshold calculation
double InventoryAlert::getCategoryMultiplier(ProductCategory category) const {
    switch(category) {
        case ProductCategory::SMARTPHONE: return 1.5;  // High turnover
        case ProductCategory::LAPTOP: return 1.3;
        case ProductCategory::GPU: return 1.2;
        case ProductCategory::TABLET: return 1.4;
        case ProductCategory::GENERAL: return 1.0;
        default: return 1.0;
    }
}

// Check if current period is promotional (simplified version)
bool InventoryAlert::isPromotionalPeriod() const {
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    
    int month = timeinfo->tm_mon + 1;  // 0-based to 1-based
    int day = timeinfo->tm_mday;
    
    // Check for major shopping events
    // 618 (June 18), Double 11 (Nov 11), Black Friday (approximation)
    if ((month == 6 && day >= 15 && day <= 20) ||     // 618
        (month == 11 && day >= 10 && day <= 12) ||     // Double 11
        (month == 11 && day >= 23 && day <= 26)) {     // Black Friday
        return true;
    }
    return false;
}

// Simple alert check (backward compatibility)
bool InventoryAlert::isAlert(const string& productID, double forecast, 
                             int currentStock, ProductCategory category) {
    if (currentStock <= 0 || forecast <= 0) {
        return false;
    }
    
    // Calculate threshold
    int threshold = getProductThreshold(productID);
    if (threshold <= 0) {
        // Use default calculation if no custom threshold
        threshold = calculateThreshold(productID, category, 
                                      static_cast<int>(forecast / 7), 7);
    }
    
    // Check if alert needed
    return forecast > currentStock;
}

// Comprehensive alert check with recording
bool InventoryAlert::checkAlert(const string& productID, const string& productName,
                                double forecast, int currentStock, 
                                ProductCategory category) {
    if (currentStock <= 0 || forecast <= 0) {
        return false;
    }
    
    // Determine alert level
    AlertLevel level = getAlertLevel(forecast, currentStock);
    
    // Only create alert if level is MEDIUM or higher
    if (level == AlertLevel::GREEN) {
        return false;
    }
    
    // Create alert record
    AlertRecord alert;
    alert.timestamp = getCurrentTimestamp();
    alert.productID = productID;
    alert.productName = productName;
    alert.currentStock = currentStock;
    alert.forecastDemand = forecast;
    alert.level = level;
    alert.category = category;
    
    // Generate appropriate message based on level
    switch(level) {
        case AlertLevel::CRITICAL:
            alert.message = "CRITICAL: Immediate replenishment required!";
            break;
        case AlertLevel::HIGH:
            alert.message = "HIGH: Replenishment needed within 3 days";
            break;
        case AlertLevel::MEDIUM:
            alert.message = "MEDIUM: Monitor closely, prepare for restocking";
            break;
        default:
            alert.message = "Inventory sufficient";
            break;
    }
    
    // Record the alert
    recordAlert(alert);
    
    // Print to console
    printAlert(alert);
    
    return true;
}

// Determine alert level based on forecast vs stock ratio
InventoryAlert::AlertLevel InventoryAlert::getAlertLevel(double forecast, 
                                                          int currentStock) const {
    if (currentStock <= 0) {
        return AlertLevel::CRITICAL;
    }
    
    double ratio = forecast / currentStock;
    
    if (ratio >= 1.5) {
        return AlertLevel::CRITICAL;
    } else if (ratio >= 1.2) {
        return AlertLevel::HIGH;
    } else if (ratio >= 1.0) {
        return AlertLevel::MEDIUM;
    } else {
        return AlertLevel::GREEN;
    }
}

// Calculate dynamic threshold for a product
int InventoryAlert::calculateThreshold(const string& productID, 
                                       ProductCategory category,
                                       int avgDailySales, 
                                       int leadTimeDays) const {
    // Base safety stock calculation
    int baseSafetyStock = avgDailySales * leadTimeDays;
    
    // Apply category-specific multiplier
    double multiplier = getCategoryMultiplier(category);
    
    // Adjust for promotional periods
    if (isPromotionalPeriod()) {
        multiplier *= 1.3;
    }
    
    return static_cast<int>(baseSafetyStock * multiplier);
}

// Set custom threshold for a product
void InventoryAlert::setProductThreshold(const string& productID, int threshold) {
    lock_guard<mutex> lock(alertMutex);
    productThresholds[productID] = threshold;
}

// Get threshold for a product
int InventoryAlert::getProductThreshold(const string& productID) const {
    lock_guard<mutex> lock(alertMutex);
    auto it = productThresholds.find(productID);
    return (it != productThresholds.end()) ? it->second : 0;
}

// Record an alert with thread safety
void InventoryAlert::recordAlert(const AlertRecord& alert) {
    lock_guard<mutex> lock(alertMutex);
    
    alertHistory.push_back(alert);
    totalAlerts++;
    
    // Update alert count by product
    alertCountByProduct[alert.productID]++;
}

// Print alert to console with color coding (simplified)
void InventoryAlert::printAlert(const AlertRecord& alert) const {
    cout << "\n";
    cout << "════════════════════════════════════════════════════════════════\n";
    cout << "  INVENTORY ALERT - " << alertLevelToString(alert.level) << "\n";
    cout << "════════════════════════════════════════════════════════════════\n";
    cout << "  Time:     " << alert.timestamp << "\n";
    cout << "  Product:  " << alert.productName << " (" << alert.productID << ")\n";
    cout << "  Category: " << categoryToString(alert.category) << "\n";
    cout << "  Stock:    " << alert.currentStock << " units\n";
    cout << "  Forecast: " << fixed << setprecision(1) << alert.forecastDemand << " units\n";
    
    double ratio = alert.forecastDemand / alert.currentStock;
    cout << "  Ratio:    " << fixed << setprecision(2) << ratio << "x\n";
    cout << "  Message:  " << alert.message << "\n";
    cout << "════════════════════════════════════════════════════════════════\n\n";
}

// Export all alerts to a log file
void InventoryAlert::exportAlertLog(const string& filename) const {
    lock_guard<mutex> lock(alertMutex);
    
    ofstream outFile(filename);
    if (!outFile.is_open()) {
        cerr << "Error: Unable to open file " << filename << " for writing.\n";
        return;
    }
    
    // Write header
    outFile << "Timestamp,ProductID,ProductName,Category,CurrentStock,ForecastDemand,AlertLevel,Message\n";
    
    // Write all alert records
    for (const auto& alert : alertHistory) {
        outFile << alert.timestamp << ","
                << alert.productID << ","
                << alert.productName << ","
                << categoryToString(alert.category) << ","
                << alert.currentStock << ","
                << fixed << setprecision(2) << alert.forecastDemand << ","
                << alertLevelToString(alert.level) << ","
                << alert.message << "\n";
    }
    
    outFile.close();
    cout << "Alert log exported to " << filename << " (" 
         << alertHistory.size() << " records)\n";
}

// Get total number of alerts
int InventoryAlert::getTotalAlerts() const {
    lock_guard<mutex> lock(alertMutex);
    return totalAlerts;
}

// Get alert count grouped by product
map<string, int> InventoryAlert::getAlertsByProduct() const {
    lock_guard<mutex> lock(alertMutex);
    return alertCountByProduct;
}

// Get all critical alerts
vector<InventoryAlert::AlertRecord> InventoryAlert::getCriticalAlerts() const {
    return getAlertsByLevel(AlertLevel::CRITICAL);
}

// Get alerts by specific level
vector<InventoryAlert::AlertRecord> InventoryAlert::getAlertsByLevel(AlertLevel level) const {
    lock_guard<mutex> lock(alertMutex);
    
    vector<AlertRecord> result;
    for (const auto& alert : alertHistory) {
        if (alert.level == level) {
            result.push_back(alert);
        }
    }
    return result;
}

// Get all alert records
vector<InventoryAlert::AlertRecord> InventoryAlert::getAllAlerts() const {
    lock_guard<mutex> lock(alertMutex);
    return alertHistory;
}

// Clear all alert history
void InventoryAlert::clearAlertHistory() {
    lock_guard<mutex> lock(alertMutex);
    alertHistory.clear();
    alertCountByProduct.clear();
    totalAlerts = 0;
}

// Display alert summary statistics
void InventoryAlert::displayAlertSummary() const {
    lock_guard<mutex> lock(alertMutex);
    
    cout << "\n";
    cout << "════════════════════════════════════════════════════════════════\n";
    cout << "              INVENTORY ALERT SUMMARY\n";
    cout << "════════════════════════════════════════════════════════════════\n";
    cout << "  Total Alerts: " << totalAlerts << "\n";
    cout << "  ────────────────────────────────────────────────────────────\n";
    
    // Count by level
    int criticalCount = 0, highCount = 0, mediumCount = 0, greenCount = 0;
    for (const auto& alert : alertHistory) {
        switch(alert.level) {
            case AlertLevel::CRITICAL: criticalCount++; break;
            case AlertLevel::HIGH: highCount++; break;
            case AlertLevel::MEDIUM: mediumCount++; break;
            case AlertLevel::GREEN: greenCount++; break;
        }
    }
    
    cout << "  Critical Alerts: " << criticalCount << "\n";
    cout << "  High Alerts:     " << highCount << "\n";
    cout << "  Medium Alerts:   " << mediumCount << "\n";
    cout << "  ────────────────────────────────────────────────────────────\n";
    
    // Top products with most alerts
    if (!alertCountByProduct.empty()) {
        cout << "  Top Products by Alert Frequency:\n";
        
        vector<pair<string, int>> sortedProducts(alertCountByProduct.begin(), 
                                                 alertCountByProduct.end());
        sort(sortedProducts.begin(), sortedProducts.end(),
             [](const pair<string, int>& a, const pair<string, int>& b) {
                 return a.second > b.second;
             });
        
        int displayCount = min(5, static_cast<int>(sortedProducts.size()));
        for (int i = 0; i < displayCount; i++) {
            cout << "    " << (i+1) << ". " << sortedProducts[i].first 
                 << " (" << sortedProducts[i].second << " alerts)\n";
        }
    }
    
    cout << "════════════════════════════════════════════════════════════════\n\n";
}

// Display recent alerts
void InventoryAlert::displayRecentAlerts(int count) const {
    lock_guard<mutex> lock(alertMutex);
    
    cout << "\n";
    cout << "════════════════════════════════════════════════════════════════\n";
    cout << "              RECENT ALERTS (Last " << count << ")\n";
    cout << "════════════════════════════════════════════════════════════════\n";
    
    int start = max(0, static_cast<int>(alertHistory.size()) - count);
    for (int i = start; i < alertHistory.size(); i++) {
        const auto& alert = alertHistory[i];
        cout << "[" << alertLevelToString(alert.level) << "] "
             << alert.timestamp << " - "
             << alert.productName << " (" << alert.productID << "): "
             << "Stock=" << alert.currentStock
             << ", Forecast=" << fixed << setprecision(1) << alert.forecastDemand << "\n";
    }
    
    cout << "════════════════════════════════════════════════════════════════\n\n";
}