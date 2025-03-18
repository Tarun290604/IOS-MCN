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
#include "ves.h"

typedef struct ves_common_header {
  ves_info_t info;
  int seq_id;
} ves_common_header_t;

extern const config_t *ves_config;
extern ves_common_header_t ves_common_header;

int ves_execute(const char *content, const char *domain, const char *event_type, const char *priority);
int ves_vsftp_daemon_init(void);
int ves_vsftp_daemon_deinit(void);
int ves_sftp_daemon_init(void);
int ves_sftp_daemon_deinit(void);
