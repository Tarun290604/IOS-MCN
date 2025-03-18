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

/*! \file xnap_messages_def.h
 * \author Sreeshma Shiv <sreeshmau@iisc.ac.in>
 * \date August 2023
 * \version 1.0
 */

#include "openair2/COMMON/xnap_messages_types.h"

/* gNB application layer -> XNAP messages */
MESSAGE_DEF(XNAP_REGISTER_GNB_REQ, MESSAGE_PRIORITY_MED, xnap_register_gnb_req_t, xnap_register_gnb_req)
/* XNAP -> gNB application layer messages */
/* handover messages XNAP <-> RRC */
MESSAGE_DEF(XNAP_SETUP_REQ, MESSAGE_PRIORITY_MED, xnap_setup_req_t, xnap_setup_req)
MESSAGE_DEF(XNAP_SETUP_RESP, MESSAGE_PRIORITY_MED, xnap_setup_resp_t, xnap_setup_resp)
MESSAGE_DEF(XNAP_SETUP_FAILURE, MESSAGE_PRIORITY_MED, xnap_setup_failure_t, xnap_setup_failure)
MESSAGE_DEF(XNAP_HANDOVER_REQ, MESSAGE_PRIORITY_MED, xnap_handover_req_t, xnap_handover_req)
MESSAGE_DEF(XNAP_HANDOVER_REQ_FAILURE, MESSAGE_PRIORITY_MED, xnap_handover_req_failure_t, xnap_handover_req_failure)
MESSAGE_DEF(XNAP_HANDOVER_REQ_ACK, MESSAGE_PRIORITY_MED, xnap_handover_req_ack_t, xnap_handover_req_ack)
MESSAGE_DEF(XNAP_UE_CONTEXT_RELEASE, MESSAGE_PRIORITY_MED, xnap_ue_context_release_t, xnap_ue_context_release)
