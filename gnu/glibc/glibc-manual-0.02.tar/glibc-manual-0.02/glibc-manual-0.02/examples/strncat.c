#include <string.h>
#include <stdio.h>

#define SIZE 10

static char buffer[SIZE];

main ()
{
  strncpy (buffer, "hello", SIZE);
  printf ("%s\n", buffer);
  strncat (buffer, ", world", SIZE - strlen (buffer) - 1);
  printf ("%s\n", buffer);
}
