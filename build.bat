@echo off
echo ========================================
echo  Building Elastic Sketch Project...
echo ========================================

echo.
echo [1/3] Compiling Main Executable (elastic_sketch.exe)...
gcc -Wall -O2 -std=c99 -o elastic_sketch.exe src/heavy_guardian.c src/count_min_sketch.c src/elastic_sketch.c src/traffic_gen.c src/main.c -lm
if %errorlevel% neq 0 (
    echo [ERROR] Failed to build main executable.
    exit /b %errorlevel%
)

echo.
echo [2/3] Compiling Benchmark Suite (benchmark.exe)...
gcc -Wall -O2 -std=c99 -o benchmark.exe src/heavy_guardian.c src/count_min_sketch.c src/elastic_sketch.c src/traffic_gen.c tests/benchmark.c -lm
if %errorlevel% neq 0 (
    echo [ERROR] Failed to build benchmark.
    exit /b %errorlevel%
)

echo.
echo [3/3] Compiling Shared Library (elastic_sketch.dll)...
gcc -shared -Wall -O2 -std=c99 -o elastic_sketch.dll src/heavy_guardian.c src/count_min_sketch.c src/elastic_sketch.c src/traffic_gen.c elastic_sketch_api.c
if %errorlevel% neq 0 (
    echo [ERROR] Failed to build DLL.
    exit /b %errorlevel%
)

echo.
echo ========================================
echo  Build Successful! ALL SYSTEMS GO.
echo ========================================
echo.
echo Commands to run your project:
echo  - .\elastic_sketch.exe    (To run the C simulation)
echo  - .\benchmark.exe         (To test performance)
echo  - streamlit run app.py    (To launch the UI)
echo.