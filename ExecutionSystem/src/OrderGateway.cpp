#include "OrderGateway.hpp"
#include "ExecutionReport.hpp"

namespace quant {

void OrderGateway::sendOrder(const std::string &symbol, Side side, double price,
                             double quantity, OrderType type,
                             long long orderId) {
  // 1. Simulate Network Latency + Matching Engine Processing (Async)
  // We launch a thread to simulate the delay and response.
  std::thread([this, symbol, side, price, quantity, orderId]() {
    // Random delay: 5ms to 50ms
    std::uniform_int_distribution<> delayDist(5, 50);
    std::this_thread::sleep_for(std::chrono::milliseconds(delayDist(engine_)));

    // 3. Generate Simulated Execution Report
    // Assume immediate fill for this demo (Market Maker scenario)
    if (execCallback_) {
      ExecutionReport report;
      report.orderId = orderId;
      report.symbol = symbol;
      report.side = side;
      report.lastQty = quantity;
      report.lastPrice = price; // Assume filled at limit price
      report.leavesQty = 0;
      report.cumQty = quantity;
      report.avgPrice = price;

      // First: Pending New (ack)
      // Then: New (open)
      // Then: Fill (trade)

      // Simplifying: Jump straight to Fill for demo speed
      report.execType = ExecType::Fill; // Fully Filled
      report.orderState = OrderState::Filled;
      report.text = "Simulated Fill";

      execCallback_(report);
    }
  }).detach();
}

void OrderGateway::triggerCallback(const ExecutionReport &report) {
  if (execCallback_) {
    execCallback_(report);
  }
}

} // namespace quant
