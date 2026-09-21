#ifndef PROC_STATS_H
#define PROC_STATS_H

typedef struct
{
    unsigned long minor_faults;
    unsigned long major_faults;
} FaultStats;

int get_fault_stats(FaultStats *stats);
void print_memory_maps(void);

#endif
