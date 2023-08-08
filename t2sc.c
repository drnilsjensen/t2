/* 
   t2sc.c = subsite indexer, needs t2 to run first 
   (w) 2023 Nils Jensen
   Licence: GnuPL3
   Using curl library: libcurl.se
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <curl/curl.h>

#include "tools.h"
#include "curlhelper.h"

char *pattern = "bcdfghjlmnrst";
char *pattern1 = "aeiou";
char *pattern2 = "-0123456789abcdefghijklmnopqrstuvwxyz";
int crwldb[SIZ * PLZ];
int crwldbsub[SIZ * PLZ];
cr_t idx[NUM];
cr_t bots[NUM];
static cr_t sits[NUM];
static char subs[DEEP][SLEN + 1];
static cr_t current;

/* init index */
static void filldb(void) {
  FILE *h = fopen("t2.dat", "rb");
  if (h) fread(crwldb, sizeof(int), SIZ * PLZ, h);
  if (h) fclose(h);
  h = fopen("t2deep.dat", "rb");
  if (h) fread(crwldbsub, sizeof(int), SIZ * PLZ, h);
  if (h) fclose(h);
  h = fopen("robots.dat", "rb");
  if (h) {
    for (int i = 0; i < NUM; ++i) {
      fread(bots[i].data, sizeof(char), BLOCK, h);
    }
    fclose(h);
  }
}

/* dump index, don't dump idx! */
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
  h = fopen("sites.dat", "wr+b");
  if (h) {
    for (int i = 0; i < NUM; ++i) {
      fwrite(sits[i].data, sizeof(char), BLOCK, h);
    }
    fclose(h);
  }
}

static void init(void) {
  for (int i = 0; i < NUM; ++i) { /* init */
    idx[i].data = calloc(BLOCK, sizeof(char));
    idx[i].read = 0;
    bots[i].data = calloc(BLOCK, sizeof(char));
    bots[i].read = 0;
    sits[i].data = calloc(BLOCK, sizeof(char));
    sits[i].read = 0;
  }
  current.read = 0;
  current.data = calloc(BLOCK*100, sizeof(char));
}

static void uninit(void) {
  for (int i = 0; i < NUM; ++i) { /* exit */
    free(idx[i].data);
    free(bots[i].data);
    free(sits[i].data);
  }
  free(current.data);
}

static void piggyimprint(int i, int offset, const char *tld) {
  char url[SLEN + 1] = {'\0'};
  strcpy(url, HTTPHEAD);
  vary(&url[strlen(HTTPHEAD)],i + offset,strlen(pattern2));
  strncat(url, tld, SLEN - strlen(url));
  strncat(url, "/impressum", SLEN - strlen(url));
  printf("GET: %s\n", url); /* print url */
  current.read = 0;
  /* make this blocking (as now) or use several current's ! */
  xget(0, url, &current, write_callback100);
  char out[BLOCK*100] = "";
  char *ptr = out;
  tidy(out, current.data);
  char n[6];
  char *ptr2 = n;
  int len = 0;
  while(*ptr && len != 5) {
    while(*ptr && !isdigit(*ptr)) {
      ptr++;
    }
    len = 0;
    ptr2 = n;
    while(*ptr && isdigit(*ptr)) {
      if (len <= 5+1) {
	*ptr2++ = *ptr++;
	++len;
      } else {
	ptr++;
      }
    }
  }
  n[5] = '\0';
  /* ugly hack, piggyback geocode (PLZ) instead of <?xml */
  memcpy(sits[i].data, n, 5);
}

/* get up to #DEEP sub pages as indexed by sitemap */
static void crawl_main(int offset, const char *tld) {
  for (int i = 0; i < NUM; ++i) {
    if (bots[i].data) {
      const char *term = "Sitemap: ";
      char *s = 0;
      char url[SLEN + 1] = {'\0'};
      /* robots.txt? */
      if ((s = strstr(bots[i].data, term))) {
	s += strlen(term);
	/* something wrong, skip */
	if (s - bots[i].data >= BLOCK) break;
	/* get sitemap url */
	sscanf(s, "%80s", url);
	if (strstr(url, HTTPHEAD) == url) {
	  printf("GET: %s\n", url);
	  /* at the moment, just supporting 1 sitemap per website */
	  cget(0, url, &sits[i]);
	  /* is it there? */
	  if (sits[i].read > 0) {
	    /* get all interesting site urls */
	    locs(sits[i].data, subs);
	    for (int k = 0; k < DEEP; ++k) {
	      char *url2 = subs[k];
	      if (*url2) {
		int tmpidx = k + 1;
		printf("GET: %d. %s\n", tmpidx, url2);
		idx[i].read = 0;
		/* reusing the same buffer i for each subsite fetch, don't keep it */
		cget(0, url2, &idx[i]);
		/* overview ("superset" of all subpages per website) */
		windex(&idx[i], offset, crwldb);
		/* detail */
		windex(&idx[i], tmpidx + offset, crwldbsub);
	      } /* url in <loc>? */
	    } /* each <loc> in sitemap */
	  } /* sitemap.xml? */
	} /* Sitemap:? */
	piggyimprint(i, offset, tld);
      } /* end of robots.txt */
    }
  } /* for each site */
}

int main(int argc, char *argv[]) {
  int offset = argc > 2 ? atoi(argv[2]) : 0;
  char tld[SLEN + 1] = ".";
  init();
  filldb();
  strncat(tld, argv[1], SLEN - strlen(tld));
  crawl_main(offset, tld);
  dumpdb();
  uninit();
  return 0;
}

