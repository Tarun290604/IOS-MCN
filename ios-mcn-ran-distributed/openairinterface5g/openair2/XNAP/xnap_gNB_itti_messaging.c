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

#include "intertask_interface.h"
#include "xnap_gNB_itti_messaging.h"

void xnap_gNB_itti_send_sctp_data_req(sctp_assoc_t assoc_id, uint8_t *buffer, uint32_t buffer_length, uint16_t stream)
{
  MessageDef *message_p;
  sctp_data_req_t *sctp_data_req;
  instance_t instance = 0; // we have only one instance
  message_p = itti_alloc_new_message(TASK_XNAP, 0, SCTP_DATA_REQ);
  sctp_data_req = &message_p->ittiMsg.sctp_data_req;
  sctp_data_req->assoc_id = assoc_id;
  sctp_data_req->buffer = buffer;
  sctp_data_req->buffer_length = buffer_length;
  sctp_data_req->stream = stream;
  itti_send_msg_to_task(TASK_SCTP, instance, message_p);
}
