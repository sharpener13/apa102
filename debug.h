/*************************************************************************//**
 * @file debug.h
 *
 *     Provides basic debug support.
 *
 ****************************************************************************/
#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

/*****************************************************************************
 * Public macros
 ****************************************************************************/
#ifdef DEBUG
#define FNAME "%-16s"
#define DEBUG_MSG(file, msg) fprintf(file, FNAME msg, __FILE__)
#define DEBUG_FMT(file, msg, ...) fprintf(file, FNAME msg, __FILE__, ##__VA_ARGS__)
#define DEBUG_DMP(file, data, length, bpl, title, ...) debug_dump_data(file, data, length, bpl, FNAME title "\n", __FILE__, ##__VA_ARGS__)
#else
#define DEBUG_MSG(file, msg) 
#define DEBUG_FMT(file, msg, ...)
#define DEBUG_DMP(file, data, length, bpl, title, ...)
#endif


/*****************************************************************************
 * Public prototypes
 ****************************************************************************/
void debug_init(void);
void debug_done(void);
void debug_dump_data(FILE *file, const uint8_t *data, int length, int bytes_per_line, const char *title, ...);


#endif
/*****************************************************************************
 * End of file
 ****************************************************************************/
