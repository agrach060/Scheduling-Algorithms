#include <stdio.h>
#include <stdlib.h>
#include "task.h"
#include "list.h"
#include "cpu.h"
#include "schedulers.h"

#define DISPATCHER_TIME 1

struct node *g_head = NULL;

void CPUutilization()
{
    int totalTime = 0;     // Total time including dispatching
    int executionTime = 0; // Total execution time without dispatching
    Task *task;

    while ((task = pickNextTask()) != NULL)
    {
        // Add dispatcher time for each task switch except before the first task
        if (totalTime > 0)
        {
            totalTime += DISPATCHER_TIME;
        }

        run(task, task->burst);
        executionTime += task->burst;
        totalTime += task->burst;

        // Here, you would remove the task from the list and free memory if needed
    }

    // Calculate CPU utilization
    double cpuUtilization = ((double)executionTime / totalTime) * 100.0;
    printf("CPU Utilization: %.2f%%\n", cpuUtilization);
}
