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

#include <stdbool.h>

extern int log_level;

typedef struct config_ves {
  struct {
    char *new_alarm;
    char *clear_alarm;
    char *pnf_registration;
    char *file_ready;
    char *heartbeat;
    char *pm_data;
  } template;

  bool pnf_registration;
  int heartbeat_interval;

  char *url;
  char *username;
  char *password;

  int file_expiry;
  int pm_data_interval;
} config_ves_t;

typedef struct config {
  int log_level;
  char *software_version;

  struct {
    char *host;
    char *username;
    char *password;

    int netconf_port;
    int sftp_port;
  } network;

  config_ves_t ves;

  struct {
    int internal_connection_lost_timeout;

    int load_downlink_exceeded_warning_threshold;
    int load_downlink_exceeded_warning_timeout;
  } alarms;

  struct {
    char *host;
    int port;
  } telnet;

  struct {
    int gnb_du_id;
    int gnb_cu_id;
    int cell_local_id;
    char *node_id;
    char *location_name;
    char *managed_by;
    char *managed_element_type;
    char *model;
    char *unit_type;
  } info;

} config_t;

int config_init(const char *filename);
config_t *config_get();
void config_free();

void config_print(const config_t *config);
