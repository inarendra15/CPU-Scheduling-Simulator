#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <iomanip>
#include <climits>
#include <string>

using namespace std;


// ============================================================
// PROCESS STRUCTURE
// ============================================================

struct Process
{
    int pid;

    int arrivalTime;
    int burstTime;
    int priority;

    // Calculated scheduling metrics
    int completionTime = 0;
    int turnaroundTime = 0;
    int waitingTime = 0;
    int responseTime = 0;

    // Used mainly by Round Robin
    int remainingTime = 0;

    // Used by SJF / Priority
    bool completed = false;

    // Used to determine first CPU allocation
    bool started = false;
};


// ============================================================
// EXECUTION STRUCTURE
// ============================================================
// Stores one CPU execution interval.
//
// Example:
// P1 executes from time 2 to 5
//
// pid       = 1
// startTime = 2
// endTime   = 5
//
// pid = -1 represents CPU IDLE.
// ============================================================

struct Execution
{
    int pid;
    int startTime;
    int endTime;
};


// ============================================================
// RESET PROCESS STATE
// ============================================================
// Important when the same process data is used by different
// scheduling algorithms.
// ============================================================

void resetProcesses(vector<Process>& processes)
{
    for (Process& p : processes)
    {
        p.completionTime = 0;
        p.turnaroundTime = 0;
        p.waitingTime = 0;
        p.responseTime = 0;

        p.remainingTime = p.burstTime;

        p.completed = false;
        p.started = false;
    }
}


// ============================================================
// ADD EXECUTION TO GANTT CHART
// ============================================================
// Consecutive identical intervals are merged.
//
// Example:
//
// P1 0-2
// P1 2-4
//
// becomes:
//
// P1 0-4
//
// This keeps the Gantt chart clean.
// ============================================================

void addExecution(
    vector<Execution>& ganttChart,
    int pid,
    int startTime,
    int endTime)
{
    if (startTime >= endTime)
        return;

    if (!ganttChart.empty() &&
        ganttChart.back().pid == pid &&
        ganttChart.back().endTime == startTime)
    {
        ganttChart.back().endTime = endTime;
    }
    else
    {
        ganttChart.push_back(
            {pid, startTime, endTime}
        );
    }
}


// ============================================================
// FCFS
// First Come First Serve
// ============================================================

void fcfs(
    vector<Process>& processes,
    vector<Execution>& ganttChart)
{
    resetProcesses(processes);

    ganttChart.clear();

    // Sort according to arrival time.
    // PID is used as tie breaker.
    stable_sort(
        processes.begin(),
        processes.end(),
        [](const Process& a, const Process& b)
        {
            if (a.arrivalTime == b.arrivalTime)
                return a.pid < b.pid;

            return a.arrivalTime < b.arrivalTime;
        }
    );


    int currentTime = 0;


    for (Process& p : processes)
    {
        // ----------------------------------------------------
        // CPU may be idle before this process arrives.
        // ----------------------------------------------------

        if (currentTime < p.arrivalTime)
        {
            addExecution(
                ganttChart,
                -1,
                currentTime,
                p.arrivalTime
            );

            currentTime = p.arrivalTime;
        }


        // First time process gets CPU.
        p.responseTime =
            currentTime - p.arrivalTime;


        int startTime = currentTime;

        currentTime += p.burstTime;


        addExecution(
            ganttChart,
            p.pid,
            startTime,
            currentTime
        );


        // Calculate metrics

        p.completionTime = currentTime;

        p.turnaroundTime =
            p.completionTime -
            p.arrivalTime;

        p.waitingTime =
            p.turnaroundTime -
            p.burstTime;

        p.completed = true;
        p.started = true;
    }
}


// ============================================================
// NON-PREEMPTIVE SJF
// Shortest Job First
// ============================================================

