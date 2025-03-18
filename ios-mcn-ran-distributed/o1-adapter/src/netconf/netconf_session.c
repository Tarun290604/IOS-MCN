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

#include "netconf_session.h"
#include "common/log.h"

sr_conn_ctx_t *netconf_session_connection = 0;
sr_session_ctx_t *netconf_session_running = 0;
sr_session_ctx_t *netconf_session_operational = 0;
const struct ly_ctx *netconf_session_context = 0;

int netconf_session_init(void)
{
  int rc = SR_ERR_OK;

  /* connect to sysrepo */
  rc = sr_connect(0, &netconf_session_connection);
  if (SR_ERR_OK != rc) {
    log_error("sr_connect failed");
    goto netconf_session_init_cleanup;
  }

  /* start session */
  rc = sr_session_start(netconf_session_connection, SR_DS_OPERATIONAL, &netconf_session_operational);
  if (rc != SR_ERR_OK) {
    log_error("sr_session_start operational failed");
    goto netconf_session_init_cleanup;
  }

  rc = sr_session_start(netconf_session_connection, SR_DS_RUNNING, &netconf_session_running);
  if (rc != SR_ERR_OK) {
    log_error("sr_session_start running failed");
    goto netconf_session_init_cleanup;
  }

  /* get context */
  netconf_session_context = sr_acquire_context(netconf_session_connection);
  if (netconf_session_context == 0) {
    log_error("sr_acquire_context failed");
    goto netconf_session_init_cleanup;
  }

  return 0;

netconf_session_init_cleanup:
  return 1;
}

void netconf_session_free(void)
{
  sr_release_context(netconf_session_connection);

  sr_session_stop(netconf_session_operational);
  sr_session_stop(netconf_session_running);

  sr_disconnect(netconf_session_connection);

  netconf_session_connection = 0;
  netconf_session_running = 0;
  netconf_session_operational = 0;
  netconf_session_context = 0;
}
