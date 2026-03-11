from fastapi import FastAPI, HTTPException
from pydantic import BaseModel
from typing import List
import os
import sys

# Insert current directory into path so it can import fin_search_bind directly
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
