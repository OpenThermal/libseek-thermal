/*
 *  Seek debug macros
 *  Author: Maarten Vandersteegen
 */

#ifndef SEEK_DEBUG_H
#define SEEK_DEBUG_H

#include <stdio.h>

#ifdef SEEK_DEBUG
#define debug printf
#else
#define debug(...)
#endif

#endif /* SEEK_DEBUG_H */
