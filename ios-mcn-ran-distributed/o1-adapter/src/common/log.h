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

#pragma once

#include <stdio.h>
#include "utils.h"
#include "config.h"

#define LOG_ENABLE
#define LOG_FILE "log.log"
#define LOG_START_MESSAGE "OAI-NETCONF started..."
#define LOG_MODE "w" // "a"pppend or over"w"rite

// DO NOT TOUCH BELOW
#if !defined(LOG_START_MESSAGE)
#error "LOG_START_MESSAGE should be defined"
#endif

#define __log_init()
#define __log_log(sec, usec, the_file, format, ...)
#define __log_error(sec, usec, the_file, format, ...)
#if defined(LOG_FILE)
#undef __log_init
#undef __log_log
#undef __log_error

#define __log_init()                     \
  {                                      \
    FILE *f = fopen(LOG_FILE, LOG_MODE); \
    fclose(f);                           \
  }

#define __log_log(sec, usec, the_file, format, ...)                          \
  {                                                                          \
    FILE *f = fopen(LOG_FILE, "a");                                          \
    fprintf(f, "[%lu.%03d/log][%-20s:%4d] ", sec, usec, the_file, __LINE__); \
    fprintf(f, format, ##__VA_ARGS__);                                       \
    fprintf(f, "\n");                                                        \
    fclose(f);                                                               \
  }

#define __log_error(sec, usec, the_file, format, ...)                        \
  {                                                                          \
    FILE *f = fopen(LOG_FILE, "a");                                          \
    fprintf(f, "[%lu.%03d/err][%-20s:%4d] ", sec, usec, the_file, __LINE__); \
    fprintf(f, format, ##__VA_ARGS__);                                       \
    fprintf(f, "\n");                                                        \
    fclose(f);                                                               \
  }
#endif

#if defined(LOG_ENABLE)
#define log_init()          \
  {                         \
    __log_init();           \
    log(LOG_START_MESSAGE); \
  }

#define log_error(format, ...)                                                                      \
  {                                                                                                 \
    long sec = get_microseconds_since_epoch();                                                      \
    int usec = (sec / 1000 % 1000);                                                                 \
    sec /= 1000000;                                                                                 \
    char ctime[128];                                                                                \
    if (put_human_timestamp(ctime) != 0)                                                            \
      sprintf(ctime, "%ld", sec);                                                                   \
    char *the_file = __FILE__;                                                                      \
    fprintf(stderr, "\033[1;31m[%s.%03d/err][%-20s:%4d]\033[0m ", ctime, usec, the_file, __LINE__); \
    fprintf(stderr, format, ##__VA_ARGS__);                                                         \
    fprintf(stderr, "\n");                                                                          \
    __log_error(sec, usec, the_file, format, ##__VA_ARGS__)                                         \
  }

#define log(format, ...)                                                                           \
  {                                                                                                \
    if (log_level > 0) {                                                                           \
      long sec = get_microseconds_since_epoch();                                                   \
      int usec = (sec / 1000 % 1000);                                                              \
      sec /= 1000000;                                                                              \
      char ctime[128];                                                                             \
      if (put_human_timestamp(ctime) != 0)                                                         \
        sprintf(ctime, "%ld", sec);                                                                \
      char *the_file = __FILE__;                                                                   \
      fprintf(stdout, "\033[1m[%s.%03d/log][%-20s:%4d]\033[0m ", ctime, usec, the_file, __LINE__); \
      fprintf(stdout, format, ##__VA_ARGS__);                                                      \
      fprintf(stdout, "\n");                                                                       \
      if (log_level > 2) {                                                                         \
        __log_log(sec, usec, the_file, format, ##__VA_ARGS__)                                      \
      }                                                                                            \
    }                                                                                              \
  }
#else
#define log_init()
#define log_error(format, args...)
#define log(format, args...)
#endif
