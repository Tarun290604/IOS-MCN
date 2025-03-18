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
#include "common/config.h"

typedef struct ves_info {
  char *vendor;
  char *managed_element_id; // string containing a number
} ves_info_t;

typedef struct ves_pnf_registration_data {
  char *mac_address;
  char *ip_v6_address; // can be null or empty
} ves_pnf_registration_data_t;

typedef struct ves_file_ready {
  char *file_location;
  int file_size;
  int notification_id;
} ves_file_ready_t;

typedef struct ves_alarm {
  char *alarm; //@alarm@ number-of-ue-exceeded
  char *severity; //@severity@ MINOR
  char *type; //@alarm-type@ COMMUNICATION_ALARM
  char *object_instance;
  int notification_id;
} ves_alarm_t;

int ves_init(const config_t *config);
int ves_set_info(const ves_info_t *info);
void ves_free();
void ves_loop();

int ves_pnf_registration_execute(const ves_pnf_registration_data_t *data);
int ves_fileready_execute(const ves_file_ready_t *data);
int ves_alarm_new_execute(const ves_alarm_t *data);
int ves_alarm_clear_execute(const ves_alarm_t *data);
int ves_heartbeat_execute();
