#include "cdf.h"

/* Initialize a CDF distribution */
void init_CDF(struct CDF_Table* table)
{
    if(!table)
        return;

    table->entries = (struct CDF_Entry*)malloc(TG_CDF_TABLE_ENTRY * sizeof(struct CDF_Entry));
    table->num_entry = 0;
    table->max_entry = TG_CDF_TABLE_ENTRY;
    table->min_cdf = 0;
    table->max_cdf = 1;

    if (!(table->entries))
        perror("Error: malloc");
}

/* Free resources of a CDF distribution */
void free_CDF(struct CDF_Table* table)
{
    if (table)
        free(table->entries);
}

/* Get CDF distribution from a given file */
void load_CDF(struct CDF_Table* table, char *file_name)
{
    FILE *fd = NULL;
    char line[256] = {'\0'};
    struct CDF_Entry* e = NULL;
    int i = 0;

    if (!table)
        return;

    fd = fopen(file_name, "r");
    if (!fd)
        perror("Error: fopen");

    while (fgets(line, sizeof(line), fd) != NULL)
    {
        /* Resize entries */
        if (table->num_entry >= table->max_entry)
        {
            table->max_entry *= 2;
            e = (struct CDF_Entry*)malloc(table->max_entry * sizeof(struct CDF_Entry));
            if (!e)
                perror("Error: malloc");
            for (i = 0; i < table->num_entry; i++)
                e[i] = table->entries[i];
            free(table->entries);
            table->entries = e;
        }

        sscanf(line, "%lf %lf", &(table->entries[table->num_entry].value), &(table->entries[table->num_entry].cdf));

        if (table->min_cdf > table->entries[table->num_entry].cdf)
            table->min_cdf = table->entries[table->num_entry].cdf;
        if (table->max_cdf < table->entries[table->num_entry].cdf)
            table->max_cdf = table->entries[table->num_entry].cdf;

        table->num_entry++;
    }
    fclose(fd);
}

/* Print CDF distribution information */
void print_CDF(struct CDF_Table* table)
{
    int i = 0;

    if (!table)
        return;

    for (i = 0; i < table->num_entry; i++)
        printf("%.2f %.2f\n", table->entries[i].value, table->entries[i].cdf);
}

/* Get average value of CDF distribution */
double avg_CDF(struct CDF_Table* table)
{
    int i = 0;
    double avg = 0;
    double value, prob;

    if (!table)
        return 0;

    for (i = 0; i < table->num_entry; i++)
    {
        if (i == 0)
        {
            value = table->entries[i].value/2;
            prob = table->entries[i].cdf;
        }
        else
        {
            value = (table->entries[i].value + table->entries[i-1].value)/2;
            prob = table->entries[i].cdf - table->entries[i-1].cdf;
        }
        avg += (value * prob);
    }

    return avg;
}

double interpolate(double x, double x1, double y1, double x2, double y2)
{
    if (x1 == x2)
        return (y1 + y2) / 2;
    else
        return y1 + (x - x1) * (y2 - y1) / (x2 - x1);
}

/* generate a random floating point number from min to max */
double rand_range(double min, double max)
{
    return min + rand() * (max - min) / RAND_MAX;
}

/* Generate a random value based on CDF distribution */
double gen_random_CDF(struct CDF_Table* table)
{
    int i = 0;
    double x = rand_range(table->min_cdf, table->max_cdf);
    //printf("%f %f %f\n", x, table->min_cdf, table->max_cdf);

    if (!table)
        return 0;

    for (i = 0; i < table->num_entry; i++)
    {
        if (x <= table->entries[i].cdf)
        {
            if (i == 0)
                return interpolate(x, 0, 0, table->entries[i].cdf, table->entries[i].value);
            else
                return interpolate(x, table->entries[i-1].cdf, table->entries[i-1].value, table->entries[i].cdf, table->entries[i].value);
        }
    }

    return table->entries[table->num_entry-1].value;
}
