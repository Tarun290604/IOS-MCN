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

#include "ves.h"
#include "ves_internal.h"

#include "common/utils.h"
#include "common/log.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static char *ves_template_new_alarm = 0;
static char *ves_template_clear_alarm = 0;
static char *ves_template_pnf_registration = 0;
static char *ves_template_file_ready = 0;
static char *ves_template_heartbeat = 0;

static bool ves_pnf_registration_sent = false;
static long int ves_heartbeat_trigger_timestamp = -1;

/**
 * initialize ves component
 *   save config internally
 *   prepare ves common header as far as possible
 */
int ves_init(const config_t *config)
{
  int rc;

  if (config == 0) {
    log_error("no ves config");
    goto failed;
  }

  ves_config = config;

  if (!config->ves.url || !config->ves.username || !config->ves.password) {
    log_error("ves config url/username/pass error");
    goto failed;
  }

  // rc = ves_vsftp_daemon_init();
  // if(rc != 0) {
  //     log_error("vsftp_daemon_init() failed");
  //     goto failed;
  // }

  rc = ves_sftp_daemon_init();
  if (rc != 0) {
    log_error("sftp_daemon_init() failed");
    goto failed;
  }

  ves_template_new_alarm = file_read_content(config->ves.template.new_alarm);
  if (ves_template_new_alarm == 0) {
    log_error("ves_template_new_alarm failed");
    goto failed;
  }

  ves_template_clear_alarm = file_read_content(config->ves.template.clear_alarm);
  if (ves_template_clear_alarm == 0) {
    log_error("ves_template_clear_alarm failed");
    goto failed;
  }

  ves_template_pnf_registration = file_read_content(config->ves.template.pnf_registration);
  if (ves_template_pnf_registration == 0) {
    log_error("ves_template_pnf_registration failed");
    goto failed;
  }

  ves_template_file_ready = file_read_content(config->ves.template.file_ready);
  if (ves_template_file_ready == 0) {
    log_error("ves_template_file_ready failed");
    goto failed;
  }

  ves_template_heartbeat = file_read_content(config->ves.template.heartbeat);
  if (ves_template_heartbeat == 0) {
    log_error("ves_template_heartbeat failed");
    goto failed;
  }

  if (config->ves.heartbeat_interval != -1) {
    ves_heartbeat_trigger_timestamp = get_seconds_since_epoch() + config->ves.heartbeat_interval;
  }

  return 0;

failed:
  ves_free();

  return 1;
}

int ves_set_info(const ves_info_t *info)
{
  if (info->managed_element_id) {
    free(ves_common_header.info.managed_element_id);
    ves_common_header.info.managed_element_id = strdup(info->managed_element_id);
    if (ves_common_header.info.managed_element_id == 0) {
      log_error("strdup failed");
      goto failure;
    }
  }

  if (info->vendor) {
    free(ves_common_header.info.vendor);
    ves_common_header.info.vendor = strdup(info->vendor);
    if (ves_common_header.info.vendor == 0) {
      log_error("strdup failed");
      goto failure;
    }
  }

  return 0;
failure:
  return 1;
}

void ves_free()
{
  // ves_vsftp_daemon_deinit();
  ves_sftp_daemon_deinit();

  free(ves_common_header.info.managed_element_id);
  ves_common_header.info.managed_element_id = 0;
  free(ves_common_header.info.vendor);
  ves_common_header.info.vendor = 0;
  memset(&ves_common_header, 0, sizeof(ves_common_header_t));

  free(ves_template_new_alarm);
  ves_template_new_alarm = 0;
  free(ves_template_clear_alarm);
  ves_template_clear_alarm = 0;
  free(ves_template_pnf_registration);
  ves_template_pnf_registration = 0;
  free(ves_template_file_ready);
  ves_template_file_ready = 0;
  free(ves_template_heartbeat);
  ves_template_heartbeat = 0;

  ves_config = 0;
}

void ves_loop()
{
  int rc;

  if (ves_common_header.info.vendor && ves_common_header.info.managed_element_id) {
    // pnf registration
    if (ves_config->ves.pnf_registration && !ves_pnf_registration_sent) {
      ves_pnf_registration_data_t data = {
          .mac_address = "00:00:00:00:00:00", // checkAL
          .ip_v6_address = "" // checkAL
      };

      rc = ves_pnf_registration_execute(&data);
      if (rc != 0) {
        log_error("ves_pnf_registration_execute() failed");
      }
      ves_pnf_registration_sent = true;
    }

    // heartbeat
    if (ves_heartbeat_trigger_timestamp != -1) {
      long int current_timestamp = get_seconds_since_epoch();

      if (current_timestamp >= ves_heartbeat_trigger_timestamp) {
        ves_heartbeat_trigger_timestamp = current_timestamp + ves_config->ves.heartbeat_interval;
        rc = ves_heartbeat_execute();
        if (rc != 0) {
          log_error("ves_heartbeat_execute() failed");
        }
      }
    }
  }
}

