#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "task.h"
#include "list.h"
#include "cpu.h"
#include "schedulers.h"

// time to switch between tasks
#define DISPATCHER_TIME 1

struct node *head = NULL;
int currentTime = 0;

typedef struct task_metrics
{
    char *name;
    int tat;
    int wt;
    int rt;
} TaskMetrics;

TaskMetrics metrics[100];
int metrics_count = 0;

void add(char *name, int priority, int burst)
{
    Task *newTask = (Task *)malloc(sizeof(Task));
    newTask->name = strdup(name);
    newTask->burst = burst;
    newTask->remainingBurst = burst;
    newTask->priority = priority;
    newTask->tat = 0;
    newTask->wt = 0;
    newTask->rt = 0;
    newTask->started = false;
    // adding a task to the list
    insert(&head, newTask);
}

// lecixographical order
bool comesBefore(char *a, char *b) { return strcmp(a, b) < 0; }

// finds the task with the shortest burst time
Task *pickNextTask()
{
    // check if the list is empty
    if (!head)
        return NULL;

    struct node *temp = head;
    Task *shortest_task = temp->task;

    // iterating through the list to find the task with the shortest CPU burst time
    while (temp != NULL)
    {
        if (temp->task->burst < shortest_task->burst)
        {
            shortest_task = temp->task;
        }
        else if (temp->task->burst == shortest_task->burst)
        {
            if (comesBefore(temp->task->name, shortest_task->name))
            {
                shortest_task = temp->task;
            }
        }
        temp = temp->next;
    }

    // removing the task from the list
    delete (&head, shortest_task);

    return shortest_task;
}

void printMetrics()
{
    printf("...|");
    for (int i = 0; i < metrics_count; i++)
    {
        printf(" %s |", metrics[i].name);
    }
    printf("\nTAT|");
    for (int i = 0; i < metrics_count; i++)
    {
        printf(" %d |", metrics[i].tat);
    }
    printf("\nWT |");
    for (int i = 0; i < metrics_count; i++)
    {
        printf(" %d |", metrics[i].wt);
    }
    printf("\nRT |");
    for (int i = 0; i < metrics_count; i++)
    {
        printf(" %d |", metrics[i].rt);
    }
    printf("\n");

    // freeing the metrics array
    for (int i = 0; i < metrics_count; i++)
    {
        free(metrics[i].name);
    }
}

void sortMetrics()
{
    int i, j;
    for (i = 0; i < metrics_count - 1; i++)
    {
        for (j = 0; j < metrics_count - i - 1; j++)
        {
            if (comesBefore(metrics[j + 1].name, metrics[j].name))
            {
                TaskMetrics temp = metrics[j];
                metrics[j] = metrics[j + 1];
                metrics[j + 1] = temp;
            }
        }
    }
}

void schedule()
{
    int totalTime = 0;     // with dispatching
    int executionTime = 0; // without dispatching
    int taskCount = 0;     // number of tasks for dispatcher time calculation
    Task *task;

    while ((task = pickNextTask()) != NULL)
    {
        // adding dispatcher time for each task switch except before the first task
        if (taskCount > 0)
        {
            totalTime += DISPATCHER_TIME;
        }

        // if the task hasn't started yet, add the current time to the response time
        if (!task->started)
        {
            task->rt = currentTime;
            task->started = true;
        }

        // running the task
        run(task, task->burst);

        executionTime += task->burst;
        totalTime += task->burst;
        currentTime += task->burst;

        // TAT = currentTime when the task completes
        task->tat = currentTime;
        // WT = TAT - burst
        task->wt = task->tat - task->burst;

        // storing task metrics
        metrics[metrics_count].name = strdup(task->name);
        metrics[metrics_count].tat = task->tat;
        metrics[metrics_count].wt = task->wt;
        metrics[metrics_count].rt = task->rt;
        metrics_count++;

        // incrementing task count after each task switch
        taskCount++;

        // freeing the memory
        free(task->name);
        free(task);
    }

    // calculating CPU utilization
    double cpuUtilization = 100.0; // assume 100% utilization by default
    cpuUtilization = ((double)executionTime / totalTime) * 100.0;
    printf("CPU Utilization: %.2f%%\n", cpuUtilization);
    sortMetrics();
    printMetrics();
}