void sjf(
    vector<Process>& processes,
    vector<Execution>& ganttChart)
{
    resetProcesses(processes);

    ganttChart.clear();


    int n = processes.size();

    int completedCount = 0;
    int currentTime = 0;


    while (completedCount < n)
    {
        int selected = -1;

        int minimumBurst = INT_MAX;


        // ----------------------------------------------------
        // Find shortest available process
        // ----------------------------------------------------

        for (int i = 0; i < n; i++)
        {
            if (!processes[i].completed &&
                processes[i].arrivalTime <= currentTime)
            {
                if (processes[i].burstTime < minimumBurst)
                {
                    minimumBurst =
                        processes[i].burstTime;

                    selected = i;
                }

                // Tie breaker
                else if (
                    processes[i].burstTime == minimumBurst &&
                    selected != -1)
                {
                    if (processes[i].arrivalTime <
                        processes[selected].arrivalTime)
                    {
                        selected = i;
                    }

                    else if (
                        processes[i].arrivalTime ==
                        processes[selected].arrivalTime &&
                        processes[i].pid <
                        processes[selected].pid)
                    {
                        selected = i;
                    }
                }
            }
        }


        // ----------------------------------------------------
        // No process available -> CPU IDLE
        // ----------------------------------------------------

        if (selected == -1)
        {
            int nextArrival = INT_MAX;

            for (int i = 0; i < n; i++)
            {
                if (!processes[i].completed)
                {
                    nextArrival =
                        min(
                            nextArrival,
                            processes[i].arrivalTime
                        );
                }
            }


            addExecution(
                ganttChart,
                -1,
                currentTime,
                nextArrival
            );

            currentTime = nextArrival;

            continue;
        }


        Process& p = processes[selected];


        // Response time
        p.responseTime =
            currentTime -
            p.arrivalTime;


        int startTime = currentTime;

        currentTime += p.burstTime;


        addExecution(
            ganttChart,
            p.pid,
            startTime,
            currentTime
        );


        // Metrics

        p.completionTime = currentTime;

        p.turnaroundTime =
            p.completionTime -
            p.arrivalTime;

        p.waitingTime =
            p.turnaroundTime -
            p.burstTime;


        p.completed = true;
        p.started = true;

        completedCount++;
    }
}


// ============================================================
// NON-PREEMPTIVE PRIORITY SCHEDULING
// ============================================================
//
// IMPORTANT:
//
// Smaller priority number = higher priority.
//
// Priority 1 > Priority 2 > Priority 3
//
// ============================================================

void priorityScheduling(
    vector<Process>& processes,
    vector<Execution>& ganttChart)
{
    resetProcesses(processes);

    ganttChart.clear();


    int n = processes.size();

    int completedCount = 0;
    int currentTime = 0;


    while (completedCount < n)
    {
        int selected = -1;

        int highestPriority = INT_MAX;


        // ----------------------------------------------------
        // Find highest-priority available process
        // ----------------------------------------------------

        for (int i = 0; i < n; i++)
        {
            if (!processes[i].completed &&
                processes[i].arrivalTime <= currentTime)
            {
                if (processes[i].priority < highestPriority)
                {
                    highestPriority =
                        processes[i].priority;

                    selected = i;
                }

                // Tie breaker
                else if (
                    processes[i].priority == highestPriority &&
                    selected != -1)
                {
                    if (processes[i].arrivalTime <
                        processes[selected].arrivalTime)
                    {
                        selected = i;
                    }

                    else if (
                        processes[i].arrivalTime ==
                        processes[selected].arrivalTime &&
                        processes[i].pid <
                        processes[selected].pid)
                    {
                        selected = i;
                    }
                }
            }
        }


        // ----------------------------------------------------
        // CPU idle
        // ----------------------------------------------------

        if (selected == -1)
        {
            int nextArrival = INT_MAX;


            for (int i = 0; i < n; i++)
            {
                if (!processes[i].completed)
                {
                    nextArrival =
                        min(
                            nextArrival,
                            processes[i].arrivalTime
                        );
                }
            }


            addExecution(
                ganttChart,
                -1,
                currentTime,
                nextArrival
            );


            currentTime = nextArrival;

            continue;
        }


        Process& p = processes[selected];


        // Response Time
        p.responseTime =
            currentTime -
            p.arrivalTime;


        int startTime = currentTime;

        currentTime += p.burstTime;


        addExecution(
            ganttChart,
            p.pid,
            startTime,
            currentTime
        );


        // Metrics

        p.completionTime = currentTime;

        p.turnaroundTime =
            p.completionTime -
            p.arrivalTime;

        p.waitingTime =
            p.turnaroundTime -
            p.burstTime;


        p.completed = true;
        p.started = true;

        completedCount++;
    }
}


