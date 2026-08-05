# CPU Scheduling Simulator

A command-line CPU Scheduling Simulator implemented in C++ that demonstrates and compares common CPU scheduling algorithms used in Operating Systems.

The simulator supports FCFS, Shortest Job First, Priority Scheduling, and Round Robin while calculating important scheduling metrics such as waiting time, turnaround time, response time, and completion time.

It also generates a process execution trace and a Gantt chart to visualize CPU allocation.

---

## Features

- First Come First Serve (FCFS)
- Shortest Job First (SJF) - Non-Preemptive
- Priority Scheduling - Non-Preemptive
- Round Robin - Preemptive
- Arrival Time support
- CPU idle-time handling
- Configurable Time Quantum for Round Robin
- Process execution trace
- Gantt chart visualization
- Completion Time calculation
- Turnaround Time calculation
- Waiting Time calculation
- Response Time calculation
- Average scheduling metrics
- Menu-driven command-line interface

---

## Scheduling Metrics

The simulator calculates the following metrics for every process.

### Completion Time (CT)

The time at which a process finishes its execution.

### Turnaround Time (TAT)

TAT = CT - AT

### Waiting Time (WT)

WT = TAT - BT

### Response Time (RT)

RT = First CPU Start Time - AT

Where:

- AT = Arrival Time
- BT = Burst Time
- CT = Completion Time

---

## Scheduling Algorithms

### 1. First Come First Serve (FCFS)

FCFS executes processes according to their arrival order.

It is a non-preemptive scheduling algorithm.

### 2. Shortest Job First (SJF)

The simulator implements non-preemptive SJF.

Among all processes that have arrived, the process with the smallest burst time is selected.

### 3. Priority Scheduling

The simulator implements non-preemptive Priority Scheduling.

Among available processes, the process with the highest priority is selected.

In this implementation:

Lower priority number = Higher priority

For example:

Priority 1 > Priority 2 > Priority 3

### 4. Round Robin

Round Robin is a preemptive scheduling algorithm.

Each process receives the CPU for a fixed amount of time called the Time Quantum.

If the process does not finish during its quantum, it is placed back into the ready queue.

---

## Project Architecture

```text
                 CPU Scheduling Simulator
                          |
                          v
                    Process Input
                          |
                          v
                 Algorithm Selection
                          |
          +---------------+---------------+
          |               |               |
        FCFS             SJF          Priority
          |               |               |
          +---------------+---------------+
                          |
                     Round Robin
                          |
                          v
                    CPU Execution
                          |
                  +-------+-------+
                  |               |
               Process           IDLE
               Execution          CPU
                  |               |
                  +-------+-------+
                          |
                          v
                   Execution Trace
                          |
              +-----------+-----------+
              |                       |
              v                       v
       Scheduling Metrics         Gantt Chart
              |
       +------+------+------+
       |      |      |      |
       CT    TAT     WT     RT
```

---

## Data Structures Used

### Vector

`std::vector` is used to store and manage process information.

### Queue

`std::queue` is used to implement the ready queue in Round Robin scheduling.

Processes are removed from the front of the queue and unfinished processes are inserted again at the rear.

### Execution Structure

An execution structure stores CPU execution intervals.

```cpp
struct Execution
{
    int pid;
    int startTime;
    int endTime;
};
```

This information is used to generate the execution trace and Gantt chart.

---

## Example Input

```text
Number of Processes: 4

Process    Arrival Time    Burst Time    Priority

P1              0              5             2
P2              1              3             1
P3              2              8             3
P4              3              6             2
```

---

## Algorithm Selection

```text
SELECT SCHEDULING ALGORITHM

1. First Come First Serve (FCFS)
2. Shortest Job First (SJF)
3. Priority Scheduling
4. Round Robin
5. Exit
```

---

## Example Round Robin Gantt Chart

For a time quantum of 2:

```text
+---------+---------+---------+---------+---------+
|P1       |P2       |P3       |P1       |P4       |
+---------+---------+---------+---------+---------+
0         2         4         6         8         10
```

A process may appear multiple times because Round Robin is preemptive.

---

## CPU Idle Handling

The simulator also handles situations where no process is available for execution.

Example:

```text
0 -> 2  : IDLE
2 -> 5  : P1
5 -> 7  : P2
7 -> 8  : IDLE
8 -> 10 : P3
```

The idle intervals are also represented in the Gantt chart.

---

## Time Complexity

| Algorithm | Implementation Complexity |
|-----------|---------------------------|
| FCFS | O(n log n) |
| SJF | O(n²) |
| Priority | O(n²) |
| Round Robin | Depends on number of time slices |

FCFS includes sorting processes according to arrival time.

SJF and Priority Scheduling repeatedly search available processes.

Round Robin uses a queue for ready-process management.

---

## Compilation

Using g++:

```bash
g++ main.cpp -o scheduler
```

On Windows:

```powershell
g++ main.cpp -o scheduler.exe
```

---

## Run

Linux/macOS:

```bash
./scheduler
```

Windows PowerShell:

```powershell
.\scheduler.exe
```

---

## Technologies

- C++
- C++ STL
- Operating Systems
- Data Structures and Algorithms

---

## Concepts Demonstrated

- CPU Scheduling
- Preemptive Scheduling
- Non-Preemptive Scheduling
- Ready Queue
- Process Arrival
- CPU Idle Time
- Process Execution
- Waiting Time
- Turnaround Time
- Response Time
- STL Vector
- STL Queue
- Sorting and Comparators
- Algorithm Complexity

---

## Limitations

The current implementation focuses on core CPU scheduling concepts.

It does not currently simulate:

- Context-switching overhead
- Preemptive Priority Scheduling
- Shortest Remaining Time First (SRTF)
- Aging
- Multicore CPU scheduling
- I/O burst behavior

---

## Future Improvements

Possible extensions include:

- SRTF scheduling
- Preemptive Priority Scheduling
- Aging to prevent starvation
- Context-switch overhead simulation
- Comparative performance analysis
- CPU utilization calculation
- Throughput calculation
- Multicore scheduling simulation
- Graphical visualization

---

## Author

Developed as an Operating Systems and C++ project to demonstrate CPU scheduling algorithms and their performance characteristics.
