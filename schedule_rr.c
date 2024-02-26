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

bool comesBefore(char *a, char *b) { return strcmp(a, b) < 0; }

// finds the next task
Task *pickNextTask()
{
    if (head == NULL)
    {
        // no tasks left in the queue
        return NULL;
    }
    // starting from the head of the queue
    struct node *temp = head;

    while (temp->next != NULL)
    {
        temp = temp->next;
    }

    Task *nextTask = temp->task;

    return nextTask;
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
    int totalTime = 0; // with dispatching
    int taskCount = 0; // without dispatching
    int executionTime = 0;
    int currentTime = 0;

    while (head != NULL)
    {
        Task *nextTask = pickNextTask();
        if (nextTask == NULL)
        {
            break;
        }

        if (taskCount > 0)
        {
            totalTime += DISPATCHER_TIME;
        }

        if (nextTask->started == false)
        {
            nextTask->started = true;
            nextTask->rt = currentTime;
        }

        int slice = nextTask->burst > QUANTUM ? QUANTUM : nextTask->burst;
        run(nextTask, slice);
        executionTime += slice;
        totalTime += slice;
        currentTime += slice;

        nextTask->burst -= slice;

        if (nextTask->burst <= 0)
        {
            nextTask->tat = currentTime;
            nextTask->wt = nextTask->tat - nextTask->remainingBurst;
            metrics[metrics_count++] = (TaskMetrics){strdup(nextTask->name), nextTask->tat, nextTask->wt, nextTask->rt};
            delete (&head, nextTask);
        }
        else
        {
            delete (&head, nextTask);
            insert(&head, nextTask);
        }
        taskCount++;
    }

    // printing CPU utilization and metrics
    double cpuUtilization = (double)totalTime ? ((double)(totalTime - (taskCount * DISPATCHER_TIME)) / totalTime) * 100 : 0;
    printf("CPU Utilization: %.2f%%\n", cpuUtilization);
    sortMetrics();
    printMetrics();
}
