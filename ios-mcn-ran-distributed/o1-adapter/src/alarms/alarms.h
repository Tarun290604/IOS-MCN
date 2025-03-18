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

#include "common/config.h"

typedef enum alarm_state {
  // state
  ALARM_STATE_CLEARED = 0,
  ALARM_STATE_RAISED,

  // command
  ALARM_STATE_CLEAR,
  ALARM_STATE_RAISE,
} alarm_state_t;

typedef enum alarm_severity {
  ALARM_SEVERITY_CLEARED = 0, // never set to cleared
  ALARM_SEVERITY_INDETERMINATE,
  ALARM_SEVERITY_WARNING,
  ALARM_SEVERITY_MINOR,
  ALARM_SEVERITY_MAJOR,
  ALARM_SEVERITY_CRITICAL,
} alarm_severity_t;

typedef enum alarm_type {
  ALARM_TYPE_COMMUNICATIONS_ALARM = 0,
  ALARM_TYPE_QUALITY_OF_SERVICE_ALARM,
  ALARM_TYPE_PROCESSING_ERROR_ALARM,
  ALARM_TYPE_EQUIPMENT_ALARM,
  ALARM_TYPE_ENVIRONMENTAL_ALARM,
  ALARM_TYPE_INTEGRITY_VIOLATION,
  ALARM_TYPE_OPERATIONAL_VIOLATION,
  ALARM_TYPE_PHYSICAL_VIOLATION,
  ALARM_TYPE_SECURITY_SERVICE_OR_MECHANISM_VIOLATION,
  ALARM_TYPE_TIME_DOMAIN_VIOLATION,
} alarm_type_t;

typedef struct alarm {
  char *alarm;
  alarm_severity_t severity;
  alarm_type_t type;
  char *object_instance;

  alarm_state_t state;
  int timeout;
} alarm_t;

typedef struct alarms_data {
  int num_du;
  int load;
} alarms_data_t;

int alarms_init(const config_t *config);
int alarms_free();
void alarms_loop();

const char *alarm_severity_to_str(const alarm_severity_t severity);
const char *alarm_type_to_str(const alarm_type_t type);

void alarms_on_telnet_connected();
void alarms_on_telnet_disconnected();
int alarms_data_feed(const alarms_data_t *alarms_data);
