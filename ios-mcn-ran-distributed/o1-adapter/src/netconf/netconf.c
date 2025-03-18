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

#include "netconf.h"
#include "netconf_session.h"
#include "common/config.h"
#include "common/log.h"
#include "common/utils.h"

static int netconf_disable_nacm(sr_session_ctx_t *session);

int netconf_init(void)
{
  int rc = 0;

  // init netconf
  sr_log_stderr(SR_LL_WRN);
  rc = netconf_session_init();
  if (rc != 0) {
    log_error("netconf_session_init() failed");
    goto failed;
  }

  rc = netconf_disable_nacm(netconf_session_running);
  if (rc != 0) {
    log("netconf_disable_nacm() failed\n");
  }

  return 0;

failed:
  return 1;
}

int netconf_free(void)
{
  netconf_session_free();

  return 0;
}

static int netconf_disable_nacm(sr_session_ctx_t *session)
{
#define IETF_NETCONF_ACM_ENABLE_NACM_SCHEMA_XPATH "/ietf-netconf-acm:nacm/enable-nacm"
#define IETF_NETCONF_ACM_GROUPS_SCHEMA_XPATH "/ietf-netconf-acm:nacm/groups"
#define IETF_NETCONF_ACM_RULE_LIST_SCHEMA_XPATH "/ietf-netconf-acm:nacm/rule-list"

  int rc = 0;
  rc = sr_set_item_str(session, IETF_NETCONF_ACM_ENABLE_NACM_SCHEMA_XPATH, "true", 0, 0);
  if (rc != SR_ERR_OK) {
    log_error("sr_set_item_str() failed");
    goto failed;
  }

  rc = sr_set_item_str(session, IETF_NETCONF_ACM_GROUPS_SCHEMA_XPATH "/group[name='sudo']/user-name", "netconf", 0, 0);
  if (rc != SR_ERR_OK) {
    log_error("sr_set_item_str() failed");
    goto failed;
  }

  rc = sr_set_item_str(session, IETF_NETCONF_ACM_RULE_LIST_SCHEMA_XPATH "[name='sudo-rules']/group", "sudo", 0, 0);
  if (rc != SR_ERR_OK) {
    log_error("sr_set_item_str() failed");
    goto failed;
  }

  rc = sr_set_item_str(session,
                       IETF_NETCONF_ACM_RULE_LIST_SCHEMA_XPATH "[name='sudo-rules']/rule[name='allow-all-sudo']/module-name",
                       "*",
                       0,
                       0);
  if (rc != SR_ERR_OK) {
    log_error("sr_set_item_str() failed");
    goto failed;
  }

  rc = sr_set_item_str(session,
                       IETF_NETCONF_ACM_RULE_LIST_SCHEMA_XPATH "[name='sudo-rules']/rule[name='allow-all-sudo']/path",
                       "/",
                       0,
                       0);
  if (rc != SR_ERR_OK) {
    log_error("sr_set_item_str() failed");
    goto failed;
  }

  rc = sr_set_item_str(session,
                       IETF_NETCONF_ACM_RULE_LIST_SCHEMA_XPATH "[name='sudo-rules']/rule[name='allow-all-sudo']/access-operations",
                       "*",
                       0,
                       0);
  if (rc != SR_ERR_OK) {
    log_error("sr_set_item_str() failed");
    goto failed;
  }

  rc = sr_set_item_str(session,
                       IETF_NETCONF_ACM_RULE_LIST_SCHEMA_XPATH "[name='sudo-rules']/rule[name='allow-all-sudo']/action",
                       "permit",
                       0,
                       0);
  if (rc != SR_ERR_OK) {
    log_error("sr_set_item_str() failed");
    goto failed;
  }

  rc = sr_set_item_str(session,
                       IETF_NETCONF_ACM_RULE_LIST_SCHEMA_XPATH "[name='sudo-rules']/rule[name='allow-all-sudo']/comment",
                       "Corresponds all the rules under the sudo group as defined in O-RAN.WG4.MP.0-v05.00",
                       0,
                       0);
  if (rc != SR_ERR_OK) {
    log_error("sr_set_item_str() failed");
    goto failed;
  }

  rc = sr_apply_changes(session, 0);
  if (rc != SR_ERR_OK) {
    log_error("sr_apply_changes() failed");
    goto failed;
  }

  return 0;

failed:
  sr_discard_changes(session);
  return 1;
}
