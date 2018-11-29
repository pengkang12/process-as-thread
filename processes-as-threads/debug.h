#ifndef MYDEBUG_H_
#define MYDEBUG_H_

#include <stdio.h>
#include <assert.h>

#ifdef DEBUG
	#define DEBUG(...) fprintf(stderr, "%20s:%-4d: ", __FILE__, __LINE__); fprintf(stderr, "\n")
#else
	#define DEBUG(_fmt, ...)
#endif

#endif
