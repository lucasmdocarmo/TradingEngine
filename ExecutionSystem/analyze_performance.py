
import re
import sys

def parse_log(filename):
    """
    Parses the C++ Execution Engine log file to extract Trade executions and PnL.
    """
    cumulative_pnl = 0.0
    trades = []
    
    # Regex to capture profit from logs
    arb_regex = re.compile(r"ARBITRAGE OPPORTUNITY FOUND! Profit: ([\d\.]+)")
    
    try:
        with open(filename, 'r') as f:
            for line in f:
                # Capture Arbitrage Profit
                match = arb_regex.search(line)
                if match:
                    profit = float(match.group(1))
                    cumulative_pnl += profit
                    trades.append(cumulative_pnl)
                    
    except FileNotFoundError:
        print(f"Error: Log file {filename} not found.")
        return []

    return trades

def print_ascii_chart(trades):
    if not trades:
        return

    print("\n--- Cumulative PnL Chart (ASCII) ---")
    max_pnl = max(trades)
    min_pnl = min(trades)
    range_pnl = max_pnl - min_pnl if max_pnl != min_pnl else 1.0
    
    height = 10
    
    for val in trades:
        normalized = int((val - min_pnl) / range_pnl * height)
        bar = "#" * (normalized + 1)
        print(f"{val:8.4f} | {bar}")

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 analyze_performance.py <log_file>")
        return

    log_file = sys.argv[1]
    trades = parse_log(log_file)
    
    if trades:
        print(f"Total Trades: {len(trades)}")
        print(f"Final PnL:    {trades[-1]:.4f} USDT")
        print_ascii_chart(trades)
    else:
        print("No arbitrage trades found in log.")

if __name__ == "__main__":
    main()
