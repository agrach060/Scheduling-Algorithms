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
    newTask->priority = priority;
    newTask->burst = burst;
    newTask->remainingBurst = burst;
    newTask->tat = 0;
    newTask->wt = 0;
    newTask->rt = 0;
    newTask->started = false;
    // inserting the task into the list
    insert(&head, newTask);
}

// finding the highest priority task that has not completed
Task *pickHighestPriorityTask()
{
    if (!head)
        return NULL;

    struct node *temp = head;
    Task *highestPriorityTask = temp->task;
    for (temp = temp->next; temp != NULL; temp = temp->next)
    {
        if (temp->task->priority > highestPriorityTask->priority ||
            (temp->task->priority == highestPriorityTask->priority &&
             temp->task->burst > 0))
        {
            // ensuring the task has remaining burst time
            highestPriorityTask = temp->task;
        }
    }
    return highestPriorityTask->burst > 0 ? highestPriorityTask : NULL;
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
    int totalTime = 0;
    int taskCount = 0;
    int currentTime = 0;
    int executionTime = 0;

    while (head != NULL)
    {
        Task *task = pickHighestPriorityTask();
        if (!task)
            break;

        if (taskCount > 0)
        {
            totalTime += DISPATCHER_TIME;
        }

        if (task->started == false)
        {
            task->started = true;
            task->rt = currentTime;
        }

        int slice = task->burst > QUANTUM ? QUANTUM : task->burst;
        run(task, slice);

        executionTime += slice;
        totalTime += slice;
        currentTime += slice;

        task->burst -= slice;

        if (task->burst <= 0)
        {
            task->tat = currentTime;
            task->wt = task->tat - task->remainingBurst;

            metrics[metrics_count++] = (TaskMetrics){strdup(task->name), task->tat, task->wt, task->rt};
            // removing completed task from list
            delete (&head, task);
        }
        else
        {
            delete (&head, task);
            // RR within the same priority
            insert(&head, task);
        }

        taskCount++;
    }

    // calculating CPU Utilization
    double cpuUtilization = (double)totalTime ? ((double)(totalTime - (taskCount * DISPATCHER_TIME)) / totalTime) * 100 : 0;
    printf("CPU Utilization: %.2f%%\n", cpuUtilization);

    sortMetrics();
    printMetrics();
}
