#!/bin/bash

echo "1. 啟動 BT Burst (50ms)..."
./pta_app bt burst &  # & 代表背景執行

echo "2. 立刻嘗試 Wi-Fi (應該失敗)..."
./pta_app wifi 5

echo "3. 等待 0.1 秒..."
sleep 0.1

echo "4. 再次嘗試 Wi-Fi (應該成功)..."
./pta_app wifi 5
