#pragma once

#include <string>
#include <ctime>

namespace pricing {

struct Product {
    std::string id;
    std::string name;
    std::string category;
    double basePrice{0.0};
    int stock{0};
    bool isNewModel{false};
    std::string series;
};

struct MarketContext {
    double competitorPrice{0.0};
    double demandForecast{0.0};
    bool isPeakSeason{false};
    int viewCount{0};
    int cartCount{0};
    int purchaseCount{0};
    std::tm currentTime{};
    bool newerModelInSeriesAvailable{false};
};

struct PricingResult {
    double newPrice{0.0};
    double adjustment{0.0};
    double stockFactor{0.0};
    double competitorFactor{0.0};
    double demandFactor{0.0};
    double timeFactor{0.0};
    std::string strategyExplanation;
};

class PricingStrategy {
public:
    PricingStrategy() = default;
    PricingResult calculatePrice(const Product& product,
                                 const MarketContext& context) const;

private:
    double computeStockFactor(const Product& product,
                              const MarketContext& context) const;
    double computeCompetitorFactor(const Product& product,
                                   const MarketContext& context) const;
    double computeDemandFactor(const Product& product,
                               const MarketContext& context) const;
    double computeTimeFactor(const Product& product,
                             const MarketContext& context) const;

    double applyNewProductStrategy(const Product& product,
                                   const MarketContext& context) const;
    double applyCompetitorStrategy(const Product& product,
                                   const MarketContext& context) const;
    double applySeasonalStrategy(const Product& product,
                                 const MarketContext& context) const;
    double applyUserBehaviorStrategy(const Product& product,
                                     const MarketContext& context) const;
    double clampPrice(double price, double basePrice) const;
};

}  // namespace pricing



