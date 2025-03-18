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

/*! \file xnap_gNB_generate_messages.h
 * \brief xnap procedures for gNB
 * \date 2023 July
 * \version 1.0
 */

#ifndef XNAP_GNB_GENERATE_MESSAGES_H_
#define XNAP_GNB_GENERATE_MESSAGES_H_

#include "xnap_gNB_defs.h"
#include "xnap_common.h"

int xnap_gNB_generate_xn_setup_request(sctp_assoc_t assoc_id, xnap_setup_req_t *req);

int xnap_gNB_generate_xn_setup_response(sctp_assoc_t assoc_id, xnap_setup_resp_t *resp);

int xnap_gNB_generate_xn_setup_failure(sctp_assoc_t assoc_id, xnap_setup_failure_t *fail);

int xnap_gNB_set_cause(XNAP_Cause_t *cause_p, XNAP_Cause_PR cause_type, long cause_value);

int xnap_gNB_generate_xn_handover_request(sctp_assoc_t assoc_id, xnap_handover_req_t *xnap_handover_req);

int xnap_gNB_generate_xn_handover_request_ack(sctp_assoc_t assoc_id,
                                              xnap_handover_req_ack_t *xnap_handover_req_ack,
                                              instance_t instance);

int xnap_gNB_generate_xn_ue_context_release(xnap_gNB_instance_t *instance_p, xnap_ue_context_release_t *xnap_ue_context_release);

#endif /*  XNAP_GNB_GENERATE_MESSAGES_H_ */
