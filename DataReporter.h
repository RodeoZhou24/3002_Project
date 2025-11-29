// DataReporter.h
#ifndef DATA_REPORTER_H
#define DATA_REPORTER_H

#include <string>
#include <fstream>
#include <mutex>


enum class ProductType { SMARTPHONE, LAPTOP, GRAPHICS_CARD, OTHER };

// Price trend data structure (aligned with ThreadManager's PriceRecord)
struct PriceTrendRecord {
    std::string timestamp;      // Time of price change
    std::string merchant_name;  // Merchant name (e.g., "Worker-0")
    std::string product_id;     // Product ID (e.g., "iPhone15")
    double original_price;      // Original price
    double adjusted_price;      // New price after adjustment
    double adjustment_rate;     // Price change percentage (e.g., -5.2 = 5.2% decrease)
    int stock_level;            // Current stock quantity
    std::string status;         // "SUCCESS" or "FAILED"
};

// Inventory warning data structure
struct InventoryWarningRecord {
    std::string timestamp;       // Time of warning
    std::string product_id;      // Product ID
    int forecasted_demand;       // Predicted demand
    int current_stock;           // Current stock
    std::string warning_level;   // "LOW_STOCK" or "OUT_OF_STOCK"
};

// Handles CSV export and console output for pricing/inventory data
class DataReporter {
private:
    std::ofstream price_file;    // CSV file for price trends
    std::ofstream warning_file;  // CSV file for inventory warnings
    std::mutex mtx;              // For thread-safe operations
    const std::string PRICE_PATH = "price_trend.csv";
    const std::string WARNING_PATH = "inventory_warning_log.csv";

    // Convert product ID to type (e.g., "RTX4090" → GRAPHICS_CARD)
    ProductType productIdToType(const std::string& product_id);
    
    // Get current time as "YYYY-MM-DD HH:MM:SS"
    std::string getCurrentTimestamp();

public:
    DataReporter();  // Initialize CSV files
    ~DataReporter(); // Close files

    // Export price trend to CSV and print to console
    void handlePriceChange(const PriceTrendRecord& record);
    
    // Export inventory warning to CSV and print to console
    void handleInventoryWarning(const InventoryWarningRecord& record);
};

#endif // DATA_REPORTER_H
