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

#include "pm_data.h"
#include "common/config.h"
#include "common/log.h"
#include "common/utils.h"
#include "ves/ves.h"

#include <string.h>
#include <stdlib.h>
#include <time.h>

#define PM_DATA_PATH "/ftp"

// pm data log rotate command; 1440 min = 24 h rotation
#define PM_DATA_ROLL_COMMAND "find " PM_DATA_PATH " -mindepth 1 -mmin +1440 -delete"

static int pm_data_feed_period = 1;
static int pm_data_feed_log_period = 15 * 60; // default 900 sec
static int pm_data_accumulator_size = 1;

static char *ves_template_pm_data = 0;

static pm_data_info_t pm_data_info = {0};

static pm_data_t *pm_data_accumulator = 0;
static int pm_data_accumulator_len = 0;
static time_t pm_data_start_time = 0;

static const config_t *pm_data_config = 0;
static int pm_data_notification_id = 1;

typedef struct pm_write_data {
  long int start_time;
  long int end_time;
  char *filename;
  int meanActiveUe;
  int maxActiveUe;
  int loadAvg;
  long int ue_thp_dl;
  long int ue_thp_ul;
} pm_write_data_t;

static int pm_data_write(pm_write_data_t *data);

int pm_data_init(const config_t *config)
{
  pm_data_feed_log_period = config->ves.pm_data_interval;
  pm_data_accumulator_size = pm_data_feed_log_period / pm_data_feed_period + 1;
  pm_data_accumulator = (pm_data_t *)malloc(sizeof(pm_data_t) * pm_data_accumulator_size);
  if (pm_data_accumulator == 0) {
    log_error("malloc failed");
    goto failure;
  }

  pm_data_accumulator_len = 0;
  pm_data_start_time = time(0);
  if (pm_data_start_time == -1) {
    log_error("time failed");
    goto failure;
  }

  pm_data_config = config;
  pm_data_notification_id = 1;

  ves_template_pm_data = file_read_content(config->ves.template.pm_data);
  if (ves_template_pm_data == 0) {
    log_error("ves_template_pm_data failed");
    goto failure;
  }

  return 0;

failure:
  free(pm_data_accumulator);
  pm_data_accumulator = 0;

  free(ves_template_pm_data);
  ves_template_pm_data = 0;
  return 1;
}

int pm_data_set_info(const pm_data_info_t *info)
{
  if (info->vendor) {
    free(pm_data_info.vendor);
    pm_data_info.vendor = strdup(info->vendor);
    if (pm_data_info.vendor == 0) {
      log_error("strdup failed");
      goto failure;
    }
  }

  return 0;
failure:
  return 1;
}

int pm_data_free()
{
  free(pm_data_accumulator);
  pm_data_accumulator = 0;
  free(ves_template_pm_data);
  ves_template_pm_data = 0;
  free(pm_data_info.vendor);
  pm_data_info.vendor = 0;

  pm_data_accumulator_len = 0;

  return 0;
}