int ves_pnf_registration_execute(const ves_pnf_registration_data_t *data)
{
  char *content = 0;
  int rc;

  if (data == 0) {
    log_error("data is null");
    goto failed;
  }

  if (!(ves_common_header.info.vendor && ves_common_header.info.managed_element_id)) {
    log_error("unset VES information");
    goto failed;
  }

  char *domain = "pnfRegistration";
  char *event_type = "OAI_pnfRegistration";
  char *priority = "Low";

  content = strdup(ves_template_pnf_registration);
  if (content == 0) {
    log_error("strdup failed");
    goto failed;
  }

  content = str_replace_inplace(content, "@oamIp@", ves_config->network.host);
  if (content == 0) {
    log_error("str_replace_inplace() failed");
    goto failed;
  }

  char portstr[8];
  sprintf(portstr, "%d", ves_config->network.netconf_port);
  content = str_replace_inplace(content, "@port@", portstr);
  if (content == 0) {
    log_error("str_replace_inplace() failed");
    goto failed;
  }

  content = str_replace_inplace(content, "@username@", ves_config->network.username);
  if (content == 0) {
    log_error("str_replace_inplace() failed");
    goto failed;
  }

  content = str_replace_inplace(content, "@password@", ves_config->network.password);
  if (content == 0) {
    log_error("str_replace_inplace() failed");
    goto failed;
  }

  content = str_replace_inplace(content, "@macAddress@", data->mac_address);
  if (content == 0) {
    log_error("str_replace_inplace() failed");
    goto failed;
  }

  content = str_replace_inplace(content, "@model@", ves_config->info.model);
  if (content == 0) {
    log_error("str_replace_inplace() failed");
    goto failed;
  }

  content = str_replace_inplace(content, "@unitType@", ves_config->info.unit_type);
  if (content == 0) {
    log_error("str_replace_inplace() failed");
    goto failed;
  }

  if ((data->ip_v6_address == 0) || (data->ip_v6_address && (data->ip_v6_address[0] == 0))) {
    // no IP v6 set, must remove whole line
    content = str_remove_whole_line_inplace(content, "@oamIpV6@");
    if (content == 0) {
      log_error("str_replace_inplace() failed");
      goto failed;
    }

  } else {
    content = str_replace_inplace(content, "@oamIpV6@", data->ip_v6_address);
    if (content == 0) {
      log_error("str_replace_inplace() failed");
      goto failed;
    }
  }

  rc = ves_execute(content, domain, event_type, priority);
  if (rc != 0) {
    log_error("ves_execute() failed");
    goto failed;
  }

  free(content);
  content = 0;

  return rc;

failed:
  free(content);
  return 1;
}

int ves_fileready_execute(const ves_file_ready_t *data)
{
  char *fileExpiry = 0;
  char *content = 0;
  int rc;

  if (data == 0) {
    log_error("data is null");
    goto failed;
  }

  if (!(ves_common_header.info.vendor && ves_common_header.info.managed_element_id)) {
    log_error("unset VES information");
    goto failed;
  }

  char *domain = "stndDefined";
  char *event_type = "OAI_FileReady";
  char *priority = "Low";
  fileExpiry = get_netconf_timestamp_with_miliseconds(ves_config->ves.file_expiry);

  content = strdup(ves_template_file_ready);
  if (content == 0) {
    log_error("strdup failed");
    goto failed;
  }

  content = str_replace_inplace(content, "@model@", ves_config->info.model);
  if (content == 0) {
    log_error("str_replace_inplace() failed");
    goto failed;
  }

  content = str_replace_inplace(content, "@fileExpiry@", fileExpiry);
  if (content == 0) {
    log_error("str_replace_inplace() failed");
    goto failed;
  }

  content = str_replace_inplace(content, "@fileLocation@", data->file_location);
  if (content == 0) {
    log_error("str_replace_inplace() failed");
    goto failed;
  }

  char file_size[32];
  sprintf(file_size, "%d", data->file_size);
  content = str_replace_inplace(content, "@fileSize@", file_size);
  if (content == 0) {
    log_error("str_replace_inplace() failed");
    goto failed;
  }

  content = str_replace_inplace(content, "@oamIp@", ves_config->network.host);
  if (content == 0) {
    log_error("str_replace_inplace() failed");
    goto failed;
  }

  char portstr[8];
  sprintf(portstr, "%d", ves_config->network.sftp_port);
  content = str_replace_inplace(content, "@port@", portstr);
  if (content == 0) {
    log_error("str_replace_inplace() failed");
    goto failed;
  }

  content = str_replace_inplace(content, "@username@", ves_config->network.username);
  if (content == 0) {
    log_error("str_replace_inplace() failed");
    goto failed;
  }

  content = str_replace_inplace(content, "@password@", ves_config->network.password);
  if (content == 0) {
    log_error("str_replace_inplace() failed");
    goto failed;
  }

  char notification_id[8];
  sprintf(notification_id, "%d", data->notification_id);
  content = str_replace_inplace(content, "@notification-id@", notification_id);
  if (content == 0) {
    log_error("str_replace_inplace() failed");
    goto failed;
  }

  rc = ves_execute(content, domain, event_type, priority);
  if (rc != 0) {
    log_error("ves_execute() failed");
    goto failed;
  }

  free(content);
  content = 0;
  free(fileExpiry);
  fileExpiry = 0;

  return rc;

failed:
  free(fileExpiry);
  free(content);
  return 1;
}

