/*
   t2.c = simple crawler & indexer based on word hashing
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
cr_t idx[NUM];
cr_t bots[NUM];

/* init index */
static void filldb(void) {
  FILE *h = fopen("t2.dat", "rb");
  if (h) fread(crwldb, sizeof(int), SIZ * PLZ, h);
  if (h) fclose(h);
}

/* dump index */
static void dumpdb(void) {
  FILE *h = fopen("t2.dat", "wr+b");
  if (h) {
    fwrite(crwldb, sizeof(int), SIZ * PLZ, h);
    fclose(h);
  }
  h = fopen("tld.dat", "wr+b");
  if (h) {
    for (int i = 0; i < NUM; ++i) {
      fwrite(idx[i].data, sizeof(char), BLOCK, h);
    }
    fclose(h);
  }
  h = fopen("robots.dat", "wr+b");
  if (h) {
    for (int i = 0; i < NUM; ++i) {
      fwrite(bots[i].data, sizeof(char), BLOCK, h);
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
  }
}

static void uninit(void) {
  for (int i = 0; i < NUM; ++i) { /* exit */
    free(idx[i].data);
    free(bots[i].data);
  }
}

/* do main crawl job using curl */
static void crawl_main(const char *tld, unsigned long int offset) {
  CURLM *cm = 0;
  CURLMsg *msg = 0;
  int msgsleft = -1;
  curl_global_init(CURL_GLOBAL_ALL);
  cm = curl_multi_init();
  curl_multi_setopt(cm, CURLMOPT_MAXCONNECTS, (long)MAX_PARALLEL); /* crawl 2*10 at a time from chunk (index+robots) */
  /* main crawl task */
  for (int n = 0; n < NUM; ++n) {
    char site[SLEN + 1] = {'\0'};
    strcpy(site, HTTPHEAD);
    vary(&site[strlen(HTTPHEAD)],n + offset,strlen(pattern2));
    strncat(site, tld, SLEN - strlen(site));
    printf("GET: %s\n", site); /* print url */
    /* calls libcurl */
    strncpy(idx[n].url, site, SLEN - 1);
    cget(cm, site, &idx[n]);
    strncat(site, "/robots.txt", SLEN - strlen(site));
    strncpy(bots[n].url, site, SLEN - 1);
    cget(cm, site, &bots[n]);
    printf("GET: %s\n", site); /* print url */
    if (n % MAX_PARALLEL == MAX_PARALLEL - 1) {
      int active = 1;
      do {
	curl_multi_perform(cm, &active);
	curl_multi_wait(cm, NULL, 0, 1000, NULL);
      } while(active); /* sync all */
      while (msg = curl_multi_info_read(cm, &msgsleft)) {
	if (msg->msg == CURLMSG_DONE) {
	  CURL *eh = msg->easy_handle;
	  cr_t *crt = 0;
	  curl_easy_getinfo(eh, CURLINFO_PRIVATE, &crt);
	  if (crt) {
	    if (!strstr(crt->url, "/robots.txt")) {
	      windex(crt, offset, crwldb);
	    }
	  }
	  curl_multi_remove_handle(cm, eh);
	  curl_easy_cleanup(eh);
	}
      }
    }
  }
  curl_multi_cleanup(cm);
  curl_global_cleanup();
}

/* well, ...main */
int main(int argc, char **argv) {
  init();
  if (argc <= 1) {
    printf("usage: t2 [--clear-all] <tld> <positive offset>\nT2, T2SC, T2RES & T2Q (c) 2023 Nils Jensen\nLicence: GNUPL3.0\nThis program comes with ABSOLUTELY NO WARRANTY; This is free software, and you are welcome to redistribute it under certain conditions; See COPYING in this folder.\nThanks to libcurl.se!\n");
    uninit();
    return 0;
  }
  if (argc == 2 && 0 == strcmp(argv[1], "--clear-all")) {
    printf("CLEARING indexes!\n");
    dumpdb();
    uninit();
    return 0;
  } /* else */
  filldb();
  if (argc == 2) { /* feed in one url */
    char *url = argv[1];
    printf("GET: %s\n", url);
    cget(0, url, &idx[0]);
    printf("RESULT: %s\n", idx[0].data);
  } else { /* crawl chunk of urls */
    unsigned long int offset = 0;
    char tld[SLEN + 1] = {'.','\0'};
    if (argc > 2) {
      strncat(tld, argv[1], sizeof(tld) - 2);
      sscanf(argv[2], "%lu", &offset);
      if (offset < 0) { /* correct wrong offsets */
	offset = 0;
      }
    }
    crawl_main(tld, offset);
  }
  dumpdb();
  uninit();
  return 0;
}
