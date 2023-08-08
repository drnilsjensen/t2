#ifndef MYDEFS_H_
#define MYDEFS_H_

#define KB (1024)

#define WLEN  (5)
/* strlen pattern = 13
   SIZ = 13^WLEN */
#define SIZ   (13*13*13*13*13)
/* free slots per index entry */
#define PLZ   (500)
/* # urls per crawl */
#define NUM   (1000)
/* crawl *just* 4K, please */
#define BLOCK (4*KB)
/* timeout */
#define SEC   (10)
/* default string, terminating null *included* */
#define SLEN  (80)
/* do crawls in parallel */
#define MAX_PARALLEL (20)
/* http(s) prefix */
#define HTTPHEAD "http://www."

#define DEEP (127)

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define FALSE (0)
#define TRUE  (1)

typedef struct cr {
  int read;
  char url[SLEN+1];
  char *data;
} cr_t;

#endif
