#ifndef _TOOLS_H_
#define _TOOLS_H_

#include "mydefs.h"

extern char *pattern;
extern char *pattern1;
extern char *pattern2;
extern int crwldb[];
extern cr_t idx[];
extern cr_t bots[];

/* needed for variation in alphabet */
char c_map(int i);

/* variation by recursion */
char *r_vary(char *str, int step, int maxn);

/* entry point for enumerating all urls */
char *vary(char *to, int step, int maxn);

/* clamp html source */
void clamp_src(char *out, char *conv);

/* extract loc's from sitemap */
void locs(const char *sz, char to[DEEP][SLEN + 1]);

/* trim leading whitespaces */
char *trim(char *inout);

/* is empty string? */
int isempty(char *inout);

/* to lower please */
void strtolower(char *out, char *conv);

/* transform diphtongs, unusual letters and such */
void boil_special(char *out, char *conv);

/* clamp vowels and some other symbols */
void boil_vowels(char *out, char *conv);

/* removes vowels etc. */
void boil(char *out, char *conv);

/* just tidy html/xml */
void tidy(char *out, char *conv);

/* find right index space (for one word) */
int cidx(const char *sz);

/* index all words per webpage p (mind the offset!) */
void windex(const cr_t *p, int offset, int *crwldb);

#endif
