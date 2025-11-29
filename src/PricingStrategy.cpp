#include "PricingStrategy.h"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <string>

namespace pricing {

namespace {
// 价格波动区间（Allowed swing range）。
constexpr double kMinPriceMultiplier = 0.5;
constexpr double kMaxPriceMultiplier = 2.0;

// 防止归一化时除零 (avoid divide-by-zero).
double safeDivider(double value) {
    return value == 0.0 ? 1.0 : value;
}
}  // namespace

// 核心入口：融合多项启发式得到最终价格 (blend heuristics).
PricingResult PricingStrategy::calculatePrice(const Product& product,
                                              const MarketContext& context) const {
    constexpr double kStockWeight = 0.35;       // Inventory pressure + new product impact
    constexpr double kCompetitorWeight = 0.25;  // Market competition reaction
    constexpr double kDemandWeight = 0.25;      // Forecast demand & user behavior
    constexpr double kTimeWeight = 0.15;        // Seasonal & temporal adjustments

    const double stockFactor = computeStockFactor(product, context);
    const double competitorFactor = computeCompetitorFactor(product, context);
    const double demandFactor = computeDemandFactor(product, context);
    const double timeFactor = computeTimeFactor(product, context);

    const double adjustment = kStockWeight * stockFactor +
                              kCompetitorWeight * competitorFactor +
                              kDemandWeight * demandFactor +
                              kTimeWeight * timeFactor;

    PricingResult result{};
    result.adjustment = adjustment;
    result.stockFactor = stockFactor;
    result.competitorFactor = competitorFactor;
    result.demandFactor = demandFactor;
    result.timeFactor = timeFactor;

    const double unclampedPrice = product.basePrice * (1.0 + adjustment);
    result.newPrice = clampPrice(unclampedPrice, product.basePrice);

    std::ostringstream oss;
    oss << "Stock factor=" << stockFactor << ", competitor factor=" << competitorFactor
        << ", demand factor=" << demandFactor << ", time factor=" << timeFactor << ". ";

    if (!product.isNewModel && context.newerModelInSeriesAvailable) {
        oss << "Newer model detected in series; discount applied. ";
    }
    if (context.competitorPrice > 0.0) {
        const double competitorGap =
            (context.competitorPrice - product.basePrice) / safeDivider(product.basePrice);
        if (competitorGap < -0.05) {
            oss << "Competitor undercut detected (" << competitorGap * 100
                << "%); responding with price decrease. ";
        } else if (competitorGap > 0.05) {
            oss << "Competitor priced higher; slight premium maintained. ";
        }
    }
    if (context.isPeakSeason) {
        oss << "Peak season active; seasonal strategy influencing price. ";
    }
    const double conversionRate =
        static_cast<double>(context.purchaseCount) / safeDivider(context.viewCount);
    if (context.viewCount > 50 && conversionRate < 0.05) {
        oss << "High interest but low conversion; engagement discount applied. ";
    }

    oss << "Final adjustment=" << adjustment * 100 << "%, price clamped to "
        << result.newPrice << ".";
    result.strategyExplanation = oss.str();

    return result;
}

// 库存与产品生命周期影响定价 (inventory & lifecycle impact).
double PricingStrategy::computeStockFactor(const Product& product,
                                           const MarketContext& context) const {
    const double demand = std::max(context.demandForecast, 1.0);
    const double inventoryRatio =
        static_cast<double>(std::max(product.stock, 0)) / safeDivider(demand);

    double factor = 0.0;
    if (inventoryRatio > 2.5) {
        factor = -0.18;
    } else if (inventoryRatio > 1.5) {
        factor = -0.08;
    } else if (inventoryRatio < 0.3) {
        factor = 0.12;
    } else if (inventoryRatio < 0.7) {
        factor = 0.05;
    }

    factor += applyNewProductStrategy(product, context);
    return std::clamp(factor, -0.25, 0.2);
}

// 竞品比价 + 活动策略 (competitor comparison & campaigns).
double PricingStrategy::computeCompetitorFactor(const Product& product,
                                                const MarketContext& context) const {
    double factor = 0.0;
    if (context.competitorPrice > 0.0) {
        const double gap =
            (context.competitorPrice - product.basePrice) / safeDivider(product.basePrice);
        if (gap < -0.05) {
            factor = -0.04;
        } else if (gap > 0.1) {
            factor = 0.05;
        }
    }
    factor += applyCompetitorStrategy(product, context);
    return std::clamp(factor, -0.3, 0.2);
}

// 需求预测叠加站内行为 (forecast + onsite behavior).
double PricingStrategy::computeDemandFactor(const Product& product,
                                            const MarketContext& context) const {
    (void)product;
    const double normalizedDemand = std::clamp(context.demandForecast / 200.0, -0.2, 0.2);
    double factor = normalizedDemand;
    factor += applyUserBehaviorStrategy(product, context);
    return std::clamp(factor, -0.25, 0.25);
}

// 季节策略 + 日内节奏 (seasonal plus intraday boosts).
double PricingStrategy::computeTimeFactor(const Product& product,
                                          const MarketContext& context) const {
    double factor = applySeasonalStrategy(product, context);
    const int hour = context.currentTime.tm_hour;
    if (hour >= 20 || hour < 6) {
        factor -= 0.01;  // Encourage conversions during late hours
    } else if (hour >= 10 && hour <= 16) {
        factor += 0.01;  // Prime shopping hours
    }
    return std::clamp(factor, -0.15, 0.2);
}

// 有新款时下调旧款 (discount legacy when newer sibling exists).
double PricingStrategy::applyNewProductStrategy(const Product& product,
                                                const MarketContext& context) const {
    if (product.isNewModel || !context.newerModelInSeriesAvailable) {
        return 0.0;
    }

    const double stockPressure =
        std::clamp(static_cast<double>(product.stock) / 500.0, 0.0, 1.0);
    return -0.05 - 0.1 * stockPressure;
}

// 应对竞品压价或溢价 (handle undercut/premium).
double PricingStrategy::applyCompetitorStrategy(const Product& product,
                                                const MarketContext& context) const {
    if (context.competitorPrice <= 0.0) {
        return 0.0;
    }

    const double priceRatio =
        (context.competitorPrice - product.basePrice) / safeDivider(product.basePrice);
    if (priceRatio < -0.05) {
        double severity =
            std::clamp((-0.05 - priceRatio) / 0.3, 0.0, 1.0);  // 5%–35% undercut window
        double adjustment = -0.06 - 0.12 * severity;
        if (product.isNewModel || context.demandForecast > 150.0) {
            adjustment *= 0.6;
        }
        return adjustment;
    }
    if (priceRatio > 0.05) {
        return std::min(0.05, priceRatio * 0.5);
    }
    return 0.0;
}

// 旺季根据库存微调 (peak-season adjustment via stock).
double PricingStrategy::applySeasonalStrategy(const Product& product,
                                              const MarketContext& context) const {
    if (!context.isPeakSeason) {
        return 0.0;
    }

    const double demand = std::max(context.demandForecast, 1.0);
    const double stockRatio =
        static_cast<double>(std::max(product.stock, 0)) / safeDivider(demand);

    double adjustment = 0.04;
    if (stockRatio < 0.3) {
        adjustment += 0.04;
    } else if (stockRatio < 0.7) {
        adjustment += 0.02;
    } else {
        adjustment -= 0.02;
    }
    return adjustment;
}

// 基于浏览/加购/成交漏斗反馈 (funnel reaction).
double PricingStrategy::applyUserBehaviorStrategy(const Product& product,
                                                  const MarketContext& context) const {
    (void)product;
    if (context.viewCount < 50) {
        return 0.0;
    }

    const double conversionRate =
        static_cast<double>(context.purchaseCount) / safeDivider(context.viewCount);
    const double cartRate =
        static_cast<double>(context.cartCount) / safeDivider(context.viewCount);

    if (conversionRate < 0.03 && cartRate > 0.1) {
        return -0.08;
    }
    if (conversionRate < 0.05) {
        return -0.05;
    }
    if (cartRate > 0.2 && conversionRate < 0.1) {
        return -0.03;
    }
    return 0.0;
}

// 保证最终价格在安全区间 (ensure safe bounds).
double PricingStrategy::clampPrice(double price, double basePrice) const {
    const double minPrice = basePrice * kMinPriceMultiplier;
    const double maxPrice = basePrice * kMaxPriceMultiplier;
    return std::clamp(price, minPrice, maxPrice);
}

}  // namespace pricing

#ifdef TEST_PRICING
#include <iostream>

int main() {
    pricing::Product phone{"p001", "NovaPhone 12", "smartphone", 6999.0, 320, false,
                           "NovaPhone"};
    std::tm now{};
    now.tm_hour = 14;
    pricing::MarketContext context{6499.0, 180.0, true, 1200, 260, 40, now, true};

    pricing::PricingStrategy strategy;
    const pricing::PricingResult result = strategy.calculatePrice(phone, context);

    std::cout << "Base price: " << phone.basePrice << "\nNew price: " << result.newPrice
              << "\nAdjustment: " << result.adjustment * 100 << "%\nExplanation: "
              << result.strategyExplanation << std::endl;

    return 0;
}
#endif  // TEST_PRICING