// ============================================================
// ROUND ROBIN
// ============================================================

void roundRobin(
    vector<Process>& processes,
    int timeQuantum,
    vector<Execution>& ganttChart)
{
    resetProcesses(processes);

    ganttChart.clear();


    int n = processes.size();


    // --------------------------------------------------------
    // Store indices according to arrival order.
    // This allows us to preserve the original process vector.
    // --------------------------------------------------------

    vector<int> order(n);

    for (int i = 0; i < n; i++)
    {
        order[i] = i;
    }


    stable_sort(
        order.begin(),
        order.end(),
        [&](int a, int b)
        {
            if (processes[a].arrivalTime ==
                processes[b].arrivalTime)
            {
                return processes[a].pid <
                       processes[b].pid;
            }

            return processes[a].arrivalTime <
                   processes[b].arrivalTime;
        }
    );


    queue<int> readyQueue;


    int currentTime = 0;
    int completedCount = 0;

    int nextProcess = 0;


    // --------------------------------------------------------
    // If first process doesn't arrive at time 0
    // --------------------------------------------------------

    if (n > 0 &&
        processes[order[0]].arrivalTime > currentTime)
    {
        int nextArrival =
            processes[order[0]].arrivalTime;


        addExecution(
            ganttChart,
            -1,
            currentTime,
            nextArrival
        );


        currentTime = nextArrival;
    }


    // Add initially arrived processes

    while (
        nextProcess < n &&
        processes[order[nextProcess]].arrivalTime <= currentTime)
    {
        readyQueue.push(order[nextProcess]);

        nextProcess++;
    }


    // --------------------------------------------------------
    // Main Round Robin loop
    // --------------------------------------------------------

    while (completedCount < n)
    {
        // ----------------------------------------------------
        // Ready queue empty -> CPU idle
        // ----------------------------------------------------

        if (readyQueue.empty())
        {
            if (nextProcess < n)
            {
                int nextArrival =
                    processes[
                        order[nextProcess]
                    ].arrivalTime;


                addExecution(
                    ganttChart,
                    -1,
                    currentTime,
                    nextArrival
                );


                currentTime = nextArrival;


                while (
                    nextProcess < n &&
                    processes[
                        order[nextProcess]
                    ].arrivalTime <= currentTime)
                {
                    readyQueue.push(
                        order[nextProcess]
                    );

                    nextProcess++;
                }
            }

            continue;
        }


        // Get process from front
        int index = readyQueue.front();

        readyQueue.pop();


        Process& p = processes[index];


        // ----------------------------------------------------
        // First CPU allocation -> calculate response time
        // ----------------------------------------------------

        if (!p.started)
        {
            p.responseTime =
                currentTime -
                p.arrivalTime;

            p.started = true;
        }


        int startTime = currentTime;


        // CPU execution duration

        int executionTime =
            min(
                timeQuantum,
                p.remainingTime
            );


        currentTime += executionTime;

        p.remainingTime -= executionTime;


        addExecution(
            ganttChart,
            p.pid,
            startTime,
            currentTime
        );


        // ----------------------------------------------------
        // Add processes that arrived while current process ran
        // ----------------------------------------------------

        while (
            nextProcess < n &&
            processes[
                order[nextProcess]
            ].arrivalTime <= currentTime)
        {
            readyQueue.push(
                order[nextProcess]
            );

            nextProcess++;
        }


        // ----------------------------------------------------
        // Process completed
        // ----------------------------------------------------

        if (p.remainingTime == 0)
        {
            p.completionTime =
                currentTime;

            p.turnaroundTime =
                p.completionTime -
                p.arrivalTime;

            p.waitingTime =
                p.turnaroundTime -
                p.burstTime;

            p.completed = true;

            completedCount++;
        }

        // ----------------------------------------------------
        // Process still has CPU burst remaining
        // Put it at back of ready queue.
        // ----------------------------------------------------

        else
        {
            readyQueue.push(index);
        }
    }
}


// ============================================================
// DISPLAY RESULTS
// ============================================================

