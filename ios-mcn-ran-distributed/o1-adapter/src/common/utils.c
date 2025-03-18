/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under
 * the OAI Public License, Version 1.1  (the "License"); you may not use this file
 * except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.openairinterface.org/?page_id=698
 *
 * Copyright: Fraunhofer Heinrich Hertz Institute
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *-------------------------------------------------------------------------------
 * For more information about the OpenAirInterface (OAI) Software Alliance:
 *      contact@openairinterface.org
 */

#define _GNU_SOURCE

#include "utils.h"
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "log.h"

long int get_seconds_since_epoch(void)
{
  time_t t = time(0);

  return (long int)t;
}

long int get_microseconds_since_epoch(void)
{
  time_t t = time(0);
  struct timeval tv;
  long int useconds;

  gettimeofday(&tv, 0);
  useconds = t * 1000000 + tv.tv_usec; // add the microseconds to the seconds

  return useconds;
}

char *get_netconf_timestamp(void)
{
  time_t rawtime = time(0);
  char *nctime = 0;

  if (rawtime == -1) {
    log_error("time() failed");
    return 0;
  } else {
    struct tm *ptm = gmtime(&rawtime);
    if (ptm == 0) {
      log_error("gmtime failed");
    } else {
      asprintf(&nctime,
               "%04d-%02d-%02dT%02d:%02d:%02d.0Z",
               ptm->tm_year + 1900,
               ptm->tm_mon + 1,
               ptm->tm_mday,
               ptm->tm_hour,
               ptm->tm_min,
               ptm->tm_sec);
    }
  }

  return nctime;
}

char *get_netconf_timestamp_with_miliseconds(int addSeconds)
{
  time_t rawtime = time(0);
  char *nctime = 0;

  if (rawtime == -1) {
    log_error("time() failed");
    return 0;
  } else {
    struct tm *ptm = gmtime(&rawtime);
    if (ptm == 0) {
      log_error("gmtime failed");
    } else {
      struct timeval tv;
      ptm->tm_sec += addSeconds;
      mktime(ptm);
      gettimeofday(&tv, 0);
      asprintf(&nctime,
               "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ",
               ptm->tm_year + 1900,
               ptm->tm_mon + 1,
               ptm->tm_mday,
               ptm->tm_hour,
               ptm->tm_min,
               ptm->tm_sec,
               (int)(tv.tv_usec / 1000));
    }
  }

  return nctime;
}

int put_human_timestamp(char *nctime)
{
  time_t rawtime = time(0);

  if (rawtime == -1) {
    log_error("time() failed");
    return 1;
  } else {
    struct tm *ptm = gmtime(&rawtime);
    if (ptm == 0) {
      log_error("gmtime failed");
      return 1;
    } else {
      sprintf(nctime,
              "%04d-%02d-%02dT%02d:%02d:%02d",
              ptm->tm_year + 1900,
              ptm->tm_mon + 1,
              ptm->tm_mday,
              ptm->tm_hour,
              ptm->tm_min,
              ptm->tm_sec);
    }
  }

  return 0;
}

char *str_replace(const char *orig, const char *rep, const char *with)
{
  char *result; // the return string
  const char *ins; // the next insert point
  char *tmp; // varies
  int len_rep; // length of rep (the string to remove)
  int len_with; // length of with (the string to replace rep with)
  int len_front; // distance between rep and end of last rep
  int count; // number of replacements

  // sanity checks and initialization
  if (!orig || !rep) {
    return 0;
  }

  len_rep = strlen(rep);
  if (len_rep == 0) {
    return 0; // empty rep causes infinite loop during count
  }

  if (!with) {
    with = "";
  }
  len_with = strlen(with);

  // count the number of replacements needed
  ins = orig;
  for (count = 0; (tmp = strstr(ins, rep)); ++count) {
    ins = tmp + len_rep;
  }

  tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

  if (!result) {
    return 0;
  }

  // first time through the loop, all the variable are set correctly
  // from here on,
  //    tmp points to the end of the result string
  //    ins points to the next occurrence of rep in orig
  //    orig points to the remainder of orig after "end of rep"
  while (count--) {
    ins = strstr(orig, rep);
    len_front = ins - orig;
    tmp = strncpy(tmp, orig, len_front) + len_front;
    tmp = strcpy(tmp, with) + len_with;
    orig += len_front + len_rep; // move to next "end of rep"
  }

  strcpy(tmp, orig);
  return result;
}

char *str_replace_inplace(char *s, const char *rep, const char *with)
{
  char *ret = str_replace(s, rep, with);
  free(s);
  if (ret == 0) {
    return 0;
  }
  return ret;
}

static int isEol(char c)
{
  switch (c) {
    case '\r':
    case '\n':
      return 1;
  }

  return 0;
}

char *str_remove_whole_line_inplace(char *haystack, const char *needle)
{
  int length = strlen(haystack) + 1;

  char *where = strstr(haystack, needle);
  while (where) {
    char *begin = where;
    char *end = where;

    while ((begin > haystack) && !isEol(*begin)) {
      begin--;
    }

    if (isEol(*begin)) {
      begin++;
    }

    while (*end && !isEol(*end)) {
      end++;
    }

    int r = 0;
    int n = 0;
    while (*end && isEol(*end) && (r < 2) && (n < 2)) {
      if (*end == '\r') {
        r++;
      }

      if (*end == '\n') {
        n++;
      }
      end++;
    }

    char *result = (char *)malloc(sizeof(char) * length);
    if (result == 0) {
      return 0;
    }
    result[0] = 0;

    if (begin != haystack) {
      *begin = 0;
      strcat(result, haystack);
    }

    if (*end) {
      strcat(result, end);
    }

    free(haystack);
    haystack = result;
    where = strstr(haystack, needle);
  }

  haystack = (char *)realloc(haystack, sizeof(char) * (strlen(haystack) + 1));
  return haystack;
}

char *get_hostname(void)
{
  char hostname[1024];
  hostname[1023] = '\0';
  gethostname(hostname, 1023);
  return strdup(hostname);
}

unsigned long int get_file_size(const char *filename)
{
  struct stat st;
  stat(filename, &st);
  return st.st_size;
}

char *file_read_content(const char *fname)
{
  char *buffer = 0;
  long length;
  FILE *f = fopen(fname, "rb");
  if (f) {
    fseek(f, 0, SEEK_END);
    length = ftell(f);
    fseek(f, 0, SEEK_SET);
    buffer = (char *)malloc(sizeof(char) * (length + 1));
    if (buffer) {
      fread(buffer, 1, length, f);
    }
    fclose(f);
    buffer[length] = 0;
  }

  return buffer;
}
