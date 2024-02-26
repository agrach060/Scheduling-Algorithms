#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "task.h"
#include "list.h"
#include "schedulers.h"
#include "cpu.h"

// time to switch between tasks
#define DISPATCHER_TIME 1

struct node *head = NULL;

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
    newTask->priority = priority;
    newTask->burst = burst;
    newTask->remainingBurst = burst;
    newTask->tat = 0;
    newTask->wt = 0;
    newTask->rt = 0;
    newTask->started = false;
    // adding a task to the list
    insert(&head, newTask);
}

bool comesBefore(char *a, char *b) { return strcmp(a, b) < 0; }

// based on traverse from list.c
// finds the task whose name comes first in dictionary
Task *pickNextTask()
{
    // check if the list is empty
    if (!head)
        return NULL;

    struct node *temp = head;
    Task *best_sofar = temp->task;

    while (temp != NULL)
    {
        if (comesBefore(temp->task->name, best_sofar->name))
            best_sofar = temp->task;
        temp = temp->next;
    }
    // delete the node from list, Task will get deleted later
    delete (&head, best_sofar);
    return best_sofar;
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

void schedule()
{
    int totalTime = 0;     // with dispatching
    int executionTime = 0; // without dispatching
    int currentTime = 0;
    Task *task;

    // executing tasks
    while ((task = pickNextTask()) != NULL)
    {
        // adding dispatcher time for each task switch except before the first task
        if (totalTime > 0)
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

        // freeing the memory
        free(task->name);
        free(task);
    }

    // calculating CPU utilization
    double cpuUtilization = ((double)executionTime / totalTime) * 100.0;
    printf("CPU Utilization: %.2f%%\n", cpuUtilization);

    printMetrics();
}
