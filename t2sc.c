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
unsigned int crwldb[SIZ * PLZ];
unsigned int crwldbsub[SIZ * PLZ];
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
  h = fopen("tld.dat", "rb");
  if (h) {
    for (int i = 0; i < NUM; ++i) {
      fread(idx[i].data, sizeof(char), BLOCK, h);
    }
    fclose(h);
  }
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

/* this saves zip code and upper part of Goedel number in sites.dat data structure, 
   therefore, we avoid to have the crwldb indexes cope with 64bit entries */
static void piggyimprint(int i, unsigned long int offset, const char *tld) {
  /* best use if sizeof(long) = 8 */
  unsigned int aux = ((i + offset) >> (sizeof(long)*4));
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
  /* ugly hack, piggyback geocode (PLZ) instead of <?xml... doesn't copy terminator */
  memcpy(sits[i].data, n, 5);
  sits[i].data[5]  = ' '; /* future use: must be ' ' because PLZ could be 6 places long, e.g. in UK */
  /* big endian, TODO: move this code elsewhere */
  sits[i].data[6]  = SELECT(aux, 3);
  sits[i].data[7]  = SELECT(aux, 2);
  sits[i].data[8]  = SELECT(aux, 1);
  sits[i].data[9] = SELECT(aux, 0);
}

/* short-hand write */
static char *lw(char *to, const char *what, int len) {
  if (to && len > 0) {
    strncpy(to, what, len);
    to[len] = '\0';
    return to += len;
  }
  return to;
}

/* simulate a (local) sitemap fetch for an absent sitemap */
static void generate_sitemap(char *p, char *source, const char *prefix) {
  const char *delim = "\"";
  const char *sheader = "<?xml ?><urlset>";
  const char *opener = "<url><loc>";
  const char *closer = "</loc></url>";
  const char *end = "</urlset>\n";
  const char *atag = "<a href=\"";
  const int z = strlen(opener) + strlen(prefix) + strlen(closer) + strlen(end) + 1;
  char *protsep = 0; /* for finding protocol URLs with protocol, e.g. javascript, outgoing http(s) */
  char *tok = 0;
  if (!p) return;
  char *begin = p;
  p = lw(p, sheader, strlen(sheader));
  while(source) {
    source = strstr(source, atag);
    if (source) {
      int len = 0;
      source += strlen(atag);
      tok = source + 1;
      /* just find if necessary */
      if (!protsep || protsep < tok) {
	protsep = strchr(tok, ':');
      }
      tok = strstr(tok, delim);
      len = tok - source;
      if (len > 0 && len < SLEN && (BLOCK - (p - begin)) > (len + z)) { /* sanity */
	if (!protsep || protsep > tok) { /* the separator could be behind the delim */
	  p = lw(p, opener, strlen(opener));
	  p = lw(p, prefix, strlen(prefix));
	  p = lw(p, source, len);
	  p = lw(p, closer, strlen(closer));
	}
      }
      source = tok;
    }
  }
  p = lw(p, end, strlen(end));
  *p = '\0';
}

/* get up to #DEEP sub pages as indexed by sitemap */
static void crawl_main(unsigned long int offset, const char *tld) {
  for (int i = 0; i < NUM; ++i) {
    if (bots[i].data) {
      const char *term = "Sitemap: ";
      const char *disallow = "Disallow: /\n";
      const char *noindex = "<meta name=\"robots\" content=\"noindex>";
      char *s = 0;
      char url[SLEN + 1] = {'\0'};
      int autogenerate = FALSE;
      if (strstr(bots[i].data, disallow) || strstr(idx[i].data, noindex)) {
	idx[i].data[0] = '\0'; /* DON'T INDEX */
	s = 0;
      } else {
	/* robots.txt allows and has sitemap? */
	s = strstr(bots[i].data, term);
      }
      if (s) {
	s += strlen(term); /* get URL after Sitemap: */
      } else {
	s = idx[i].data[0] ? HTTPHEAD : 0; /* dummy */
      }
      if (s) {
	/* something wrong, autogenerate */
	if (s - bots[i].data >= BLOCK || s < bots[i].data) {
	  autogenerate = TRUE;
	}
	/* get sitemap url */
	sscanf(s, "%80s", url);
	if (strstr(url, HTTPHEAD) == url) {
	  /* at the moment, just supporting 1 sitemap per website */
	  if (autogenerate) {
	    char site[SLEN + 1] = "";
	    strcpy(site, HTTPHEAD);
	    vary(&site[strlen(HTTPHEAD)], i + offset, strlen(pattern2));
	    strncat(site, tld, SLEN - strlen(site));
	    strncat(site, "/", SLEN - strlen(site));
	    sits[i].read = BLOCK;
	    generate_sitemap(sits[i].data, idx[i].data, site);
	  } else {
	    printf("GET: %s\n", url);
	    cget(0, url, &sits[i]); /* this can fail, but don't try to heal for now */
	  }
	  /* is it there? */
	  if (sits[i].read > 0) {
	    /* get all interesting site urls */
	    locs(sits[i].data, subs);
	    for (int k = 0; k < DEEP; ++k) {
	      char *url2 = subs[k];
	      char *tt = strstr(strnthchrnul(url, '.', 2), tld) + strlen(tld);
	      if (url2 && *url2 && 0 == strncmp(url, url2, tt - url)) {
		int tmpidx = k + 1;
		printf("GET: %d. %s\n", tmpidx, url2);
		idx[i].read = 0;
		/* reusing the same buffer i for each subsite fetch, don't keep it */
		cget(0, url2, &idx[i]);
		/* overview ("superset" of all subpages per website) offset+i */
		windex(&idx[i], offset, crwldb);
		/* detail offset+i+k+1 */
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
  unsigned long int offset = argc > 2 ? atol(argv[2]) : 0;
  char tld[SLEN + 1] = ".";
  init();
  filldb();
  strncat(tld, argv[1], SLEN - strlen(tld));
  crawl_main(offset, tld);
  dumpdb();
  uninit();
  return 0;
}

