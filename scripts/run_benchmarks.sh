#!/bin/bash

set -e

echo "========================================"
echo " Linux Memory-Mapped File Benchmark"
echo "========================================"

mkdir -p data results

rm -f data/test_1MB.bin
rm -f data/test_10MB.bin
rm -f data/test_50MB.bin
rm -f results/benchmark.csv

echo
echo "[1/4] Creating test files..."

./mmfs create data/test_1MB.bin 1048576
./mmfs create data/test_10MB.bin 10485760
./mmfs create data/test_50MB.bin 52428800

echo
echo "[2/4] Running 1 MB benchmark..."
./mmfs benchmark data/test_1MB.bin results/benchmark.csv

echo
echo "[3/4] Running 10 MB benchmark..."
./mmfs benchmark data/test_10MB.bin results/benchmark.csv

echo
echo "[4/4] Running 50 MB benchmark..."
./mmfs benchmark data/test_50MB.bin results/benchmark.csv

echo
echo "========================================"
echo " Benchmark completed"
echo "========================================"

echo
echo "Results:"
cat results/benchmark.csv
