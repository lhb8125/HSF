#include "utilityExceptions.h"


void handler(int sig) {
  void *array[10];
  size_t size;
  char **strings;
  size_t i;
  // get void*'s for all entries on the stack
  size = backtrace(array, 10);

  // print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);
  strings = backtrace_symbols (array, size);

  printf ("Obtained %zd stack frames.\n", size);

  for (i = 0; i < size; i++)
     printf ("%s\n", strings[i]);

  free (strings);
  exit(1);
}

