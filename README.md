# blueare

**blueare** is a Redis-inspired in-memory data store written from scratch in modern C++ (C++17).

## 🚀 Features

- In-memory key-value storage  
- Intrusive data structures for memory-efficient, zero-indirection containers  
  - Chaining hashtable with progressive (non-blocking) rehashing  
  - AVL tree for balanced, ordered indexing  
- Sorted-set (ZSET) support with O(log N) rank and range queries  
- Single-threaded, event-loop networking  
  - Non-blocking sockets + `poll()` readiness API  
  - Custom request/response framing (binary, length-prefixed)  
  - Pipelined request support  
- Built-in CLI client (`client`) & server (`server`) executables  
- Automated tests for data structures (AVL, offset, commands)

## 📦 Commands

Once the `server` is running, use the `client` to issue commands:

- **SET** `` ``  
- **GET** ``  
- **DEL** ``  
- **ZADD** `` `` ``  
- **ZSCORE** `` ``  
- **ZREM** `` ``  
- **ZQUERY** `` `` `` `` ``

Example:
```
$ ./client SET foo bar
(int) 1
$ ./client GET foo
(str) bar
$ ./client ZADD myz 1.5 alice
(int) 1
$ ./client ZADD myz 2.0 bob
(int) 1
$ ./client ZQUERY myz 1.0 "" 0 10
(arr) len=4
(str) alice
(dbl) 1.5
(str) bob
(dbl) 2
(arr) end
```

## 🏗️ Architecture

- **Networking**  
  - Linux TCP sockets, `socket()`, `bind()`, `listen()`, `accept()`  
  - Non-blocking I/O via `fcntl(..., O_NONBLOCK)`  
  - Single thread `poll()`-based event loop  

- **Intrusive Data Structures**  
  - **Hashtable** (`hashtable.{h,cpp}`)  
    - `HMap` with two `HTab` instances for progressive rehash  
    - `HNode` embedded in payload for O(1) lookups/deletes  
  - **AVL Tree** (`avl.{h,cpp}`)  
    - `AVLNode` stores parent/child pointers, height & subtree size  
    - `avl_fix()`, `avl_del()`, `avl_offset()` for balanced insert/delete and rank-based traversal  
  - **ZSet** (`zset.{h,cpp}`)  
    - Combines `HMap` (by member name) + AVL tree (by score,name)  
    - Supports fast point/range/rank queries  

- **Serialization & Protocol**  
  - Binary TLV-style messages (Tag + Length + Value)  
  - 4-byte message-length prefix + payload  

## 🔨 Build & Run

```
git clone https://github.com/PranavGaur7/blueare.git
cd blueare
mkdir build
cd build
cmake ..
make
# start server:
./server
# in another shell, run client:
./client SET key value

# or if it not works then
./bin/server
# in another shell, run client:
./bin/client SET key value
```

## ✅ Tests

- `test_avl.cpp` — AVL insert/delete/balance  
- `test_offset.cpp` — rank/offset traversal  
- `test_cmds.py` — end-to-end command tests via `client`

Run individual tests with your C++ test runner or Python:

```
g++ -std=c++17 test_avl.cpp avl.cpp -o test_avl && ./test_avl
g++ -std=c++17 test_offset.cpp avl.cpp -o test_offset && ./test_offset
./test_cmds.py
```

