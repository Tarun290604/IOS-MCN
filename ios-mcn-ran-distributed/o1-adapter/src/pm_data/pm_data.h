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

typedef struct pm_data {
  int numUes;
  int load;
  long int ue_thp_dl_sum;
  long int ue_thp_ul_sum;
} pm_data_t;

typedef struct pm_data_info {
  char *vendor;
} pm_data_info_t;

int pm_data_init(const config_t *config);
int pm_data_set_info(const pm_data_info_t *info);
int pm_data_free();
void pm_data_loop();

int pm_data_feed(const pm_data_t *pm_data);
