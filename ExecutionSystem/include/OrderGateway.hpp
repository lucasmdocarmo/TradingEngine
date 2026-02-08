#pragma once
#include <iostream>
#include <string>

namespace quant {

// Enum to represent order types
enum class OrderType { Limit, Market };

// Enum to represent order side
enum class Side { Buy, Sell };

// Interface for an Order Entry Gateway.
// In a real system, this would handle TCP connections to the exchange's
// order entry port (e.g., FIX protocol or REST API).
class OrderGateway {
public:
  virtual ~OrderGateway() = default;

  // Send a new order implementation
  virtual void sendOrder(const std::string &symbol, Side side, double price,
                         double quantity, OrderType type) {
    // In a real implementation:
    // 1. Construct the JSON payload or FIX message.
    // 2. Sign the request with HMAC-SHA256 using the API Secret.
    // 3. Send via HTTPS/TCP.

    // For simulation/display, we just print the action.
    std::cout << "[OEG] Sending Order: "
              << (side == Side::Buy ? "BUY " : "SELL ") << quantity << " of "
              << symbol << " @ " << price << std::endl;
  }

  // Cancel an order
  virtual void cancelOrder(long long orderId) {
    std::cout << "[OEG] Cancelling Order: " << orderId << std::endl;
  }
};

} // namespace quant
