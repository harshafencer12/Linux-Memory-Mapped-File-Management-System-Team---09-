# Linux Memory-Mapped File Management System

## Team 9 — Section 11

A Linux-based systems programming project demonstrating memory-mapped file I/O using `mmap()`, `munmap()`, and `msync()`, with comparisons against traditional `read()`/`write()` based file I/O.

The project also demonstrates process concurrency using `fork()`, shared memory mappings, race conditions, POSIX semaphore synchronization, `/proc` process inspection, and `strace` system-call verification.

---

## 1. Objectives

The project implements and demonstrates:

- Memory-mapped file I/O using `mmap()`
- Unmapping using `munmap()`
- Synchronization using `msync()`
- `MAP_PRIVATE` and `MAP_SHARED`
- Traditional `read()` and `write()` file I/O
- Throughput benchmarking
- Minor and major page-fault measurement
- Performance testing with different file sizes
- Shared memory between processes using `fork()`
- Race-condition demonstration
- POSIX semaphore synchronization
- `/proc/[pid]/stat` process statistics
- `/proc/[pid]/maps` memory-map inspection
- `strace` system-call verification

---

## 2. Project Structure

```text
Linux-Memory-Mapped-File-Management-System/
│
├── Makefile
├── README.md
├── .gitignore
│
├── include/
│   ├── benchmark.h
│   ├── concurrency.h
│   ├── io_ops.h
│   ├── mmap_ops.h
│   └── proc_stats.h
│
├── src/
│   ├── main.c
│   ├── mmap_ops.c
│   ├── io_ops.c
│   ├── proc_stats.c
│   ├── benchmark.c
│   └── concurrency.c
│
├── data/
│   └── Test and demonstration files
│
├── results/
│   ├── benchmark.csv
│   ├── throughput_comparison.png
│   ├── page_faults_comparison.png
│   ├── strace_mmap.txt
│   ├── strace_read.txt
│   ├── strace_shared.txt
│   ├── strace_race.txt
│   └── strace_sync.txt
│
├── scripts/
│   ├── plot_results.py
│   └── run_benchmarks.sh
│
└── docs/
    └── report.md

3. Requirements

Software
Linux / Ubuntu / WSL
GCC
GNU Make
Python 3
Matplotlib
strace
Install required packages
sudo apt update
sudo apt install -y build-essential python3-matplotlib strace

4. Build

From the project root:

make

The executable is:

./mmfs

To clean the build:

make clean

5. Program Commands

Memory-mapped read

./mmfs map-read <file>

Example:

./mmfs map-read data/test_1MB.bin

This opens the file, maps it into virtual memory using mmap(), reads the mapped contents, and releases the mapping using munmap().

Memory-mapped write
./mmfs map-write <file> <text>

Example:

./mmfs map-write data/test_1MB.bin "Hello from mmap"
MAP_PRIVATE demonstration
./mmfs private <file>

Example:

./mmfs private data/test_1MB.bin

MAP_PRIVATE creates a private copy-on-write mapping. Modifications made through the mapping are not shared with other processes and do not become normal file modifications.

MAP_SHARED demonstration
./mmfs shared <file>

Example:

./mmfs shared data/test_1MB.bin

MAP_SHARED allows modifications to the mapped region to be shared with other processes and synchronized with the backing file.

The project uses:

msync(..., MS_SYNC)

to explicitly synchronize the shared mapping.

Traditional file read
./mmfs io-read <file>

Example:

./mmfs io-read data/test_1MB.bin

This uses traditional file-descriptor based read() operations.

Traditional file write
./mmfs io-write <file> <text>

Example:

./mmfs io-write data/test_1MB.bin "Hello from read/write"
6. Benchmarking

The benchmark compares traditional read() against memory-mapped I/O.

Three file sizes are tested:

1 MB
10 MB
50 MB

Run the complete benchmark:

./scripts/run_benchmarks.sh

The results are stored in:

results/benchmark.csv

Benchmark Results

| File Size |      read() |      mmap() | read() Minor Faults | mmap() Minor Faults |
| --------- | ----------: | ----------: | ------------------: | ------------------: |
| 1 MB      | 521.17 MB/s | 555.83 MB/s |                   4 |                  16 |
| 10 MB     | 546.09 MB/s | 586.12 MB/s |                   4 |                 160 |
| 50 MB     | 545.79 MB/s | 571.62 MB/s |                   4 |                 800 |

The benchmark also verifies that the processed data produces matching checksums for both methods.

7. Graphs

Generate the graphs with:

python3 scripts/plot_results.py

Generated files:

results/throughput_comparison.png
results/page_faults_comparison.png

The throughput graph compares the measured throughput of read() and mmap().

The page-fault graph uses a logarithmic Y-axis because the measured minor-fault values differ substantially between the two methods.

8. Process Inspection

Run:

./mmfs inspect

The program reads:

/proc/<pid>/stat
/proc/<pid>/maps

and displays:

Process ID
Minor page faults
Major page faults
Virtual memory mappings

Example output includes mappings for:

mmfs
[heap]
libc.so.6
[stack]
[vvar]
[vdso]
ld-linux-x86-64.so.2
9. Race Condition Demonstration

Run:

./mmfs race data/race.bin

Two child processes modify the same counter through a MAP_SHARED mapping without synchronization.

The program intentionally separates the read-modify-write operation to make the race condition observable.

Example result:

Expected counter: 200000
Actual counter:   100000

The lost updates occur because both processes can read the same old value before either process writes the incremented value.

10. Synchronization Demonstration

Run:

./mmfs sync data/sync.bin

A process-shared POSIX semaphore protects the critical section.

Example result:

Expected counter: 200000
Actual counter:   200000

Synchronization successful.
The semaphore protected the read-modify-write operation.

This demonstrates how synchronization prevents lost updates when multiple processes access shared memory concurrently.

11. strace Verification

The project uses strace to verify the system calls used by the implementation.

mmap()
strace -f -o results/strace_mmap.txt \
./mmfs map-read data/test_1MB.bin

Relevant calls include:

openat(...)
mmap(...)
munmap(...)
close(...)
Traditional read()
strace -f -o results/strace_read.txt \
./mmfs io-read data/test_1MB.bin

Relevant calls include:

openat(...)
read(...)
close(...)
MAP_SHARED and msync()
strace -f -o results/strace_shared.txt \
./mmfs shared data/test_1MB.bin

Relevant calls include:

openat(...)
mmap(... MAP_SHARED ...)
msync(...)
munmap(...)
close(...)
Race condition
strace -f -o results/strace_race.txt \
./mmfs race data/race.bin

The trace demonstrates process creation, shared mapping, waiting for child processes, and cleanup.

Synchronization
strace -f -o results/strace_sync.txt \
./mmfs sync data/sync.bin

The trace demonstrates process creation, shared memory mapping, waiting, and cleanup.

POSIX semaphore operations may be implemented partly in user space, so the presence of semaphore protection should primarily be verified from the program implementation and successful synchronized result rather than expecting a direct sem_wait() system call in strace.

12. Important Experimental Observation

In the tested environment, mmap() achieved higher measured throughput than traditional read() for all three tested file sizes.

However, this should not be interpreted as a universal performance guarantee.

File-system caching, operating-system state, storage hardware, CPU behavior, memory pressure, compiler settings, and workload characteristics can affect the measurements.

The benchmark therefore reports observations from the experimental environment rather than claiming that one technique is always faster.

13. Key Learning Outcomes

This project demonstrates the relationship between:

File
 ↓
File descriptor
 ↓
Virtual memory mapping
 ↓
mmap()
 ↓
Virtual address space
 ↓
Page faults
 ↓
Process memory

It also demonstrates that shared memory between processes requires synchronization when multiple processes modify the same data.

14. Team Contribution Area

The assigned contribution focuses on:

Traditional I/O baseline
Memory-mapped I/O performance comparison
Throughput measurement
Page-fault measurement
Multiple file-size experiments
Result collection
Graph generation
Performance analysis
15. Conclusion

The Linux Memory-Mapped File Management System successfully demonstrates memory-mapped file operations, traditional file I/O, process-shared mappings, synchronization, /proc inspection, and system-call tracing.

The experimental results show that memory-mapped I/O achieved slightly higher throughput than traditional read() in the tested environment, while producing more minor page faults.

The concurrency demonstrations further show why synchronization is necessary when multiple processes modify shared memory.

EOF


---

# 2. Replace the technical report

Now run:

```bash
cat > docs/report.md <<'EOF'
# Linux Memory-Mapped File Management System

