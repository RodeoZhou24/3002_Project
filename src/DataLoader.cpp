#include "DataLoader.h"
#include <fstream>
#include <sstream>
#include <iostream>

DataLoader::DataLoader(const std::string& filename) : filename(filename) {}

bool DataLoader::loadData() {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        return false;
    }

    std::string line;
    // 跳过标题行（如果有）
    std::getline(file, line);
    
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        Sale sale;
        std::string token;
        
        // 解析CSV格式：date,productId,sales,price,stock
        std::getline(ss, token, ',');
        sale.date = token;
        
        std::getline(ss, token, ',');
        sale.productId = token;
        
        std::getline(ss, token, ',');
        sale.sales = std::stoi(token);
        
        std::getline(ss, token, ',');
        sale.price = std::stod(token);
        
        std::getline(ss, token, ',');
        sale.stock = std::stoi(token);
        
        salesData.push_back(sale);
    }
    
    file.close();
    std::cout << "Successfully loaded " << salesData.size() << " sales records." << std::endl;
    return true;
}

const std::vector<Sale>& DataLoader::getSalesData() const {
    return salesData;
}

void DataLoader::displayData() const {
    std::cout << "\n=== Sales Data Preview ===" << std::endl;
    std::cout << "Date\t\tProductID\tSales\tPrice\tStock" << std::endl;
    for (const auto& sale : salesData) {
        std::cout << sale.date << "\t" << sale.productId << "\t\t" 
                  << sale.sales << "\t" << sale.price << "\t" << sale.stock << std::endl;
    }
}
