/*
   t2q.c = simple query tool based on word hashing
   (w) 2023 Nils Jensen
   Licence: GnuPL3
   Using curl library: libcurl.se
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "tools.h"

#define DEEP (127)

char *pattern = "bcdfghjlmnrst";
char *pattern1 = "aeiou";
char *pattern2 = "-0123456789abcdefghijklmnopqrstuvwxyz";
unsigned int crwldb[SIZ * PLZ];
unsigned int crwldbsub[SIZ * PLZ];
cr_t idx[NUM];
cr_t bots[NUM]; /* for subsite retrieval, not robots! */
static char subs[DEEP][SLEN + 1];

/* init index */
static void filldb(void) {
  FILE *h = fopen("ram/t2.dat", "rb");
  if (h) fread(crwldb, sizeof(int), SIZ * PLZ, h);
  if (h) fclose(h);
  h = fopen("ram/t2deep.dat", "rb");
  if (h) fread(crwldbsub, sizeof(int), SIZ * PLZ, h);
  if (h) fclose(h);
}

static void init(void) {
  for (int i = 0; i < NUM; ++i) { /* init */
    idx[i].data = calloc(BLOCK, sizeof(char));
    idx[i].read = 0;
    bots[i].data = calloc(BLOCK, sizeof(char));
    bots[i].read = 0;
  }
}

static void uninit(void) {
  for (int i = 0; i < NUM; ++i) { /* exit */
    free(idx[i].data);
    free(bots[i].data);
  }
}

static int details(cr_t *p, cr_t *p2, unsigned long int site) {
  /* note: it is inefficient to retrieve the handle for every call */
  FILE *h = fopen("www_data/tld.dat", "rb");
  FILE *hs = fopen("www_data/sites.dat", "rb");
  int rc = 0;
  if (p) {
    p->read = 0;
    p->data[0] = '\0';
  }
  if (p2) {
    p2->read = 0;
    p2->data[0] = '\0';
  }
  if (h) {
    rc = fseek(h, site * BLOCK, SEEK_SET);
    if (!rc && p) {
      rc = fread(p->data, sizeof(char), BLOCK, h);
      if (!rc) p->read = BLOCK;
      p->data[BLOCK - 1] = '\0';
    }
    fclose(h);
  }
  if (hs) {
    rc = fseek(hs, site * BLOCK, SEEK_SET);
    if (!rc && p2) {
      rc = fread(p2->data, sizeof(char), BLOCK, hs);
      if (!rc) p2->read = BLOCK;
      p2->data[BLOCK - 1] = '\0';
    }
    fclose(hs);
  }
  /* nothing else to do */
  return !h || rc || !p || !hs || !p2;
}

static unsigned long int get_aux(char *data) {
  if (!data) return 0UL;
  return PUT(data[0], 3) | PUT(data[1], 2) | PUT(data[2], 1) | PUT(data[3], 0);
}

static int rank(int hit, int slots, int begin, int secondterm, int id, int id2, int locator, char *tld, FILE *cache) {
  char out[SLEN + 1] = "";
  char tmp[BLOCK] = "";
  char tmp2[BLOCK] = "";
  for (int i = 0; hit < slots + begin && i < PLZ; ++i) {
    unsigned int site = crwldb[id + i];
    unsigned int subsite = crwldbsub[id + i];
    /* linear search (PLZ is small) */
    for (int k = 0; k < PLZ; ++k) {
      unsigned int site2 = crwldb[id2 + k];
      unsigned int subsite2 = crwldbsub[id2 + k];
      /* do hardcoded join */
      if (site && (!secondterm || site == site2 && subsite == subsite2)) {
	if (hit++ >= begin) {
	  int siteloc = 0;
	  unsigned long int aux = get_aux(bots[0].data + 6); /* after max len PLZ, get 2nd id segment */
	  unsigned long int fullsite = site + aux;
	  *out = '\0';
	  vary(out, fullsite, strlen(pattern2));
	  fprintf(cache, "<li><!--[%d.]--> <a href='%s%s%s'>%s%s%s</a><!--[%d,%d,%d]--></li>\n", hit, HTTPHEAD, out, tld, HTTPHEAD, out, tld, site, subsite ? subsite - site : 0, locator);
	  *tmp = *tmp2 = '\0';
	  details(&idx[0], &bots[0], fullsite); /* take fullsite if we don't overwrite buckets in sites.dat */
	  strtolower(tmp, idx[0].data);
	  clamp_src(tmp2, tmp);
	  sscanf(bots[0].data, "%d", &siteloc);
	  fprintf(cache, "&nbsp;%s [...] <span class='zip'>[%d]</span><br>\n", trim(tmp2), siteloc / 1000);
	  if (subsite) {
	    /* get all interesting site urls */
	    locs(bots[0].data, subs);
	    if (site < subsite) { /* sanitize */
	      char *sub = subs[subsite - site - 1];
	      fprintf(cache, "&nbsp;<a href='%s'>%s</a><br><br>\n", sub, sub);
	    }
	  }
	}
	/* exit for-loop */
	if (!secondterm) break;
      }
    }
  }
  return hit;
}