## Team 9 — Section 11

---

# 1. Introduction

Memory-mapped file I/O is a Linux mechanism that maps a file into the virtual address space of a process. Instead of repeatedly transferring data through explicit `read()` and `write()` system calls, a process can access the mapped file through normal memory operations.

This project implements a Linux-based memory-mapped file management system using:

- `mmap()`
- `munmap()`
- `msync()`
- `MAP_PRIVATE`
- `MAP_SHARED`

The project also compares memory-mapped I/O with traditional file I/O and investigates the effects of memory mapping using Linux `/proc` information and `strace`.

A concurrency component demonstrates how shared memory can produce race conditions and how POSIX synchronization can prevent them.

---

# 2. Objectives

The major objectives are:

1. Implement memory-mapped file input/output.
2. Demonstrate `MAP_PRIVATE` and `MAP_SHARED`.
3. Demonstrate `msync()` for synchronization of shared mappings.
4. Implement traditional `read()`/`write()` file operations.
5. Compare memory-mapped I/O with traditional I/O.
6. Measure throughput for different file sizes.
7. Measure minor and major page faults.
8. Inspect process memory mappings through `/proc`.
9. Demonstrate concurrency using `fork()`.
10. Demonstrate a race condition using shared memory.
11. Correct the race condition using POSIX synchronization.
12. Verify system calls using `strace`.

