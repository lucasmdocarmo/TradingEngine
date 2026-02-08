#pragma once
#include "OrderGateway.hpp"
#include <atomic>
#include <iostream>
#include <string>

namespace quant {

// Risk Manager
// In a real HFT system, this component is the "Circuit Breaker".
// It intercepts every order before it reaches the Exchange.
// Its job is to prevent catastrophic bugs (like an infinite loop sending
// orders) from bankrupting the firm. It must be EXTREMELY fast (nanoseconds).
class RiskManager {
public:
  RiskManager()
      : maxOrderSize_(10.0), maxPosition_(100.0), currentPosition_(0.0) {}

  // Check if an order is safe to send.
  // Returns true if safe, false if rejected.
  bool checkOrder(const std::string &symbol, Side side, double price,
                  double quantity) {
    // 1. Check Max Order Size
    // Prevent "Fat Finger" errors where an algorithm tries to buy 1000 BTC
    // instead of 1.
    if (quantity > maxOrderSize_) {
      std::cerr << "[RISK] REJECTED: Order size " << quantity
                << " exceeds limit " << maxOrderSize_ << std::endl;
      return false;
    }

    // 2. Check Max Position Limit
    // Prevent the algorithm from accumulating too much inventory.
    // If we are buying, adding this quantity must not exceed maxPosition_.
    // If we are selling, subtracting it (or going short) must not exceed
    // -maxPosition_.
    double projectedPosition = currentPosition_;
    if (side == Side::Buy) {
      projectedPosition += quantity;
    } else {
      projectedPosition -= quantity;
    }

    if (std::abs(projectedPosition) > maxPosition_) {
      std::cerr << "[RISK] REJECTED: Projected position " << projectedPosition
                << " exceeds limit " << maxPosition_ << std::endl;
      return false;
    }

    // 3. Price Reasonability Check (Optional but recommended)
    // Ensure we aren't buying 50% above the current market price due to bad
    // data. (Implementation omitted for brevity, requires passing in current
    // market price).

    return true;
  }

  // Update position when an order is filled (or assumed filled for this demo).
  // In a real system, this would be updated by the Order Manager upon receiving
  // an Execution Report from the exchange.
  void updatePosition(Side side, double quantity) {
    if (side == Side::Buy) {
      currentPosition_ += quantity;
    } else {
      currentPosition_ -= quantity;
    }
  }

private:
  double maxOrderSize_;
  double maxPosition_;
  std::atomic<double> currentPosition_; // Thread-safe position tracking
};

} // namespace quant
