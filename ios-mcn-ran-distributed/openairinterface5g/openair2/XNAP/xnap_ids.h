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
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *-------------------------------------------------------------------------------
 * For more information about the OpenAirInterface (OAI) Software Alliance:
 *      contact@openairinterface.org
 */
#ifndef XNAP_IDS_H_
#define XNAP_IDS_H_
#include <stdio.h>
#include <stdint.h>

/* maximum number of simultaneous handovers, do not set too high */
#define XNAP_MAX_IDS 16

/*
 * state:
 * - when starting handover in source, UE is in state XNID_STATE_SOURCE_PREPARE
 * - after receiving HO_ack in source, UE is in state XNID_STATE_SOURCE_OVERALL
 * - in target, UE is in state XNID_STATE_TARGET
 * The state is used to check timers.
 */
typedef enum {
  XNID_STATE_SOURCE_PREPARE,
  XNID_STATE_SOURCE_OVERALL,
  XNID_STATE_TARGET,
  XNID_STATE_NSA_GNB_PREPARE,
  XNID_STATE_NSA_GNB_OVERALL,
} xnid_state_t;

typedef struct {
  int cu_ue_id; /* -1 when free */
  int id_source;
  int id_target;
  void *target;
  /* state: needed to check timers */
  xnid_state_t state;
  /* timers */
  uint64_t t_reloc_prep_start;
  uint64_t tx2_reloc_overall_start;
  uint64_t t_dc_prep_start;
  uint64_t t_dc_overall_start;
} xnap_id;

typedef struct {
  xnap_id ids[XNAP_MAX_IDS];
} xnap_id_manager;

void xnap_id_manager_init(xnap_id_manager *m);
int xnap_allocate_new_id(xnap_id_manager *m);
void xnap_release_id(xnap_id_manager *m, int id);
int xnap_find_id(xnap_id_manager *, int id_source, int id_target);
int xnap_find_id_from_id_source(xnap_id_manager *, int id_source);
int xnap_find_id_from_id_target(xnap_id_manager *, int id_source);
int xnap_find_id_from_rnti(xnap_id_manager *, int rnti);
void xnap_set_ids(xnap_id_manager *m, int ue_id, int rnti, int id_source, int id_target);
void xnap_id_set_state(xnap_id_manager *m, int ue_id, xnid_state_t state);
void xnap_id_set_target(xnap_id_manager *m, int ue_id, void *target);
void xnap_set_reloc_prep_timer(xnap_id_manager *m, int ue_id, uint64_t time);
void xnap_set_reloc_overall_timer(xnap_id_manager *m, int ue_id, uint64_t time);
void xnap_set_dc_prep_timer(xnap_id_manager *m, int ue_id, uint64_t time);
void xnap_set_dc_overall_timer(xnap_id_manager *m, int ue_id, uint64_t time);
int xnap_id_get_id_source(xnap_id_manager *m, int ue_id);
int xnap_id_get_id_target(xnap_id_manager *m, int ue_id);
int xnap_id_get_ueid(xnap_id_manager *m, int xn_id);
void *xnap_id_get_target(xnap_id_manager *m, int ue_id);

#endif /* X2AP_IDS_H_ */
