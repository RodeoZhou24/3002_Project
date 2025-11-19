#ifndef DATALOADER_H
#define DATALOADER_H

#include <vector>
#include <string>

struct Sale {
    std::string date;
    std::string productId;
    int sales;
    double price;
    int stock;
};

class DataLoader {
public:
    DataLoader(const std::string& filename);
    bool loadData();
    const std::vector<Sale>& getSalesData() const;
    void displayData() const;

private:
    std::string filename;
    std::vector<Sale> salesData;
};

#endif
