/* 
   t2res.c = resets index, run before re-running t2 
   (w) 2023 Nils Jensen
   Licence: GnuPL3
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "mydefs.h"

unsigned int crwldb[SIZ * PLZ];
unsigned int crwldbsub[SIZ * PLZ];

/* init index */
static void filldb(void) {
  FILE *h = fopen("t2.dat", "rb");
  if (h) fread(crwldb, sizeof(int), SIZ * PLZ, h);
  if (h) fclose(h);
  h = fopen("t2deep.dat", "rb");
  if (h) fread(crwldbsub, sizeof(int), SIZ * PLZ, h);
  if (h) fclose(h);
}

/* dump partially nullified crawl index */
static void dumpdb(void) {
  FILE *h = fopen("t2.dat", "wr+b");
  if (h) {
    fwrite(crwldb, sizeof(int), SIZ * PLZ, h);
    fclose(h);
  }
  h = fopen("t2deep.dat", "wr+b");
  if (h) {
    fwrite(crwldbsub, sizeof(int), SIZ * PLZ, h);
    fclose(h);
  }
}

void reset_interval(unsigned long int offset) {
  for (int i = 0; i < SIZ * PLZ; ++i) {
    if (crwldb[i] != 0 && crwldb[i] >= (unsigned int)offset && crwldb[i] < (unsigned int)(offset + NUM)) {
      crwldb[i] = 0;
      crwldbsub[i] = 0;
    }
  }
}

int main(int argc, char *argv[]) {
  unsigned long int offset = argc > 1 ? atol(argv[1]) : 0;
  filldb();
  reset_interval(offset);
  dumpdb();
  return 0;
}

