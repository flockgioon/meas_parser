# meas_parser

> **Status: Work In Progress**

一個用純 C 實作的二進位量測協定解析器 (binary measurement protocol parser)。可從串流資料中辨識封包邊界、驗證完整性、並將 payload 解碼為結構化的量測數據。

## Protocol Format

封包採用 big-endian 位元組序，結構如下：

| Offset | Size | Field |
|--------|------|-------|
| 0 | 2 | Sync bytes (`0xAA 0x55`) |
| 2 | 1 | Version |
| 3 | 1 | Message type |
| 4 | 2 | Sequence number |
| 6 | 2 | Payload length |
| 8 | N | Payload |
| 8+N | 2 | CRC-16 (CCITT-FALSE) |

### Message Types

- `0x01` — Measure
- `0x02` — Status
- `0x03` — ACK
- `0x04` — Error

### Measurement Payload

當 message type 為 Measure 時，payload 結構為：

| Offset | Size | Field |
|--------|------|-------|
| 0 | 4 | Timestamp (ms) |
| 4 | 1 | Channel count |
| 5 + i*6 | 1 | Channel ID |
| 5 + i*6 + 1 | 1 | Unit |
| 5 + i*6 + 2 | 4 | Value (mili, int32) |

支援的量測單位：Volt, Amp, Celsius。

## Modules

| Module | Description |
|--------|-------------|
| `big_endian` | Big-endian 讀寫工具函式 |
| `crc` | CRC-16/CCITT-FALSE 計算 |
| `mp_frame` | 從完整 buffer 解析單一封包 |
| `mp_parser` | 串流式解析器，處理封包邊界偵測與 resync |
| `mp_decode` | 將 frame payload 解碼為量測結構 |

## Build

需要 CMake >= 3.25 與支援 C11 的編譯器。

```bash
cmake -B build
cmake --build build
```

## Test

使用 CTest 執行測試：

```bash
cd build
ctest --output-on-failure
```

## TODO

- [ ] 完成 `main.c` 的實際應用邏輯
- [ ] 完成完整功能開發
