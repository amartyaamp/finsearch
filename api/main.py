from fastapi import FastAPI, HTTPException
from pydantic import BaseModel
from typing import List, Optional
import os
import sys
import pandas as pd
import yfinance as yf# Insert current directory into path so it can import fin_search_bind directly
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fin_search_bind import FinSearchCore

app = FastAPI(title="FinSearch API", description="Time Series Pattern Matcher Service")

try:
    core = FinSearchCore()
except Exception as e:
    core = None
    print(f"Warning: Failed to load libfin_search_core: {e}")

class IndexRequest(BaseModel):
    csv_file_path: str
    window_size: int = 60

class SearchRequest(BaseModel):
    query_features: List[List[float]]
    k_neighbors: int = 5
    window_size: int = 60

@app.post("/index")
def build_index(req: IndexRequest):
    if not core:
        raise HTTPException(status_code=500, detail="C++ Core not loaded")
    if not os.path.exists(req.csv_file_path):
        raise HTTPException(status_code=400, detail="CSV file not found")
        
    ret = core.build_index_from_csv(req.csv_file_path, req.window_size)
    if ret < 0:
        raise HTTPException(status_code=500, detail=f"Failed to build index, error code: {ret}")
    return {"status": "success", "message": "Index built successfully"}

@app.post("/search")
def search_index(req: SearchRequest):
    if not core:
        raise HTTPException(status_code=500, detail="C++ Core not loaded")
    
    try:
        results = core.search(req.query_features, req.k_neighbors, req.window_size)
        return {"status": "success", "results": results}
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

class SymbolSearchRequest(BaseModel):
    symbol: str
    window_size: int = 60
    k_neighbors: int = 5

@app.post("/search/symbol")
def search_symbol(req: SymbolSearchRequest):
    if not core:
        raise HTTPException(status_code=500, detail="C++ Core not loaded")

    if req.window_size != 60:
        raise HTTPException(status_code=400, detail="Only window_size=60 is currently supported by the active index.")
    
    try:
        # Download 1 year of data to ensure we have enough trading days
        df = yf.download(req.symbol, period="1y", progress=False)
    except Exception as e:
        raise HTTPException(status_code=400, detail=f"Failed to fetch data for symbol {req.symbol}: {str(e)}")
        
    if df.empty or len(df) < req.window_size:
        raise HTTPException(status_code=404, detail=f"Not enough historical data found for {req.symbol}")
        
    # Extract the last window_size rows
    df_window = df.tail(req.window_size)
    
    try:
        # Squeeze in case of multi-index columns format from yfinance >= 0.2.x
        if isinstance(df_window.columns, pd.MultiIndex):
            df_window.columns = df_window.columns.droplevel(1)
            
        q_close = df_window["Close"].values.tolist()
        q_vol = df_window["Volume"].values.tolist()
    except KeyError as e:
        raise HTTPException(status_code=500, detail=f"Required target columns missing from yfinance data: {str(e)}")

    query_features = [q_close, q_vol]
    
    try:
        results = core.search(query_features, req.k_neighbors, req.window_size)
        return {"status": "success", "results": results}
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@app.get("/ohlcv")
def get_ohlcv(symbol: str, from_date: Optional[str] = None, to_date: Optional[str] = None):
    try:
        if from_date and to_date:
            df = yf.download(symbol, start=from_date, end=to_date, progress=False)
        else:
            df = yf.download(symbol, period="1y", progress=False)
            
        if df.empty:
            return []
            
        if isinstance(df.columns, pd.MultiIndex):
            df.columns = df.columns.droplevel(1)
            
        df = df.reset_index()
        
        records = []
        for _, row in df.iterrows():
            date_col = 'Date' if 'Date' in row else 'Datetime' if 'Datetime' in row else df.columns[0]
            
            date_val = row[date_col]
            if isinstance(date_val, pd.Timestamp):
                time_str = date_val.strftime('%Y-%m-%d')
            else:
                time_str = str(date_val).split(' ')[0]
                
            records.append({
                "time": time_str,
                "open": float(row.get("Open", 0.0)),
                "high": float(row.get("High", 0.0)),
                "low": float(row.get("Low", 0.0)),
                "close": float(row.get("Close", 0.0)),
                "volume": float(row.get("Volume", 0.0))
            })
            
        return records
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Failed to fetch OHLCV: {str(e)}")