void displayResults(
    const vector<Process>& processes,
    const string& algorithmName)
{
    cout << "\n\n"
         << algorithmName
         << " Scheduling Results\n\n";


    cout
        << left
        << setw(8)  << "PID"
        << setw(8)  << "AT"
        << setw(8)  << "BT"
        << setw(8)  << "CT"
        << setw(8)  << "TAT"
        << setw(8)  << "WT"
        << setw(8)  << "RT"
        << "\n";


    cout
        << "--------------------------------------------------------\n";


    double totalWaiting = 0;
    double totalTurnaround = 0;
    double totalResponse = 0;


    // --------------------------------------------------------
    // Display in PID order
    // --------------------------------------------------------

    vector<Process> sortedProcesses = processes;


    sort(
        sortedProcesses.begin(),
        sortedProcesses.end(),
        [](const Process& a, const Process& b)
        {
            return a.pid < b.pid;
        }
    );


    for (const Process& p : sortedProcesses)
    {
        cout
            << left
            << setw(8)
            << ("P" + to_string(p.pid))

            << setw(8)
            << p.arrivalTime

            << setw(8)
            << p.burstTime

            << setw(8)
            << p.completionTime

            << setw(8)
            << p.turnaroundTime

            << setw(8)
            << p.waitingTime

            << setw(8)
            << p.responseTime

            << "\n";


        totalWaiting +=
            p.waitingTime;

        totalTurnaround +=
            p.turnaroundTime;

        totalResponse +=
            p.responseTime;
    }


    int n = processes.size();


    cout << fixed << setprecision(2);


    cout
        << "\nAverage Waiting Time    : "
        << totalWaiting / n;


    cout
        << "\nAverage Turnaround Time : "
        << totalTurnaround / n;


    cout
        << "\nAverage Response Time   : "
        << totalResponse / n
        << "\n";
}


// ============================================================
// DISPLAY PROCESS EXECUTION
// ============================================================
//
// This is useful both for debugging and interview explanation.
//
// Example:
//
// 0 -> 2 : IDLE
// 2 -> 5 : P1
// 5 -> 7 : P2
//
// ============================================================

void displayExecution(
    const vector<Execution>& ganttChart)
{
    cout << "\nProcess Execution\n";
    cout << "-----------------------------\n";


    for (const Execution& e : ganttChart)
    {
        cout
            << setw(4)
            << e.startTime

            << " -> "

            << setw(4)
            << e.endTime

            << " : ";


        if (e.pid == -1)
        {
            cout << "IDLE";
        }
        else
        {
            cout << "P" << e.pid;
        }


        cout << "\n";
    }
}


// ============================================================
// DISPLAY GANTT CHART
// ============================================================

void displayGanttChart(
    const vector<Execution>& ganttChart)
{
    if (ganttChart.empty())
    {
        cout << "\nNo Gantt chart data available.\n";
        return;
    }


    cout << "\nGantt Chart\n\n";


    const int width = 10;


    // --------------------------------------------------------
    // Top border
    // --------------------------------------------------------

    for (size_t i = 0;
         i < ganttChart.size();
         i++)
    {
        cout << "+---------";
    }

    cout << "+\n";


    // --------------------------------------------------------
    // Process names
    // --------------------------------------------------------

    for (const Execution& e : ganttChart)
    {
        string label;


        if (e.pid == -1)
        {
            label = "IDLE";
        }
        else
        {
            label =
                "P" +
                to_string(e.pid);
        }


        cout
            << "|"
            << left
            << setw(width - 1)
            << label;
    }


    cout << "|\n";


    // --------------------------------------------------------
    // Bottom border
    // --------------------------------------------------------

    for (size_t i = 0;
         i < ganttChart.size();
         i++)
    {
        cout << "+---------";
    }

    cout << "+\n";


    // --------------------------------------------------------
    // Timeline
    // --------------------------------------------------------

    cout << left;

    cout
        << setw(width)
        << ganttChart[0].startTime;


    for (size_t i = 0;
         i < ganttChart.size();
         i++)
    {
        if (i ==
            ganttChart.size() - 1)
        {
            cout
                << ganttChart[i].endTime;
        }
        else
        {
            cout
                << setw(width)
                << ganttChart[i].endTime;
        }
    }


    cout << "\n";
}


// ============================================================
// MAIN
// ============================================================

