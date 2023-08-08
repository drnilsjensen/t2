#ifndef CURLHELPER_H_
#define CURLHELPER_H_

/* callback needed by libcurl */
static size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
  size_t realsize = size * nmemb;
  cr_t *p = (cr_t *)userdata;
  if (realsize > 0 && p && p->data && ptr) {
    int len = MIN(realsize, (BLOCK - 1 - (p->read)));
    if (len > 0) {
      memcpy(p->data + p->read, ptr, len);
      p->read += len;
      p->data[p->read] = '\0';
    }
  }
  return realsize; /* return offered size */
}

/* callback needed by libcurl */
static size_t write_callback100(char *ptr, size_t size, size_t nmemb, void *userdata) {
  size_t realsize = size * nmemb;
  cr_t *p = (cr_t *)userdata;
  if (realsize > 0 && p && p->data && ptr) {
    int len = MIN(realsize, (BLOCK*100 - 1 - (p->read)));
    if (len > 0) {
      memcpy(p->data + p->read, ptr, len);
      p->read += len;
      p->data[p->read] = '\0';
    }
  }
  return realsize; /* return offered size */
}

/* GET (with libcurl and flexible callback) */
static int xget(CURLM *cm, const char *sz, cr_t *to, void *write_callback) {
  CURL *curl = 0;
  CURLcode res = 0;
  curl = curl_easy_init();
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, sz);
    curl_easy_setopt(curl, CURLOPT_PRIVATE, to);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, to);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, SEC);
    if (!cm) { /* single, blocking */
      /* cm==0? Perform the request, res will get the return code */
      res = curl_easy_perform(curl);
      /* Check for errors */
      if(res != CURLE_OK) {
	fprintf(stderr, "curl_easy_perform() failed: %s\n",
		curl_easy_strerror(res));
      }
      curl_easy_cleanup(curl);
    } else { /* multi */
      curl_multi_add_handle(cm, curl);
    }
  }
  return 0;
}

/* GET (with libcurl) */
static int cget(CURLM *cm, const char *sz, cr_t *to) {
  return xget(cm, sz, to, write_callback);
}

#endif
