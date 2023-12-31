/*
   tools.c = tools for simple crawler & indexer based on word hashing
   (w) 2023 Nils Jensen
   Licence: GnuPL3
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "tools.h"

/* no double indexing of the same token per site */
static char done[SIZ];

/* needed for variation in alphabet */
char c_map(int i) {
  char c = '-';
  if (i > 10) {
    c = 'a' + i - 11;
  } else if (i > 0) {
    c = '0' + i - 1;
  }
  return c;
}

/* variation by recursion */
char *r_vary(char *str, unsigned long int step, unsigned int maxn) {
  if (step == 0 || maxn == 0) {
    return str;
  }
  char ce[] = {c_map(step % maxn), '\0'};
  return strncat(r_vary(str, step / maxn, maxn), ce, SLEN - strlen(str));
}

/* entry point for enumerating all urls */
char *vary(char *to, unsigned long int step, unsigned int maxn) {
  char domain[SLEN + 1] = {'\0'};
  r_vary(domain, step, maxn);
  strncat(to, domain, SLEN - strlen(to));
  return to;
}

/* clamp html source */
void clamp_src(char *out, char *conv) {
  int on = TRUE;
  int content = TRUE;
  int first = TRUE;
  char *script = "<script";
  char *style = "<style";
  char *noscript = "</script";
  char *nostyle = "</style";
  int k = 0;
  for (int i = 0; conv[i]; ++i) {
    if (conv[i] == '<') {
      on = FALSE;
    } else if (conv[i] == '>') {
      on = content ? TRUE : FALSE;
      conv[i] = ' '; /* ignore '>' */
    }
    if (!strncmp(&conv[i], script, strlen(script))) {
      content = FALSE;
    } else if (!strncmp(&conv[i], style, strlen(style))) {
      content = FALSE;
    } else if (!strncmp(&conv[i], noscript, strlen(noscript))) {
      content = TRUE;
    } else if (!strncmp(&conv[i], nostyle, strlen(nostyle))) {
      content = TRUE;
    }
    out[k] = on ? conv[i] : ' ';
    if (isgraph(out[k])) {
      first = TRUE;
      ++k;
    } else if (first) {
      ++k;
      first = FALSE;
    } /* else !first */
  }
  out[k++] = '\0';
}

/* extract loc's from sitemap */
void locs(const char *sz, char to[DEEP][SLEN + 1]) {
  const char *loc = "<loc>";
  const char *tok = sz + 11; /* ignore piggypack */
  int i = 0;
  while (tok++) {
    tok = strstr(tok, loc);
    if (tok && i < DEEP) {
      char *url = to[i++];
      sscanf(tok, "<loc>%80s</loc>", url);
      char *t = index(url, '<');
      if (t) {
	*t = '\0';
      }
      tok += strlen(url) + 1;
    }
  }
}

/* find nth repetition of char in string sz, ending by \0 */
char *strnthchrnul(char *sz, char c, int n) {
  if (!sz || !*sz) {
    return sz;
  }
  char *t = strchr(sz, c);
  if (!t) t = strchr(sz, '\0');
  if (n < 2 || !t || !*t) return t;
  else return strnthchrnul(t+1, c, n-1);
}

/* trim leading whitespaces */
char *trim(char *sz) {
  while (isspace((unsigned char) *sz))
    sz++;
  return sz;
}

/* to lower please */
void strtolower(char *out, char *conv) {
  while (*out++ = tolower(*conv++))
    ;
}

/* null or nothing? */
int isempty(char *sz) {
  return !sz || !*trim(sz);
}

/* TODO delete 
void xswap(char *a, char *b) {
  if (!a || !b) return;
  *a ^= *b;
  *b ^= *a;
  *a ^= *b;
} */

/* convert number to text (simple) */
const char *convert(const char *from) {
  static const char *s[] ={"null ",
	     "eins ", "tswei ",
	     "drei ", "vier ",
	     "fuenf ", "sechs ",
	     "sieben ", "acht ",
	     "neun "};
  int i = 0;
  if (from && isdigit(*from)) {
    i = *from - '0';
    return s[i];
  } else return "";
}

/* verbalize numbers to ca.-values (no exact values) */
void boil_numbers(char *out, char *conv) {
  int k = 0;
  for (int i = 0; conv[i] && i < BLOCK && k < BLOCK - 1; ++i) {
    /* room? */
    if (isdigit(conv[i]) && BLOCK - k - 1 >= 8) {
      const char *sz = convert(&conv[i]);
      strcpy(out + k, sz);
      k += strlen(sz);
    }
    out[k++] = conv[i];
  }
  out[k++] = '\0';
}

