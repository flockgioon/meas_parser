# meas_parser

純 C 實作的二進位量測協定解析器。從串流資料中偵測封包邊界、驗證 CRC 完整性、將 payload 解碼為結構化量測數據。支援檔案讀取與 TCP 即時接收兩種模式。

跨平台：Linux / macOS / Windows 皆可編譯與執行。

## Quick Start

需要 CMake >= 3.25 與支援 C11 的編譯器 (GCC, Clang, MSVC 皆可)。

```sh
cmake -S . -B build
cmake --build build
cd build && ctest --output-on-failure
```

### 從檔案解析

先用 simulator 產生測試用的 binary 檔，再餵給 parser：

```sh
cd build
./simu            # 產生 test_data.bin
./meas_parser test_data.bin
```

### 從 TCP 即時接收

開兩個 terminal，一邊跑 TCP simulator (server)，一邊跑 parser (client)：

```sh
# terminal 1 — server，監聽 port 9000 並送出模擬封包
cd build && ./tcp_simu

# terminal 2 — client，連線後即時解析
cd build && ./meas_parser --tcp 127.0.0.1 9000
```

## Protocol Format

封包採用 big-endian 位元組序：

```
+--------+--------+---------+----------+-----+------------+---------+-----+
| SYNC 0 | SYNC 1 | VERSION | MSG_TYPE | SEQ | PAYLOAD_LEN| PAYLOAD | CRC |
|  0xAA  |  0x55  |  1 byte | 1 byte   | 2B  |    2B      |  N bytes| 2B  |
+--------+--------+---------+----------+-----+------------+---------+-----+
```

- CRC：CRC-16/CCITT-FALSE，計算範圍為 VERSION 到 PAYLOAD 結尾
- Message types：Measure (`0x01`), Status (`0x02`), ACK (`0x03`), Error (`0x04`)

### Measurement Payload

| Offset | Size | Field |
|--------|------|-------|
| 0 | 4 | Timestamp (ms) |
| 4 | 1 | Channel count |
| 5 + i\*6 | 1 | Channel ID |
| 5 + i\*6 + 1 | 1 | Unit (Volt / Amp / Celsius) |
| 5 + i\*6 + 2 | 4 | Value (milli, signed int32) |

## Architecture

```
raw bytes ──► mp_parser (stream, resync) ──► mp_frame (validate) ──► mp_decode (structured data)
```

| Module | 職責 |
|--------|------|
| `big_endian` | Big-endian 讀寫工具 |
| `crc` | CRC-16/CCITT-FALSE |
| `mp_frame` | 從完整 buffer 解析並驗證單一封包 |
| `mp_parser` | 串流式解析：封包邊界偵測、自動 resync |
| `mp_decode` | 將 frame payload 解碼為 `mp_measurement_t` |
| `mp_log` | 分級 log (DEBUG / INFO / WARN / ERROR) |
| `mp_socket` | 跨平台 socket 抽象 (header-only) |
| `portable` | 跨平台 `fopen` 封裝 |
| `build_helper` | 封包建構工具，供 simulator 和測試使用 |

## Project Structure

```
include/          header files
src/              core library + main entry point
tests/            unit tests (CTest)
tools/            instrument_simulator (file) + tcp_simulator (TCP)
CMakeLists.txt
```

## Tests

共 5 組 unit test，涵蓋各模組：

| Test | 涵蓋範圍 |
|------|----------|
| `test_big_endian` | big-endian 讀寫正確性 |
| `test_crc` | CRC-16 計算結果驗證 |
| `test_mp_frame` | 封包解析：正常封包、錯誤 CRC、版本錯誤、長度異常 |
| `test_mp_parser` | 串流解析：resync、跨 chunk 封包、連續封包 |
| `test_mp_decode` | Measurement payload 解碼、邊界條件 |

## Cross-Platform

| | Linux / macOS | Windows |
|---|---|---|
| 編譯器 | GCC / Clang | MSVC / Clang |
| Warning flags | `-Wall -Wextra -Wpedantic -Werror -Wshadow -Wconversion -Wmissing-prototypes` | `/W4 /WX` |
| Socket | POSIX socket | Winsock2 (`ws2_32`) |
| 差異處理 | `mp_socket.h` 透過 `#ifdef _WIN32` 統一 API | 同左 |

CMake 的 generator expression 與 `if(WIN32)` 自動處理，build 指令三個平台完全相同。
