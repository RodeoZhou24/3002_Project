#ifndef FORECASTER_H
#define FORECASTER_H

#include <vector>

class Forecaster {
public:
    static std::vector<double> movingAverage(const std::vector<double>& history, int window);
    static double predictNext(const std::vector<double>& history, int window = 3);
    static void displayForecast(const std::vector<double>& history, 
                               const std::vector<double>& forecast, 
                               const std::vector<std::string>& dates);
};

#endif