/* transform diphtongs, unusual letters and such */
void boil_special(char *out, char *conv) {
  int k = 0;
  char mixedin = 0;
  for (int i = 0; conv[i] && i < BLOCK && k < BLOCK - 1; ++i) {
    if (mixedin) {
      out[k++] = mixedin;
      mixedin = 0;
      i--; /* correct input index */
      continue;
    }
    if (conv[i] == 'p' && conv[i + 1] == 'h') {
      conv[i] = 'f';
      conv[i + 1] = '*';
    }
    if (conv[i] == 'q') {
      conv[i] = 'k';
      mixedin = 'u';
    }      
    if (conv[i] == 'x') {
      conv[i] = 'k';
      mixedin = 's';
    }
    if (conv[i] == 'z') {
      conv[i] = 't';
      mixedin = 's';
    }      
    if (conv[i] == 'ß') {
      conv[i] = 's';
      mixedin = 's';
    }
    out[k++] = conv[i];
  }
  out[k++] = '\0';
}

/* clamp vowels and some other symbols, MAGIC */
void boil_vowels(char *out, char *conv) {
  int k = 0;
  for (int i = 0; conv[i] && i < BLOCK && k < BLOCK - 1; ++i) {
    char c = ' ';
    switch(conv[i]) {
    case 'b': c = 'b'; break;
    case 'c': c = 'c'; break;
    case 'd': c = 'd'; break;
    case 'f': c = 'f'; break;
    case 'g': c = 'g'; break;
    case 'h': c = 'h'; break;
    case 'i': c = 'j'; break;
    case 'j': c = 'j'; break;
    case 'k': c = 'c'; break;
    case 'l': c = 'l'; break;
    case 'm': c = 'm'; break;
    case 'n': c = 'n'; break;
    case 'p': c = 'b'; break;
    case 'r': c = 'r'; break;
    case 's': c = 's'; break;
    case 't': c = 't'; break;
    case 'v': c = 'f'; break;
    case 'w': c = 'f'; break;
    case 'y': c = 'j'; break;
    case '0': c = '0'; break;
    case '1': c = '1'; break;
    case '2': c = '2'; break;
    case '3': c = '3'; break;
    case '4': c = '4'; break;
    case '5': c = '5'; break;
    case '6': c = '6'; break;
    case '7': c = '7'; break;
    case '8': c = '8'; break;
    case '9': c = '9'; break;
    case '-': c = ' '; break;
    case '+': c = ' '; break;
    case '*': c = ' '; break;
    case '/': c = ' '; break;
    case '&': c = ' '; break;
    case '@': c = ' '; break;
    case '%': c = ' '; break;
    case '.': c = ' '; break;
    case ',': c = ' '; break;
    case ';': c = ' '; break;
    case ':': c = ' '; break;
    case '"': c = ' '; break;
    case '\'': c = ' '; break;
    case '\\': c = ' '; break;
    case ' ': c = ' '; break;
    case '\r': c = ' '; break;
    case '\n': c = ' '; break;
    case '\t': c = ' '; break;
    default: c = '*'; break;
    }
    out[k++] = c;
    if (c == '*') {
      k--;
    }
  }
  out[k++] = '\0';
}

/* removes vowels etc. */
void boil(char *out, char *conv) {
  strtolower(out, conv);
  clamp_src(conv, out);
  boil_special(out, conv);
  boil_numbers(conv, out);
  boil_vowels(out, conv);
}

/* just tidy html/xml */
void tidy(char *out, char *conv) {
  strtolower(out, conv);
  clamp_src(conv, out);
  strcpy(out, conv);
}

/* find right index space (for one word) */
int cidx(const char *sz) {
  char *p = 0;
  int n = 0;
  int radix = strlen(pattern);
  for (int i = 0; sz[i] && i < WLEN; ++i) {
    p = strchr(pattern, sz[i]);
    if (p) {
      int c0 = p - pattern;
      n = n * radix + c0;
    }
  }
  int mn = n * PLZ % (SIZ * PLZ);
  /* else: there is *no* sense in indexing single letters or numbers, two vowels unimportant, skip */
  return mn;
}

/* index all words per webpage p (mind the offset!) */
void windex(const cr_t *p, unsigned long int offset, unsigned int *crwldb) {
  if (p && offset >= 0 && p->read > 0) {
    int slot = p - idx; /* auto-detecting slot (=1st. entry) */
    char conv[BLOCK] = {'\0'};
    char boiled[BLOCK] = {'\0'};
    char *token = 0;
    bzero(done, SIZ);
    strncpy(conv, p->data, BLOCK - 1);
    boil(boiled, conv);
    for (token = strtok(boiled, " "); token; token = strtok(NULL, " ")) {
      int toxel = cidx(token);
      int ratio = PLZ;
      /* keep first PLZ entries empty for future use */
      for (int rem = 0; rem < ratio && toxel > 0 && !done[toxel / PLZ]; ++rem) {
	/* linear search for free slot */
	unsigned int *ptr = crwldb + toxel + rem;
	if (*ptr == 0) { /* fillable? */
	  /* this could be a subsite! */
	  *ptr = (unsigned int)(slot + offset);
	  break;
	}
      }
      done[toxel / PLZ] = TRUE;
    }
  }
}