int ves_alarm_new_execute(const ves_alarm_t *data)
{
  char *content = 0;
  int rc;

  if (data == 0) {
    log_error("data is null");
    goto failed;
  }

  if (!(ves_common_header.info.vendor && ves_common_header.info.managed_element_id)) {
    log_error("unset VES information");
    goto failed;
  }

  char *domain = "stndDefined";
  char *event_type = "OAI_Alarm";
  char *priority = "Low";

  content = strdup(ves_template_new_alarm);
  if (content == 0) {
    log_error("strdup failed");
    goto failed;
  }

  content = str_replace_inplace(content, "@alarm@", data->alarm);
  if (content == 0) {
    log_error("str_replace_inplace() failed");
    goto failed;
  }

  content = str_replace_inplace(content, "@severity@", data->severity);
  if (content == 0) {
    log_error("str_replace_inplace() failed");
    goto failed;
  }

  content = str_replace_inplace(content, "@alarm-type@", data->type);
  if (content == 0) {
    log_error("str_replace_inplace() failed");
    goto failed;
  }

  content = str_replace_inplace(content, "@object-instance@", data->object_instance);
  if (content == 0) {
    log_error("str_replace_inplace() failed");
    goto failed;
  }

  char notification_id[8];
  sprintf(notification_id, "%d", data->notification_id);
  content = str_replace_inplace(content, "@notification-id@", notification_id);
  if (content == 0) {
    log_error("str_replace_inplace() failed");
    goto failed;
  }

  rc = ves_execute(content, domain, event_type, priority);
  if (rc != 0) {
    log_error("ves_execute() failed");
    goto failed;
  }

  free(content);
  content = 0;

  return rc;

failed:
  free(content);
  return 1;
}

int ves_alarm_clear_execute(const ves_alarm_t *data)
{
  char *content = 0;
  int rc;

  if (data == 0) {
    log_error("data is null");
    goto failed;
  }

  if (!(ves_common_header.info.vendor && ves_common_header.info.managed_element_id)) {
    log_error("unset VES information");
    goto failed;
  }

  char *domain = "stndDefined";
  char *event_type = "OAI_Alarm";
  char *priority = "Low";

  content = strdup(ves_template_clear_alarm);
  if (content == 0) {
    log_error("strdup failed");
    goto failed;
  }

  content = str_replace_inplace(content, "@alarm@", data->alarm);
  if (content == 0) {
    log_error("str_replace_inplace() failed");
    goto failed;
  }

  content = str_replace_inplace(content, "@severity@", data->severity);
  if (content == 0) {
    log_error("str_replace_inplace() failed");
    goto failed;
  }

  content = str_replace_inplace(content, "@alarm-type@", data->type);
  if (content == 0) {
    log_error("str_replace_inplace() failed");
    goto failed;
  }

  content = str_replace_inplace(content, "@object-instance@", data->object_instance);
  if (content == 0) {
    log_error("str_replace_inplace() failed");
    goto failed;
  }

  char notification_id[8];
  sprintf(notification_id, "%d", data->notification_id);
  content = str_replace_inplace(content, "@notification-id@", notification_id);
  if (content == 0) {
    log_error("str_replace_inplace() failed");
    goto failed;
  }

  rc = ves_execute(content, domain, event_type, priority);
  if (rc != 0) {
    log_error("ves_execute() failed");
    goto failed;
  }

  free(content);
  content = 0;

  return rc;

failed:
  free(content);
  return 1;
}

int ves_heartbeat_execute()
{
  char *content = 0;
  int rc;

  if (!(ves_common_header.info.vendor && ves_common_header.info.managed_element_id)) {
    log_error("unset VES information");
    goto failed;
  }

  char *domain = "heartbeat";
  char *event_type = "OAI_HeartBeat";
  char *priority = "Low";

  content = strdup(ves_template_heartbeat);
  if (content == 0) {
    log_error("strdup failed");
    goto failed;
  }

  char heartbeat_interval[8];
  sprintf(heartbeat_interval, "%d", ves_config->ves.heartbeat_interval);
  content = str_replace_inplace(content, "@heartbeat-interval@", heartbeat_interval);
  if (content == 0) {
    log_error("str_replace_inplace() failed");
    goto failed;
  }

  rc = ves_execute(content, domain, event_type, priority);
  if (rc != 0) {
    log_error("ves_execute() failed");
    goto failed;
  }

  free(content);
  content = 0;

  return rc;

failed:
  free(content);
  return 1;
}
