from fastapi import FastAPI, HTTPException, Query
from pydantic import BaseModel, Field, model_validator
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
    source_path: str = Field(..., min_length=1, description="Path to the source data file (e.g., CSV)")
    window_size: int = Field(60, ge=1, le=100)

class SearchRequest(BaseModel):
    query_features: List[List[float]] = Field(..., description="List of feature sequences (e.g., [close_prices, volumes])")
    k_neighbors: int = Field(5, ge=1, le=100)
    window_size: int = Field(60, ge=1, le=100)

    @model_validator(mode='after')
    def validate_features(self) -> 'SearchRequest':
        if not self.query_features:
            raise ValueError('query_features cannot be empty')
        for idx, feature_list in enumerate(self.query_features):
            if len(feature_list) != self.window_size:
                raise ValueError(f'Feature list at index {idx} has length {len(feature_list)}, expected window_size {self.window_size}')
        return self

@app.post("/index")
def build_index(req: IndexRequest):
    if not core:
        raise HTTPException(status_code=500, detail="C++ Core not loaded")
    if not os.path.exists(req.source_path):
        raise HTTPException(status_code=400, detail="Source file not found")
        
    ret = core.build_index_from_csv(req.source_path, req.window_size)
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
    symbol: str = Field(..., min_length=1)
    window_size: int = Field(60, ge=1, le=100)
    k_neighbors: int = Field(5, ge=1, le=100)

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
def get_ohlcv(
    symbol: str = Query(..., min_length=1),
    from_date: Optional[str] = Query(None, pattern=r"^\d{4}-\d{2}-\d{2}$"),
    to_date: Optional[str] = Query(None, pattern=r"^\d{4}-\d{2}-\d{2}$")
):
    if from_date and to_date and from_date > to_date:
        raise HTTPException(status_code=400, detail="from_date must be less than or equal to to_date")
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
