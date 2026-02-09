#pragma once
#include "Types.hpp"
#include <chrono>
#include <functional>
#include <iostream>
#include <random>
#include <string>
#include <thread>

namespace quant {

// Forward DECLARATIONS only.
// Do not include ExecutionReport.hpp here to avoid circular dependency.
struct ExecutionReport;

// Interface for Order Entry Gateway
class OrderGateway {
public:
  // Define callback type: Takes a const reference to ExecutionReport
  using ExecCallback = std::function<void(const ExecutionReport &)>;

  OrderGateway() : engine_(std::random_device{}()) {}
  virtual ~OrderGateway() = default;

  // Set the callback for execution reports (fills/cancels)
  void setExecCallback(ExecCallback cb) { execCallback_ = cb; }

  // Send New Order Single (D)
  // Non-blocking: Sends request and returns immediately.
  // Execution Report (8) comes back asynchronously via callback.
  virtual void sendOrder(const std::string &symbol, Side side, double price,
                         double quantity, OrderType type, long long orderId);

  // Cancel an order
  virtual void cancelOrder(long long orderId) {
    std::cout << "[OEG] Cancelling Order: " << orderId << std::endl;
  }

protected:
  // Helper to trigger callback safely on a separate thread
  void triggerCallback(const ExecutionReport &report);

private:
  ExecCallback execCallback_;
  std::mt19937 engine_;
};

} // namespace quant
