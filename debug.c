/*************************************************************************//**
 * @file debug.c
 *
 *     Provides basic debug support.
 *
 ****************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <pthread.h>
#include "debug.h"


/*******************************************************************************
 * Private variables
 ******************************************************************************/
static pthread_mutex_t mx_output;
static bool is_init = false;


/*******************************************************************************
 * Private functions
 ******************************************************************************/
static void lock(bool enter)
{
    enter ? pthread_mutex_lock(&mx_output) : pthread_mutex_unlock(&mx_output);
}


/*******************************************************************************
 * Public functions
 ******************************************************************************/


void debug_init(void)
{
    if (!is_init)
    {
        pthread_mutex_init(&mx_output, NULL);
        is_init = true;
    }
}


void debug_done(void)
{
    if (is_init)
    {
        pthread_mutex_destroy(&mx_output);
        is_init = false;
    }
}


void debug_dump_data(FILE *file, const uint8_t *data, int length, int bytes_per_line, const char *title, ...)
{
    va_list args;
    int lines = (bytes_per_line <= 0) ? bytes_per_line = length, 1 : ((length + bytes_per_line - 1) / bytes_per_line);
    int i;
    int ptr = 0;

    
    lock(true);
    {
        if (title)
        {
            va_start(args, title);
            vfprintf(file, title, args);
            va_end(args);
        }

        for (i = 0; i < lines; ++i)
        {
            int b;
            for (b = 0; b < bytes_per_line; ++b)
            {
                if (ptr >= length)
                    break;

                fprintf(file, "%s%02x", b == 0 ? "" : " ", data[ptr]);
                ++ptr;
            }
            fprintf(file, "\n");
        }
        lock(false);
    }
}


/*****************************************************************************
 * End of file
 ****************************************************************************/
