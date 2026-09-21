#include "proc_stats.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int get_fault_stats(FaultStats *stats)
{
    char path[128];

    snprintf(path,
             sizeof(path),
             "/proc/%d/stat",
             getpid());

    FILE *fp = fopen(path, "r");

    if (fp == NULL)
    {
        perror("fopen /proc/stat");
        return -1;
    }

    char line[4096];

    if (fgets(line, sizeof(line), fp) == NULL)
    {
        fclose(fp);
        return -1;
    }

    fclose(fp);

    /*
     * /proc/PID/stat:
     * field 10 = minor faults
     * field 12 = major faults
     *
     * The process name can contain spaces, so locate
     * the final ')' before parsing the remaining fields.
     */

    char *closing = strrchr(line, ')');

    if (closing == NULL)
        return -1;

    char *p = closing + 2;

    char state;
    unsigned long value;

    /*
     * After the process name:
     * field 3 = state
     * field 4 = ppid
     * ...
     * field 10 = minflt
     * field 12 = majflt
     */

    int field = 3;

    char *token = strtok(p, " ");

    unsigned long minflt = 0;
    unsigned long majflt = 0;

    while (token != NULL)
    {
        if (field == 3)
        {
            state = token[0];
            (void)state;
        }

        if (field == 10)
        {
            minflt = strtoul(token, NULL, 10);
        }

        if (field == 12)
        {
            majflt = strtoul(token, NULL, 10);
            break;
        }

        token = strtok(NULL, " ");
        field++;
    }

    (void)value;

    stats->minor_faults = minflt;
    stats->major_faults = majflt;

    return 0;
}

void print_memory_maps(void)
{
    char path[128];

    snprintf(path,
             sizeof(path),
             "/proc/%d/maps",
             getpid());

    FILE *fp = fopen(path, "r");

    if (fp == NULL)
    {
        perror("fopen /proc/maps");
        return;
    }

    printf("\n========== /proc/%d/maps ==========\n",
           getpid());

    char line[512];

    while (fgets(line, sizeof(line), fp) != NULL)
    {
        printf("%s", line);
    }

    printf("====================================\n");

    fclose(fp);
}