---

# 3. System Architecture

The project is organized into separate modules.

```text
                    Linux Memory-Mapped File
                              |
             +----------------+----------------+
             |                                 |
       Memory-Mapped I/O                 Traditional I/O
             |                                 |
     mmap / munmap / msync              read / write
             |                                 |
             +----------------+----------------+
                              |
                       Benchmark Module
                              |
             +----------------+----------------+
             |                                 |
        Throughput                     Page Faults
             |                                 |
             +----------------+----------------+
                              |
                       Results / Graphs
                              |
             +----------------+----------------+
             |                                 |
           /proc                            strace
             |                                 |
       Process statistics               System calls
       Memory mappings
                              |
                       Concurrency Module
                              |
                   +----------+----------+
                   |                     |
              Race Condition       Synchronization
                   |                     |
             MAP_SHARED             Semaphore



4. Source Code Organization
src/mmap_ops.c

Implements memory-mapped file operations.

Important functions include:

File mapping
Mapped file reading
Mapped file writing
MAP_PRIVATE demonstration
MAP_SHARED demonstration
msync()
Memory-mapped file copying
src/io_ops.c

Implements traditional file I/O using:

open()
read()
write()
close()

This provides the baseline used for comparison against memory-mapped I/O.

src/proc_stats.c

Reads Linux process information from:

/proc/<pid>/stat
/proc/<pid>/maps

The implementation extracts:

PID
Minor page faults
Major page faults
Process memory mappings
src/benchmark.c

Performs performance measurements for:

read()
mmap()

The benchmark processes every byte of the test file so that both approaches perform an equivalent data-processing workload.

It records:

File size
Method
Execution time
Throughput
Minor page faults
Major page faults

A checksum is also calculated to verify that both methods process the same data.

src/concurrency.c

Implements two process-concurrency demonstrations.

Race condition

Two child processes modify a shared counter through a MAP_SHARED mapping without synchronization.

Synchronized access

Two child processes modify the same shared counter while using a process-shared POSIX semaphore.

5. Memory-Mapped File I/O

The basic mapping operation follows:

open()
   |
   v
mmap()
   |
   v
Access mapped memory
   |
   v
munmap()
   |
   v
close()

For a read-only mapping, the file is opened with read permission and mapped using:

mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);

The returned pointer represents the beginning of the mapped region.

The process can then access the file contents using normal memory accesses.

6. MAP_PRIVATE

MAP_PRIVATE creates a private copy-on-write mapping.

A modification made through a private mapping does not become a normal shared modification of the underlying file.

This mode is useful when a process needs file-backed data but must isolate its modifications from other processes.

The project provides:

./mmfs private data/test_1MB.bin

to demonstrate this behavior.

7. MAP_SHARED

MAP_SHARED creates a mapping whose modifications can be shared with other processes and synchronized with the underlying file.

The project provides:

./mmfs shared data/test_1MB.bin

The implementation uses:

msync(..., MS_SYNC);

to explicitly synchronize the modified mapping.

The corresponding strace output confirms the use of:

openat()
mmap()
msync()
munmap()
close()
8. Traditional File I/O

Traditional file I/O uses explicit system calls.

The general operation is:

open()
   |
   v
read()/write()
   |
   v
close()

The project provides:

./mmfs io-read data/test_1MB.bin

and:

./mmfs io-write data/test_1MB.bin "Example text"

This implementation provides the baseline for comparison with memory-mapped I/O.

9. Performance Benchmark Methodology

Three file sizes were tested:

1 MB
10 MB
50 MB

The test files were generated by the project.

The benchmark then executed both:

Traditional read()
Memory-mapped access

for each file.

The benchmark processes every byte for both methods and calculates a checksum.

This prevents an unfair comparison in which one method processes less data than the other.

The measurements include:

Execution time
Throughput
Minor page faults
Major page faults

10. Throughput Results

The measured results were:

| File Size |      read() |      mmap() |
| --------- | ----------: | ----------: |
| 1 MB      | 521.17 MB/s | 555.83 MB/s |
| 10 MB     | 546.09 MB/s | 586.12 MB/s |
| 50 MB     | 545.79 MB/s | 571.62 MB/s |

The graph is stored at:

results/throughput_comparison.png
Observation

In the experimental environment, memory-mapped I/O achieved higher measured throughput than traditional read() for all three tested file sizes.

The difference was:

approximately 34.66 MB/s for 1 MB
approximately 40.03 MB/s for 10 MB
approximately 25.83 MB/s for 50 MB

These values describe this particular experiment and should not be treated as universal performance guarantees.


11. Page-Fault Results

Measured minor page faults were:

File Size	read()	mmap()
1 MB	4	16
10 MB	4	160
50 MB	4	800

Major page faults were:

0

for all recorded benchmark cases.

The graph is stored at:

results/page_faults_comparison.png

The graph uses a logarithmic Y-axis because the mmap page-fault values are substantially larger than the read values.

Observation

The memory-mapped implementation produced more minor page faults.

The number increased with file size:

1 MB  -> 16
10 MB -> 160
50 MB -> 800

Traditional read() recorded 4 minor faults for each of these benchmark runs.

This demonstrates that memory-mapped access interacts directly with the process virtual-memory system and page-fault mechanism.

12. Page Fault Concepts

A page fault occurs when a process accesses a virtual-memory page that is not currently available in the required state.

Minor page faults do not require disk I/O.

Major page faults require disk I/O.

The benchmark recorded:

Major faults = 0

for the tested cases.

The observed minor faults are therefore the primary difference in the recorded page-fault measurements.

13. /proc Process Inspection

The project provides:

./mmfs inspect

The command reads:

/proc/<pid>/stat
/proc/<pid>/maps

An observed execution produced:

PID           : 1248
Minor faults  : 83
Major faults  : 0

The /proc/<pid>/maps output showed mappings belonging to:

mmfs
[heap]
libc.so.6
[vvar]
[vvar_vclock]
[vdso]
ld-linux-x86-64.so.2
[stack]

This demonstrates that the program can inspect the virtual address space of the running process.

14. Race Condition

The project demonstrates a race condition using:

fork()
MAP_SHARED

Two child processes modify a common counter.

The increment operation is intentionally separated into:

read counter
        |
        v
yield CPU
        |
        v
write counter + 1

This creates a larger window in which both processes can access the same old value.

An observed execution produced:

Expected counter: 200000
Actual counter:   100000

The difference occurs because updates can be lost when both processes read the same counter value before either writes the new value.

Other executions also produced values below the expected 200000, demonstrating the nondeterministic nature of the race.

15. Synchronization

The race condition is corrected using a process-shared POSIX semaphore.

The critical section becomes:

sem_wait()
     |
     v
read-modify-write
     |
     v
sem_post()

An observed execution produced:

Expected counter: 200000
Actual counter:   200000

This demonstrates that the semaphore protects the complete read-modify-write operation and prevents the lost updates observed in the unsynchronized version.

16. strace Verification

strace was used to verify system calls.

Memory-mapped read

Relevant trace:

openat(AT_FDCWD, "data/test_1MB.bin", O_RDONLY) = 3
mmap(NULL, 1048576, PROT_READ, MAP_PRIVATE, 3, 0) = ...
munmap(..., 1048576) = 0
close(3) = 0

This confirms the use of memory mapping instead of repeated file reads.

Traditional read

The trace contained:

openat(..., "data/test_1MB.bin", O_RDONLY) = 3
read(3, ..., 8192) = 8192
...
read(3, "", 8192) = 0
close(3) = 0

This confirms traditional descriptor-based reading.

The program also writes file contents to standard output, so write(1, ...) calls in the trace correspond to terminal output rather than the file-input mechanism.

MAP_SHARED

The trace contained:

openat(AT_FDCWD, "data/test_1MB.bin", O_RDWR) = 3
mmap(NULL, 1048576, PROT_READ|PROT_WRITE, MAP_SHARED, 3, 0) = ...
msync(..., 1048576, MS_SYNC) = 0
munmap(..., 1048576) = 0
close(3) = 0

This directly verifies:

MAP_SHARED
msync()
munmap()
17. Fork and Shared-Memory Concurrency

The concurrency traces showed process creation through Linux's process-creation mechanism.

The trace contains calls of the form:

clone(...)

Modern Linux implementations may show clone() in strace for programs using fork() because the C library implements process creation using Linux's clone-based process-creation mechanism.

The traces also showed:

mmap(...)
wait4(...)
munmap(...)
close(...)

This confirms the creation of child processes, shared mappings, waiting for child completion, and cleanup.

18. Synchronization and strace

The synchronized program uses a POSIX semaphore in shared memory.

The important evidence for synchronization is:

The source code initializes a process-shared semaphore.
Child processes use the semaphore around the critical section.
The expected value of 200000 is consistently achieved in the synchronized demonstration.

strace does not necessarily show direct sem_wait() or sem_post() calls because POSIX semaphore operations can be implemented partly in user space and may use lower-level synchronization mechanisms only when required.

Therefore, successful synchronization is verified through both source-code inspection and program output.

19. Comparison
Memory-Mapped I/O

Advantages demonstrated by the project:

Direct access through virtual memory
Reduced need for explicit read calls
Convenient random access
Shared mappings can support inter-process data sharing
Competitive measured throughput in the experiment

Observed characteristic:

Higher number of minor page faults in the benchmark
Traditional I/O

Advantages:

Simple and familiar interface
Explicit control over reads and writes
Predictable descriptor-based I/O model

Observed characteristic:

Lower recorded minor page-fault count in the benchmark
Slightly lower measured throughput than mmap in the tested cases
20. Limitations

The benchmark has several limitations.

20.1 Environment dependency

Performance can vary depending on:

Linux version
WSL environment
CPU
storage device
filesystem
system load
filesystem cache state
memory pressure
20.2 Small number of test sizes

Only three file sizes were tested:

1 MB
10 MB
50 MB

A larger range of file sizes and repeated trials would provide a more comprehensive performance evaluation.

20.3 Benchmark scope

The quantitative benchmark in the current implementation compares:

read()
vs
mmap()

The project also implements traditional and memory-mapped write operations, but the recorded benchmark CSV focuses on the read comparison.

20.4 Page-fault measurement

Page-fault counts are process-level measurements and include effects
from the benchmark process and its execution environment. 
They should therefore be interpreted as observed measurements rather than 
perfectly isolated costs of a single instruction.


21. Results Summary

The completed implementation demonstrates all major functional components:

| Requirement           | Status       |
| --------------------- | ------------ |
| `mmap()`              | Implemented  |
| `munmap()`            | Implemented  |
| `msync()`             | Implemented  |
| `MAP_PRIVATE`         | Implemented  |
| `MAP_SHARED`          | Implemented  |
| Traditional `read()`  | Implemented  |
| Traditional `write()` | Implemented  |
| Throughput benchmark  | Implemented  |
| Multiple file sizes   | Implemented  |
| Minor page faults     | Measured     |
| Major page faults     | Measured     |
| `/proc/<pid>/stat`    | Implemented  |
| `/proc/<pid>/maps`    | Implemented  |
| `fork()` concurrency  | Implemented  |
| Race condition        | Demonstrated |
| POSIX semaphore       | Implemented  |
| `strace` verification | Completed    |
| Result graphs         | Generated    |

22. Conclusion

The Linux Memory-Mapped File Management System demonstrates how Linux virtual memory can be used for file I/O through memory mapping.

The implementation provides both traditional file operations and memory-mapped operations, allowing their behavior and performance to be compared.

For the tested 1 MB, 10 MB, and 50 MB files, memory-mapped I/O produced higher measured throughput than traditional read(). At the same time, memory-mapped access produced substantially more minor page faults.

The project also demonstrates that MAP_SHARED can be used for inter-process shared data, but synchronization is required when multiple processes modify the same data. The race-condition experiment produced lost updates, while the semaphore-protected implementation achieved the expected result.

Finally, /proc inspection and strace provide operating-system-level evidence of process memory mappings, page faults, memory mapping calls, file operations, process creation, and synchronization-related behavior.

The project therefore connects file I/O, virtual memory, process management, synchronization, and Linux system-call tracing in a single practical implementation.

EOF