void pm_data_loop()
{
  time_t timestamp = time(0);
  if (timestamp == -1) {
    return;
  }

  if (!(pm_data_info.vendor)) {
    return;
  }

  if ((pm_data_accumulator_len) && ((timestamp / pm_data_feed_log_period) != (pm_data_start_time / pm_data_feed_log_period))) {
    int rc;
    char *filename = 0;
    char *full_path = 0;

    long int meanActiveUeAccum = 0;
    int maxActiveUe = 0;
    int loadAvgAccum = 0;
    long int ue_thp_dl_accum = 0;
    long int ue_thp_ul_accum = 0;

    for (int i = 0; i < pm_data_accumulator_len; i++) {
      meanActiveUeAccum += pm_data_accumulator[i].numUes;
      if (pm_data_accumulator[i].numUes > maxActiveUe) {
        maxActiveUe = pm_data_accumulator[i].numUes;
      }
      loadAvgAccum += pm_data_accumulator[i].load;

      ue_thp_dl_accum += pm_data_accumulator[i].ue_thp_dl_sum;
      ue_thp_ul_accum += pm_data_accumulator[i].ue_thp_ul_sum;
    }
    int meanActiveUe = meanActiveUeAccum / pm_data_accumulator_len;
    int loadAvg = loadAvgAccum / pm_data_accumulator_len;
    long int ue_thp_dl = ue_thp_dl_accum / pm_data_accumulator_len;
    long int ue_thp_ul = ue_thp_ul_accum / pm_data_accumulator_len;

    struct tm *ptm = gmtime(&pm_data_start_time);
    if (ptm == 0) {
      log_error("gmtime error");
      goto failure_loop;
    }

    struct tm start_ptm;
    memcpy(&start_ptm, ptm, sizeof(struct tm));

    ptm = gmtime(&timestamp);
    if (ptm == 0) {
      log_error("gmtime error");
      goto failure_loop;
    }

    struct tm now_ptm;
    memcpy(&now_ptm, ptm, sizeof(struct tm));

    asprintf(&filename,
             "A%04d%02d%02d.%02d%02d+0000-%02d%02d+0000_1_%s.xml",
             start_ptm.tm_year + 1900,
             start_ptm.tm_mon + 1,
             start_ptm.tm_mday,
             start_ptm.tm_hour,
             start_ptm.tm_min,
             now_ptm.tm_hour,
             now_ptm.tm_min,
             pm_data_config->info.node_id);
    if (filename == 0) {
      log_error("asprintf error");
      goto failure_loop;
    }

    asprintf(&full_path, "%s/%s", PM_DATA_PATH, filename);
    if (full_path == 0) {
      log_error("asprintf error");
      goto failure_loop;
    }

    pm_write_data_t data = {
        .start_time = pm_data_start_time,
        .end_time = timestamp,
        .filename = full_path,
        .meanActiveUe = meanActiveUe,
        .maxActiveUe = maxActiveUe,
        .loadAvg = loadAvg,
        .ue_thp_dl = ue_thp_dl,
        .ue_thp_ul = ue_thp_ul,
    };

    rc = pm_data_write(&data);
    if (rc != 0) {
      log_error("pm_data_write error");
      goto failure_loop;
    }

    // cleanup
    pm_data_start_time = timestamp;
    pm_data_accumulator_len = 0;
    rc = system(PM_DATA_ROLL_COMMAND);
    if (rc != 0) {
      log_error("system error");
      goto failure_loop;
    }

    // send ves message
    ves_file_ready_t file_ready = {
        .file_location = full_path,
        .file_size = get_file_size(full_path),
        .notification_id = pm_data_notification_id,
    };

    rc = ves_fileready_execute(&file_ready);
    if (rc != 0) {
      log_error("ves_fileready_execute error");
      goto failure_loop;
    }

    pm_data_notification_id++;

  failure_loop:
    free(full_path);
    free(filename);
  }
}

int pm_data_feed(const pm_data_t *pm_data)
{
  if (pm_data == 0) {
    log_error("pm_data is null");
    goto failure;
  }

  memcpy(&pm_data_accumulator[pm_data_accumulator_len], pm_data, sizeof(pm_data_t));
  pm_data_accumulator_len++;
  if (pm_data_accumulator_len >= pm_data_accumulator_size) {
    pm_data_accumulator_len = 0;
  }

  return 0;

failure:
  return 1;
}

