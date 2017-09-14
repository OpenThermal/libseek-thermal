/*
 *  Seek debug macros
 *  Author: Maarten Vandersteegen
 */

#ifndef SEEK_DEBUG_H
#define SEEK_DEBUG_H

#include <stdio.h>


#ifdef SEEK_DEBUG
#define debug(fmt, ...)     printf("%s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, ## __VA_ARGS__);
#define error(fmt, ...)     fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, ## __VA_ARGS__);
#else
#define debug(...)
#define error(fmt, ...)     fprintf(stderr, fmt, ## __VA_ARGS__);
#endif

#endif /* SEEK_DEBUG_H */
