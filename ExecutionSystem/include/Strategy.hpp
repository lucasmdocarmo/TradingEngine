#pragma once
#include "LatencyMonitor.hpp"
#include "LockFreeQueue.hpp"
#include "Logger.hpp"
#include "MarketDataAdapter.hpp" // For BookTicker struct
#include "OrderBook.hpp"
#include "OrderGateway.hpp"
#include "OrderManager.hpp"
#include "RiskManager.hpp"
#include "SymbolManager.hpp"
#include <string>
#include <thread>
#include <unordered_map>

namespace quant {

// The Strategy Engine.
class Strategy {
public:
  Strategy(OrderGateway &gateway, OrderManager &orderManager,
           RiskManager &riskManager)
      : gateway_(gateway), orderManager_(orderManager),
        riskManager_(riskManager), latencyMonitor_("Strategy::onMarketData") {

    // Register symbols and get IDs
    btcUsdtId_ = SymbolManager::instance().getId("BTCUSDT");
    ethBtcId_ = SymbolManager::instance().getId("ETHBTC");
    ethUsdtId_ = SymbolManager::instance().getId("ETHUSDT");

    // Initialize books using SymbolId as key
    orderBooks_.emplace(btcUsdtId_, OrderBook("BTCUSDT"));
    orderBooks_.emplace(ethBtcId_, OrderBook("ETHBTC"));
    orderBooks_.emplace(ethUsdtId_, OrderBook("ETHUSDT"));
  }

  void run(LockFreeQueue<BookTicker, 1024> &queue) {
    Logger::instance().log("Strategy Engine started.");
    BookTicker ticker;

    while (running_) {
      while (queue.pop(ticker)) {
        latencyMonitor_.start();
        onMarketData(ticker);
        latencyMonitor_.stop();
      }
      std::this_thread::yield();
    }

    latencyMonitor_.report();
  }

  void stop() { running_ = false; }

private:
  void onMarketData(const BookTicker &ticker) {
    // Convert string symbol to ID (fast lookup if we cache it,
    // but here we might need to lookup if the ticker comes blindly)
    SymbolId symId = SymbolManager::instance().getId(ticker.symbol);

    auto it = orderBooks_.find(symId);
    if (it != orderBooks_.end()) {
      it->second.updateBid(ticker.best_bid_price, ticker.best_bid_qty);
      it->second.updateAsk(ticker.best_ask_price, ticker.best_ask_qty);
    } else {
      return;
    }

    // Check for Triangular Arbitrage Opportunity
    // We access books directly using stored IDs (Fast!)
    if (orderBooks_.count(btcUsdtId_) && orderBooks_.count(ethBtcId_) &&
        orderBooks_.count(ethUsdtId_)) {

      double btcUsdtAsk = orderBooks_.at(btcUsdtId_).getBestAsk();
      double ethBtcAsk = orderBooks_.at(ethBtcId_).getBestAsk();
      double ethUsdtBid = orderBooks_.at(ethUsdtId_).getBestBid();

      if (btcUsdtAsk > 0 && ethBtcAsk > 0 && ethUsdtBid > 0) {
        // Simple Profit Calc
        double amountUSDT = 100.0;
        double btcAmount = amountUSDT / btcUsdtAsk;
        double ethAmount = btcAmount / ethBtcAsk;
        double endUSDT = ethAmount * ethUsdtBid;

        double profit = endUSDT - amountUSDT;

        if (profit > 0.3) {
          Logger::instance().log("ARBITRAGE OPPORTUNITY FOUND! Profit: " +
                                 std::to_string(profit));
          executeArbitrage(btcUsdtAsk, ethBtcAsk, ethUsdtBid);
        }
      }
    }
  }

  void executeArbitrage(double price1, double price2, double price3) {
    Logger::instance().log(
        "Executing Arbitrage Strategy at prices: " + std::to_string(price1) +
        ", " + std::to_string(price2) + ", " + std::to_string(price3));

    double qty = 0.001;
    // Note: RiskManager still takes string, we should update it eventually.
    if (riskManager_.checkOrder("BTCUSDT", Side::Buy, price1, qty)) {

      // Use efficient Integer ID for Order creation
      long long id =
          orderManager_.createOrder(btcUsdtId_, Side::Buy, price1, qty);

      gateway_.sendOrder("BTCUSDT", Side::Buy, price1, qty, OrderType::Market);
      Logger::instance().log("Sent leg 1 order ID: " + std::to_string(id));

      // ... legs 2 and 3
    }
  }

  OrderGateway &gateway_;
  OrderManager &orderManager_;
  RiskManager &riskManager_;
  LatencyMonitor latencyMonitor_;

  // Pre-computed IDs for fast access
  SymbolId btcUsdtId_;
  SymbolId ethBtcId_;
  SymbolId ethUsdtId_;

  // Map of SymbolId -> OrderBook
  std::unordered_map<SymbolId, OrderBook> orderBooks_;

  std::atomic<bool> running_{true};
};

} // namespace quant
