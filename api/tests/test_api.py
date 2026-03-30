from fastapi.testclient import TestClient
import os
import sys
import tempfile
import csv
from unittest.mock import patch
import pandas as pd

# Add the parent directory to sys.path to import api.main
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from main import app

client = TestClient(app)

def test_index_and_search_endpoints():
    from main import core
    if not core:
        import pytest
        pytest.skip("C++ Core not loaded, skipping integration tests.")

    with tempfile.NamedTemporaryFile(mode='w', delete=False, suffix=".csv") as f:
        writer = csv.writer(f)
        writer.writerow(["Date", "Open", "High", "Low", "Close", "Volume"])
        for i in range(100):
            writer.writerow([f"2023-01-{i%28 + 1:02d}", 100, 105, 95, 100 + i, 1000])
        csv_path = f.name

    try:
        # Invalid file
        resp = client.post("/index", json={"source_path": "/invalid/path.csv", "window_size": 10})
        assert resp.status_code == 400

        # Valid Indexing (using window_size=60 for compatibility with search/symbol)
        resp = client.post("/index", json={"source_path": csv_path, "window_size": 60})
        assert resp.status_code == 200

        # Valid Search (Close and Volume features)
        q_close = [100.0] * 60
        q_vol = [1000.0] * 60
        resp = client.post("/search", json={
            "query_features": [q_close, q_vol],
            "k_neighbors": 2,
            "window_size": 60
        })
        assert resp.status_code == 200
        data = resp.json()
        assert "results" in data
        assert len(data["results"]) > 0

    finally:
        os.remove(csv_path)

def test_search_symbol_and_ohlcv():
    from main import core
    if not core:
        import pytest
        pytest.skip("C++ Core not loaded, skipping integration tests.")
        
    dates = pd.date_range("2023-01-01", periods=100)
    df = pd.DataFrame({
        "Open": [100.0] * 100,
        "High": [105.0] * 100,
        "Low": [95.0] * 100,
        "Close": [100.0] * 100,
        "Volume": [1000.0] * 100
    }, index=dates)

    with patch("main.yf.download") as mock_download:
        mock_download.return_value = df
        
        # Test /search/symbol
        resp = client.post("/search/symbol", json={
            "symbol": "RELIANCE.NS",
            "window_size": 60,
            "k_neighbors": 2
        })
        assert resp.status_code == 200
        data = resp.json()
        assert "results" in data
        
        # Test /ohlcv
        resp = client.get("/ohlcv?symbol=RELIANCE.NS")
        assert resp.status_code == 200
        ohlcv_data = resp.json()
        assert isinstance(ohlcv_data, list)
        assert len(ohlcv_data) == 100
        assert "time" in ohlcv_data[0]
        assert "close" in ohlcv_data[0]

def test_input_validation():
    # Test /index
    resp = client.post("/index", json={"source_path": "dummy.csv", "window_size": -5})
    assert resp.status_code == 422
    
    resp = client.post("/index", json={"source_path": "dummy.csv", "window_size": 150})
    assert resp.status_code == 422

    # Test /search
    resp = client.post("/search", json={
        "query_features": [[1.0]*10],
        "k_neighbors": 5,
        "window_size": 60
    })
    assert resp.status_code == 422

    # Test /search/symbol
    resp = client.post("/search/symbol", json={
        "symbol": "AAPL",
        "window_size": -1,
        "k_neighbors": 5
    })
    assert resp.status_code == 422

    # Test /ohlcv
    resp = client.get("/ohlcv?symbol=AAPL&from_date=invalid-date")
    assert resp.status_code == 422

    resp = client.get("/ohlcv?symbol=AAPL&from_date=2023-12-31&to_date=2023-01-01")
    assert resp.status_code == 400

