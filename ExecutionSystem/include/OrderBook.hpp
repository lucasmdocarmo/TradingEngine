#pragma once
#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace quant {

// Represents a single level in the order book
struct PriceLevel {
  double price;
  double quantity;
};

// Represents the full state of the order book for a symbol.
// In a real HFT system, we would use flat arrays or custom hash maps
// instead of std::map for cache locality (O(1) vs O(log n)), but
// std::map is sufficient for correctness and easier to implement for display.
class OrderBook {
public:
  OrderBook(const std::string &symbol) : symbol_(symbol) {}

  // Update the book with a new bid (buy order)
  void updateBid(double price, double quantity) {
    if (quantity == 0) {
      bids_.erase(price); // Remove level if quantity is 0
    } else {
      bids_[price] = quantity; // Insert or update level
    }
  }

  // Update the book with a new ask (sell order)
  void updateAsk(double price, double quantity) {
    if (quantity == 0) {
      asks_.erase(price);
    } else {
      asks_[price] = quantity;
    }
  }

  // Get the best bid (highest price someone is willing to buy at)
  // std::map is sorted by key (price). For bids, we want the highest key.
  // rbegin() gives us the last element (highest price).
  double getBestBid() const {
    if (bids_.empty())
      return 0.0;
    return bids_.rbegin()->first;
  }

  // Get the best ask (lowest price someone is willing to sell at)
  // std::map is sorted by key (price). For asks, we want the lowest key.
  // begin() gives us the first element (lowest price).
  double getBestAsk() const {
    if (asks_.empty())
      return 0.0;
    return asks_.begin()->first;
  }

  // Calculate the Mid Price: (Best Bid + Best Ask) / 2
  double getMidPrice() const {
    double bb = getBestBid();
    double ba = getBestAsk();
    if (bb == 0 || ba == 0)
      return 0.0;
    return (bb + ba) / 2.0;
  }

  // Print the top levels of the book for visualization
  void print() const {
    std::cout << "Order Book [" << symbol_ << "]" << std::endl;

    // Show top 3 Asks (lowest prices first)
    std::cout << "  ASKS:" << std::endl;
    int count = 0;
    for (auto it = asks_.begin(); it != asks_.end() && count < 3;
         ++it, ++count) {
      std::cout << "    " << it->first << " x " << it->second << std::endl;
    }

    std::cout << "  -----------------" << std::endl;

    // Show top 3 Bids (highest prices first)
    std::cout << "  BIDS:" << std::endl;
    count = 0;
    for (auto it = bids_.rbegin(); it != bids_.rend() && count < 3;
         ++it, ++count) {
      std::cout << "    " << it->first << " x " << it->second << std::endl;
    }
    std::cout << std::endl;
  }

private:
  std::string symbol_;

  // Bids: Sorted Ascending by Price.
  // We want the highest price, so we use rbegin().
  std::map<double, double> bids_;

  // Asks: Sorted Ascending by Price.
  // We want the lowest price, so we use begin().
  std::map<double, double> asks_;
};

} // namespace quant