static int pm_data_write(pm_write_data_t *data)
{
  char *content = 0;
  FILE *f = 0;

  f = fopen(data->filename, "w");
  if (f == 0) {
    log_error("fopen failed");
    goto failure;
  }

  content = strdup(ves_template_pm_data);
  if (content == 0) {
    log_error("strdup() failed");
    goto failure;
  }

  char start_time_full[64];
  char end_time_full[64];

  time_t *timestamp = (time_t *)&(data->start_time);
  struct tm *ptm = gmtime(timestamp);
  if (ptm == 0) {
    log_error("gmtime error");
    goto failure;
  }
  sprintf(start_time_full,
          "%04d-%02d-%02dT%02d:%02d:%02d+00:00",
          ptm->tm_year + 1900,
          ptm->tm_mon + 1,
          ptm->tm_mday,
          ptm->tm_hour,
          ptm->tm_min,
          ptm->tm_sec);

  timestamp = (time_t *)&(data->end_time);
  ptm = gmtime(timestamp);
  if (ptm == 0) {
    log_error("gmtime error");
    goto failure;
  }
  sprintf(end_time_full,
          "%04d-%02d-%02dT%02d:%02d:%02d+00:00",
          ptm->tm_year + 1900,
          ptm->tm_mon + 1,
          ptm->tm_mday,
          ptm->tm_hour,
          ptm->tm_min,
          ptm->tm_sec);

  const char *suspect = "";
  if ((data->end_time - data->start_time) < pm_data_feed_log_period) {
    suspect = "<suspect>true</suspect>";
  }

  char log_period[16];
  sprintf(log_period, "%d", pm_data_feed_log_period);
  content = str_replace_inplace(content, "@log-period@", log_period);
  if (content == 0) {
    log_error("str_replace_inplace() failed");
    goto failure;
  }

  content = str_replace_inplace(content, "@suspect@", suspect);
  if (content == 0) {
    log_error("str_replace_inplace() failed");
    goto failure;
  }

  content = str_replace_inplace(content, "@vendor@", pm_data_info.vendor);
  if (content == 0) {
    log_error("str_replace_inplace() failed");
    goto failure;
  }

  content = str_replace_inplace(content, "@node-id@", pm_data_config->info.node_id);
  if (content == 0) {
    log_error("str_replace_inplace() failed");
    goto failure;
  }

  content = str_replace_inplace(content, "@start-time@", start_time_full);
  if (content == 0) {
    log_error("str_replace_inplace() failed");
    goto failure;
  }

  content = str_replace_inplace(content, "@end-time@", end_time_full);
  if (content == 0) {
    log_error("str_replace_inplace() failed");
    goto failure;
  }

  char mean_active_ue_str[32];
  char max_active_ue_str[32];
  char load_avg_str[8];
  char ue_thp_dl_str[10];
  char ue_thp_ul_str[10];

  sprintf(mean_active_ue_str, "%d", data->meanActiveUe);
  sprintf(max_active_ue_str, "%d", data->maxActiveUe);
  sprintf(load_avg_str, "%d", data->loadAvg);
  sprintf(ue_thp_dl_str, "%ld", data->ue_thp_dl);
  sprintf(ue_thp_ul_str, "%ld", data->ue_thp_ul);

  content = str_replace_inplace(content, "@mean-active-ue@", mean_active_ue_str);
  if (content == 0) {
    log_error("str_replace_inplace() failed");
    goto failure;
  }

  content = str_replace_inplace(content, "@max-active-ue@", max_active_ue_str);
  if (content == 0) {
    log_error("str_replace_inplace() failed");
    goto failure;
  }

  content = str_replace_inplace(content, "@load-avg@", load_avg_str);
  if (content == 0) {
    log_error("str_replace_inplace() failed");
    goto failure;
  }

  char gnb_du_id[16];
  sprintf(gnb_du_id, "%d", pm_data_config->info.gnb_du_id);
  content = str_replace_inplace(content, "@du-id@", gnb_du_id);
  if (content == 0) {
    log_error("str_replace_inplace() failed");
    goto failure;
  }

  char cell_local_id[16];
  sprintf(cell_local_id, "%d", pm_data_config->info.cell_local_id);
  content = str_replace_inplace(content, "@cell-id@", cell_local_id);
  if (content == 0) {
    log_error("str_replace_inplace() failed");
    goto failure;
  }

  content = str_replace_inplace(content, "@load-avg@", load_avg_str);
  if (content == 0) {
    log_error("str_replace_inplace() failed");
    goto failure;
  }

  content = str_replace_inplace(content, "@ue-thp-dl@", ue_thp_dl_str);
  if (content == 0) {
    log_error("str_replace_inplace() failed");
    goto failure;
  }

  content = str_replace_inplace(content, "@ue-thp-ul@", ue_thp_ul_str);
  if (content == 0) {
    log_error("str_replace_inplace() failed");
    goto failure;
  }

  fprintf(f, "%s", content);
  fclose(f);

  free(content);
  content = 0;

  return 0;

failure:
  if (f) {
    fclose(f);
  }

  free(content);
  content = 0;

  return 1;
}
