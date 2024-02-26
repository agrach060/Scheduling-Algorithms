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

typedef struct task_metrics
{
    char *name;
    int tat;
    int wt;
    int rt;
} TaskMetrics;

TaskMetrics metrics[100];
int metrics_count = 0;

struct node *head = NULL;
int currentTime = 0;

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

// finding the next task based on priority scheduling
Task *pickNextTask()
{
    // check if the list is empty
    if (!head)
        return NULL;

    struct node *temp = head;
    Task *highest_priority_task = temp->task;

    // iterating through the list to find the task with the highest priority
    while (temp != NULL)
    {
        if (temp->task->priority > highest_priority_task->priority ||
            (temp->task->priority == highest_priority_task->priority &&
             strcmp(temp->task->name, highest_priority_task->name) < 0))
        {
            highest_priority_task = temp->task;
        }
        temp = temp->next;
    }

    // removing the task from the list
    delete (&head, highest_priority_task);

    return highest_priority_task;
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

bool comesBefore(char *a, char *b) { return strcmp(a, b) < 0; }

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
    int currentTime = 0;
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

        // increment task count after each task switch
        taskCount++;
    }

    // calculating CPU utilization
    double cpuUtilization = 100.0; // assume 100% utilization by default
    cpuUtilization = ((double)executionTime / totalTime) * 100.0;
    printf("CPU Utilization: %.2f%%\n", cpuUtilization);
    sortMetrics();
    printMetrics();
}
