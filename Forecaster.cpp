#include "Forecaster.h"
#include <iostream>
#include <numeric>

std::vector<double> Forecaster::movingAverage(const std::vector<double>& history, int window) {
    std::vector<double> forecast;
    
    if (history.size() < window) {
        std::cerr << "Error: Not enough data for moving average with window " << window << std::endl;
        return forecast;
    }
    
    for (size_t i = window - 1; i < history.size(); ++i) {
        double sum = 0.0;
        for (int j = 0; j < window; ++j) {
            sum += history[i - j];
        }
        forecast.push_back(sum / window);
    }
    
    return forecast;
}

double Forecaster::predictNext(const std::vector<double>& history, int window) {
    if (history.size() < window) {
        std::cerr << "Error: Not enough data for prediction" << std::endl;
        return 0.0;
    }
    
    double sum = 0.0;
    for (int i = 0; i < window; ++i) {
        sum += history[history.size() - 1 - i];
    }
    return sum / window;
}

void Forecaster::displayForecast(const std::vector<double>& history, 
                                const std::vector<double>& forecast, 
                                const std::vector<std::string>& dates) {
    std::cout << "\n=== Sales Forecast Results ===" << std::endl;
    std::cout << "Date\t\tActual\tForecast" << std::endl;
    
    size_t forecastStart = history.size() - forecast.size();
    for (size_t i = 0; i < forecast.size(); ++i) {
        std::cout << dates[forecastStart + i] << "\t" 
                  << history[forecastStart + i] << "\t" 
                  << forecast[i] << std::endl;
    }
    
    // 预测下一天
    double nextPrediction = predictNext(history);
    std::cout << "\nNext day prediction: " << nextPrediction << " units" << std::endl;
}