static void precache(FILE **h, int q, int q2, int locator, int begin) {
  if (!h) return;
  *h = stdout;
  fprintf(*h, "%s", "<!doctype html>\n<html>\n<head>\n                                         \
      <meta charset='UTF-8'>\n                                                                 \
      <title>T2</title><script src='../js.js'></script>                                        \
    <style>input {border-radius: 5px; line-height: 1.7;}\nspan.zip {font-weight: bold;}</style></head>\n<body onload='init()' \
                                                        style='font-family: arial;' id='main'> \
      <div style='width: 99%; margin: auto; font-family: arial; text-align: left;'>            \
      \n\n                                                                                      \
      <h1>T2 - Die Websuche</h1>\n                                                             \
      <br/>\n                                                                                  \
    <form action='q' onsubmit='return validirium()' method='GET'>\n                            \
      <input type='text' name='p' id='p'></input>\n                                            \
      <input type='hidden' name='l' id='l' value='");
  fprintf(*h, "%d", locator);
  fprintf(*h, "%s", "'></input>\n						               \
      <input type='hidden' name='r' id='r' value='");
  fprintf(*h, "%d", begin + 10);
  fprintf(*h, "%s", "'></input>\n                                                              \
      <input type='submit' value='Los!'></input>\n                                             \
    </form><br><span id='out'></span>\n                                                        \
    <ol style='margin-left: -20px'>\n");
}

static void postcache(FILE **h) {
  if (!h || !*h) return;
  fprintf(*h, "%s", "\n</ol></div></body></html>\n");
  *h = 0;
}

static int getcache(int q, int q2, int begin) {
  return 1;
}

/* well, ...main */
int main(int argc, char **argv) {
  if (argc > 2) {
    int begin = 0;
    int slots = 10;
    char in[BLOCK + 1] = {'\0'};
    char in2[BLOCK + 1] = {'\0'};
    char combi[BLOCK + 1] = {'\0'};
    char out[BLOCK + 1] = {'\0'};
    char out2[BLOCK + 1] = {'\0'};
    int locator = 0;
    int id = 0;
    int id2 = 0;
    int id3 = 0;
    int hit = 0;
    char *tld = argv[1];
    char *client = argv[2];
    int secondterm = (!!index(client, '+'));
    FILE *cache = 0;
    init();
    filldb();
    sscanf(client, "p=%30[a-zA-Z0-9]", in);
    char *ptr = 0;
    while (isempty(in2) && (ptr = index(client, '+'))) {
      sscanf(ptr, "+%30[a-zA-Z]", in2);
      client++;
    }
    client = argv[2];
    if ((ptr = index(client, '&'))) {
      sscanf(ptr, "&l=%5d", &locator);
    }
    if ((ptr = rindex(client, '&'))) {
      sscanf(ptr, "&r=%d", &begin);
      if (begin < 0) begin = 0;
    }
    in[SLEN - 1] = '\0';
    in2[SLEN - 1] = '\0';
    boil(out, trim(in));
    boil(out2, trim(in2));
    strncpy(combi, out, SLEN - 1);
    combi[SLEN - 1] = '\0';
    strncat(combi, out2, SLEN - strlen(combi));
    combi[SLEN - 1] = '\0';
    id = cidx(out);
    id2 = cidx(out2);
    id3 = cidx(combi);
    if (!id && id2) { /* sanitize */
      id = id2;
      id2 = 0;
    }
    if (getcache(id, id2, begin)) {
      secondterm = !!id2;
      precache(&cache, id, id2, locator, begin);
      if (id3 != id) {
	hit = rank(hit, slots, begin, FALSE, id3, 0, locator, tld, cache);
      }
      hit = rank(hit, slots, begin, secondterm, id, id2, locator, tld, cache);
      if (!hit) {
	fprintf(cache, "%s\n", "<span style='margin-left: -20px'>Leider 0 Treffer, versuchen Sie bis zu zwei andere Begriffe.</span><br>");
      }
      postcache(&cache);
      getcache(id, id2, begin);
    }
    uninit();
  } else {
    printf("%s\n", "usage: t2q TLD <query>;\nexample: t2q .de 'p=hello+world&l=20000'\n");
  }
  return 0;
}
