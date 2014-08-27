/**
 * @file common.h
 * @brief things common to all parts of landslide
 * @author Ben Blum <bblum@andrew.cmu.edu>
 */

#ifndef __ID_COMMON_H
#define __ID_COMMON_H

#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#define NORETURN __attribute__((noreturn))

#define COLOUR_BOLD "\033[01m"
#define COLOUR_DARK "\033[00m"
#define COLOUR_RED "\033[31m"
#define COLOUR_GREEN "\033[32m"
#define COLOUR_YELLOW "\033[33m"
#define COLOUR_BLUE "\033[34m"
#define COLOUR_MAGENTA "\033[35m"
#define COLOUR_CYAN "\033[36m"
#define COLOUR_GREY "\033[37m"
#define COLOUR_WHITE "\033[38m"
#define COLOUR_DEFAULT "\033[00m"

#define BUF_SIZE 256 /* default length for internal print buffers */

#define typeof __typeof__

#define ERR(...) do {							\
		fprintf(stderr, COLOUR_BOLD COLOUR_RED __VA_ARGS__);	\
		fprintf(stderr, COLOUR_DEFAULT);			\
	} while (0)

#define WARN(...) do {							\
		fprintf(stderr, COLOUR_BOLD COLOUR_YELLOW __VA_ARGS__);	\
		fprintf(stderr, COLOUR_DEFAULT);			\
	} while (0)

#endif