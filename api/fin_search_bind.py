import ctypes
from ctypes import c_void_p, c_int, c_float, c_size_t, POINTER, c_char_p, Structure
import os
import sys

# Define the C struct for mapping
class CSearchResult(Structure):
    _fields_ = [
        ("distance", c_float),
        ("confidence_score", c_float),
        ("index", ctypes.c_long),
        ("start_date", c_char_p),
        ("end_date", c_char_p)
    ]

class FinSearchCore:
    def __init__(self, lib_path: str = None):
        if not lib_path:
            # Default to checking common locations
            if sys.platform == 'darwin':
                lib_name = "libfin_search_core.dylib"
            else:
                lib_name = "libfin_search_core.so"
            
            # Look in parent dir build, install directories
            base_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
            paths = [
                os.path.join(base_dir, "build", lib_name),
                os.path.join(base_dir, "libfin_search_core.so"), 
                lib_name
            ]
            
            for p in paths:
                if os.path.exists(p):
                    lib_path = p
                    break
            if not lib_path:
                lib_path = lib_name # Fallback to system search path

        self.lib = ctypes.CDLL(lib_path)
        self._setup_argtypes()
        self.handle = self.lib.fin_search_service_create()

    def _setup_argtypes(self):
        self.lib.fin_search_service_create.restype = c_void_p
        
        self.lib.fin_search_service_destroy.argtypes = [c_void_p]
        
        self.lib.fin_search_service_build_index_from_csv.argtypes = [c_void_p, c_char_p, c_int]
        self.lib.fin_search_service_build_index_from_csv.restype = c_int

        self.lib.fin_search_service_save_index.argtypes = [c_void_p, c_char_p]
        self.lib.fin_search_service_save_index.restype = c_int

        self.lib.fin_search_service_load_index.argtypes = [c_void_p, c_char_p, c_int]
        self.lib.fin_search_service_load_index.restype = c_int

        self.lib.fin_search_service_search.argtypes = [
            c_void_p, 
            POINTER(POINTER(c_float)), 
            c_size_t, 
            c_size_t, 
            c_int, 
            POINTER(c_size_t)
        ]
        self.lib.fin_search_service_search.restype = POINTER(CSearchResult)

        self.lib.fin_search_free_results.argtypes = [POINTER(CSearchResult), c_size_t]

    def __del__(self):
        if hasattr(self, 'handle') and self.handle:
            self.lib.fin_search_service_destroy(self.handle)

    def build_index_from_csv(self, csv_file_path: str, window_size: int) -> int:
        return self.lib.fin_search_service_build_index_from_csv(
            self.handle, csv_file_path.encode('utf-8'), window_size
        )

    def save_index(self, index_path: str) -> int:
        return self.lib.fin_search_service_save_index(
            self.handle, index_path.encode('utf-8')
        )

    def load_index(self, index_path: str, window_size: int) -> int:
        return self.lib.fin_search_service_load_index(
            self.handle, index_path.encode('utf-8'), window_size
        )

    def search(self, query_features: list[list[float]], k_neighbors: int, window_size: int):
        num_features = len(query_features)
        
        # Convert python lists to C arrays of pointers
        FloatArray = c_float * window_size
        c_arrays = []
        for feature in query_features:
            if len(feature) != window_size:
                raise ValueError(f"Feature size {len(feature)} does not match window_size {window_size}")
            c_arrays.append(FloatArray(*feature))
            
        float_ptr_array = (POINTER(c_float) * num_features)()
        for i in range(num_features):
            float_ptr_array[i] = ctypes.cast(c_arrays[i], POINTER(c_float))

        out_count = c_size_t(0)
        
        res_ptr = self.lib.fin_search_service_search(
            self.handle, float_ptr_array, num_features, window_size, k_neighbors, ctypes.byref(out_count)
        )

        n_results = out_count.value
        results = []
        if n_results > 0 and res_ptr:
            for i in range(n_results):
                c_res = res_ptr[i]
                results.append({
                    "distance": c_res.distance,
                    "confidence_score": c_res.confidence_score,
                    "index": c_res.index,
                    "start_date": c_res.start_date.decode('utf-8') if c_res.start_date else "",
                    "end_date": c_res.end_date.decode('utf-8') if c_res.end_date else ""
                })
            # Free C allocated memory
            self.lib.fin_search_free_results(res_ptr, n_results)

        return results