int main()
{
    cout
        << "============================================\n";

    cout
        << "          CPU SCHEDULING SIMULATOR\n";

    cout
        << "============================================\n";


    int n;


    cout
        << "\nEnter number of processes: ";

    cin >> n;


    if (n <= 0)
    {
        cout
            << "\nNumber of processes must be greater than 0.\n";

        return 0;
    }


    vector<Process> processes(n);


    // ========================================================
    // PROCESS INPUT
    // ========================================================

    cout
        << "\nEnter process information\n";

    cout
        << "--------------------------------------------\n";


    for (int i = 0; i < n; i++)
    {
        processes[i].pid =
            i + 1;


        cout
            << "\nProcess P"
            << processes[i].pid
            << "\n";


        cout
            << "Arrival Time : ";

        cin
            >> processes[i].arrivalTime;


        cout
            << "Burst Time   : ";

        cin
            >> processes[i].burstTime;


        cout
            << "Priority     : ";

        cin
            >> processes[i].priority;


        // Input validation

        if (processes[i].arrivalTime < 0)
        {
            cout
                << "\nArrival time cannot be negative.\n";

            return 0;
        }


        if (processes[i].burstTime <= 0)
        {
            cout
                << "\nBurst time must be greater than 0.\n";

            return 0;
        }


        processes[i].remainingTime =
            processes[i].burstTime;
    }


    // Keep original process information.
    //
    // Every algorithm receives a fresh copy.
    // This prevents one algorithm from modifying
    // another algorithm's results.

    vector<Process> originalProcesses =
        processes;


    // ========================================================
    // MENU LOOP
    // ========================================================

    while (true)
    {
        cout
            << "\n\n============================================\n";

        cout
            << "        SELECT SCHEDULING ALGORITHM\n";

        cout
            << "============================================\n";


        cout
            << "1. First Come First Serve (FCFS)\n";

        cout
            << "2. Shortest Job First (SJF)\n";

        cout
            << "3. Priority Scheduling\n";

        cout
            << "4. Round Robin\n";

        cout
            << "5. Exit\n";


        int choice;


        cout
            << "\nEnter your choice: ";

        cin >> choice;


        // Fresh process copy for each run

        vector<Process> currentProcesses =
            originalProcesses;


        vector<Execution> ganttChart;


        // ====================================================
        // FCFS
        // ====================================================

        if (choice == 1)
        {
            fcfs(
                currentProcesses,
                ganttChart
            );


            displayResults(
                currentProcesses,
                "FCFS"
            );


            displayExecution(
                ganttChart
            );


            displayGanttChart(
                ganttChart
            );
        }


        // ====================================================
        // SJF
        // ====================================================

        else if (choice == 2)
        {
            sjf(
                currentProcesses,
                ganttChart
            );


            displayResults(
                currentProcesses,
                "SJF"
            );


            displayExecution(
                ganttChart
            );


            displayGanttChart(
                ganttChart
            );
        }


        // ====================================================
        // PRIORITY
        // ====================================================

        else if (choice == 3)
        {
            priorityScheduling(
                currentProcesses,
                ganttChart
            );


            displayResults(
                currentProcesses,
                "Priority"
            );


            displayExecution(
                ganttChart
            );


            displayGanttChart(
                ganttChart
            );
        }


        // ====================================================
        // ROUND ROBIN
        // ====================================================

        else if (choice == 4)
        {
            int timeQuantum;


            cout
                << "\nEnter Time Quantum: ";

            cin >> timeQuantum;


            if (timeQuantum <= 0)
            {
                cout
                    << "\nTime Quantum must be greater than 0.\n";

                continue;
            }


            roundRobin(
                currentProcesses,
                timeQuantum,
                ganttChart
            );


            displayResults(
                currentProcesses,
                "Round Robin"
            );


            displayExecution(
                ganttChart
            );


            displayGanttChart(
                ganttChart
            );
        }


        // ====================================================
        // EXIT
        // ====================================================

        else if (choice == 5)
        {
            cout
                << "\nExiting CPU Scheduling Simulator...\n";

            cout
                << "Program terminated successfully.\n";


            break;
        }


        // ====================================================
        // INVALID CHOICE
        // ====================================================

        else
        {
            cout
                << "\nInvalid choice. Please select 1-5.\n";
        }
    }


    return 0;
}