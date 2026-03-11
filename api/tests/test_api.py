from fastapi.testclient import TestClient
import os
import sys
import tempfile
import csv

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
        resp = client.post("/index", json={"csv_file_path": "/invalid/path.csv", "window_size": 10})
        assert resp.status_code == 400

        # Valid Indexing
        resp = client.post("/index", json={"csv_file_path": csv_path, "window_size": 10})
        assert resp.status_code == 200

        # Valid Search (Close and Volume features)
        q_close = [100.0] * 10
        q_vol = [1000.0] * 10
        resp = client.post("/search", json={
            "query_features": [q_close, q_vol],
            "k_neighbors": 2,
            "window_size": 10
        })
        assert resp.status_code == 200
        data = resp.json()
        assert "results" in data
        assert len(data["results"]) > 0

    finally:
        os.remove(csv_path)
