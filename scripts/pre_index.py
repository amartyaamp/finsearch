import yfinance as yf
import pandas as pd
import os
import sys

# Add api to path so we can import the binding
sys.path.insert(0, os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "api"))
from fin_search_bind import FinSearchCore

def fetch_and_save_data(symbol, period, filename):
    print(f"Fetching {period} of data for {symbol}...")
    df = yf.download(symbol, period=period, progress=False)
    if df.empty:
        print(f"Failed to fetch data for {symbol}")
        return False
        
    if isinstance(df.columns, pd.MultiIndex):
        df.columns = df.columns.droplevel(1)
        
    df = df.reset_index()
    # Save with Date, Close, Volume for the C++ indexer
    # The C++ indexer expects Date, Close, Volume at minimum.
    df.to_csv(filename, index=False)
    print(f"Saved {len(df)} rows to {filename}")
    return True

def main():
    symbols = {
        "^NSEI": "data/nifty50_temp.csv",
        "^NSEBANK": "data/banknifty_temp.csv"
    }
    
    period = "5y"
    window_size = 60
    
    # Initialize Core
    try:
        core = FinSearchCore()
    except Exception as e:
        print(f"Failed to load FinSearchCore: {e}")
        print("Make sure you have compiled the C++ core library.")
        return
        
    # Fetch and index
    for symbol, csv_path in symbols.items():
        if fetch_and_save_data(symbol, period, csv_path):
            print(f"Indexing {symbol}...")
            ret = core.build_index_from_csv(csv_path, window_size)
            if ret < 0:
                print(f"Error indexing {symbol}: code {ret}")
            else:
                print(f"Successfully indexed {symbol}.")
            # Clean up temp file
            if os.path.exists(csv_path):
                os.remove(csv_path)
                
    # Save combined index
    output_path = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "nifty.faiss")
    print(f"Saving combined index to {output_path}...")
    ret = core.save_index(output_path)
    if ret < 0:
        print(f"Error saving index: code {ret}")
    else:
        print("Combined index saved successfully.")

if __name__ == "__main__":
    main()
