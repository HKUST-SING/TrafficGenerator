#ifndef CDF_H
#define CDF_H

#include <stdio.h>
#include <stdlib.h>

#include "common.h"

#define TG_CDF_TABLE_ENTRY 32

struct CDF_Entry
{
    double value;
    double cdf;
};

/* CDF distribution */
struct CDF_Table
{
    struct CDF_Entry* entries;
    int num_entry;  //number of entries in CDF table
    int max_entry; //maximum number of entries in CDF table
    double min_cdf;    //min value of CDF (default 0)
    double max_cdf;    //max value of CDF (default 1)
};

/* Initialize a CDF distribution */
void init_CDF(struct CDF_Table* table);

/* Free resources of a CDF distribution */
void free_CDF(struct CDF_Table* table);

/* Get CDF distribution from a given file */
void load_CDF(struct CDF_Table* table, char *file_name);

/* Print CDF distribution information */
void print_CDF(struct CDF_Table* table);

/* Get average value of CDF distribution */
double avg_CDF(struct CDF_Table* table);

/* Generate a random value based on CDF distribution */
double gen_random_CDF(struct CDF_Table* table);

#endif
