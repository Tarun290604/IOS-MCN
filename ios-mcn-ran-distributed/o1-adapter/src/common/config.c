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

#include "config.h"
#include "utils.h"
#include "log.h"

#include <cjson/cJSON.h>
#include <stdlib.h>
#include <string.h>

static config_t config = {0};
static int config_ready = 0;
int log_level;

int config_init(const char *filename)
{
  char *fcontent = 0;
  cJSON *cjson = 0;

  fcontent = file_read_content(filename);
  if (fcontent == 0) {
    log_error("config file error");
    goto failure;
  }

  cjson = cJSON_Parse(fcontent);
  free(fcontent);
  fcontent = 0;
  if (cjson == 0) {
    log_error("config file parse error");
    goto failure;
  }

  cJSON *top;
  cJSON *object;
  char *strobject;
  top = cJSON_GetObjectItem(cjson, "log-level");
  if (top == 0) {
    log_error("config json parse error: log-level");
    goto failure;
  }
  config.log_level = top->valueint;
  *(&log_level) = config.log_level;

  object = cJSON_GetObjectItem(cjson, "software-version");
  if (object == 0) {
    log_error("config json parser error: software-version");
    goto failure;
  }
  strobject = cJSON_GetStringValue(object);
  if (strobject == 0) {
    log_error("config json strobject null");
    goto failure;
  }
  config.software_version = strdup(strobject);
  if (config.software_version == 0) {
    log_error("config json strdup error");
    goto failure;
  }

  top = cJSON_GetObjectItem(cjson, "network");
  if (top == 0) {
    log_error("config json parse error: network");
    goto failure;
  }
  object = cJSON_GetObjectItem(top, "host");
  if (object == 0) {
    log_error("config json parser error: host");
    goto failure;
  }
  strobject = cJSON_GetStringValue(object);
  if (strobject == 0) {
    log_error("config json strobject null");
    goto failure;
  }
  config.network.host = strdup(strobject);
  if (config.network.host == 0) {
    log_error("config json strdup error");
    goto failure;
  }

  object = cJSON_GetObjectItem(top, "username");
  if (object == 0) {
    log_error("config json parser error: username");
    goto failure;
  }
  strobject = cJSON_GetStringValue(object);
  if (strobject == 0) {
    log_error("config json strobject null");
    goto failure;
  }
  config.network.username = strdup(strobject);
  if (config.network.username == 0) {
    log_error("config json strdup error");
    goto failure;
  }

  object = cJSON_GetObjectItem(top, "password");
  if (object == 0) {
    log_error("config json parser error: password");
    goto failure;
  }
  strobject = cJSON_GetStringValue(object);
  if (strobject == 0) {
    log_error("config json strobject null");
    goto failure;
  }
  config.network.password = strdup(strobject);
  if (config.network.password == 0) {
    log_error("config json strdup error");
    goto failure;
  }

  object = cJSON_GetObjectItem(top, "netconf-port");
  if (object == 0) {
    log_error("config json parser error: netconf-port");
    goto failure;
  }
  config.network.netconf_port = object->valueint;

  object = cJSON_GetObjectItem(top, "sftp-port");
  if (object == 0) {
    log_error("config json parser error: sftp-port");
    goto failure;
  }
  config.network.sftp_port = object->valueint;

  top = cJSON_GetObjectItem(cjson, "ves");
  if (top == 0) {
    log_error("config json parse error: ves");
    goto failure;
  }
  cJSON *template = cJSON_GetObjectItem(top, "template");
  if (template == 0) {
    log_error("config json parse error: template");
    goto failure;
  }
  object = cJSON_GetObjectItem(template, "new-alarm");
  if (object == 0) {
    log_error("config json parser error: new-alarm");
    goto failure;
  }
  strobject = cJSON_GetStringValue(object);
  if (strobject == 0) {
    log_error("config json strobject null");
    goto failure;
  }
  config.ves.template.new_alarm = strdup(strobject);
  if (config.ves.template.new_alarm == 0) {
    log_error("config json strdup error");
    goto failure;
  }
  object = cJSON_GetObjectItem(template, "clear-alarm");
  if (object == 0) {
    log_error("config json parser error: clear-alarm");
    goto failure;
  }
  strobject = cJSON_GetStringValue(object);
  if (strobject == 0) {
    log_error("config json strobject null");
    goto failure;
  }
  config.ves.template.clear_alarm = strdup(strobject);
  if (config.ves.template.clear_alarm == 0) {
    log_error("config json strdup error");
    goto failure;
  }
  object = cJSON_GetObjectItem(template, "pnf-registration");
  if (object == 0) {
    log_error("config json parser error: pnf-registration");
    goto failure;
  }
  strobject = cJSON_GetStringValue(object);
  if (strobject == 0) {
    log_error("config json strobject null");
    goto failure;
  }
  config.ves.template.pnf_registration = strdup(strobject);
  if (config.ves.template.pnf_registration == 0) {
    log_error("config json strdup error");
    goto failure;
  }
  object = cJSON_GetObjectItem(template, "file-ready");
  if (object == 0) {
    log_error("config json parser error: file-ready");
    goto failure;
  }
  strobject = cJSON_GetStringValue(object);
  if (strobject == 0) {
    log_error("config json strobject null");
    goto failure;
  }
  config.ves.template.file_ready = strdup(strobject);
  if (config.ves.template.file_ready == 0) {
    log_error("config json strdup error");
    goto failure;
  }
  object = cJSON_GetObjectItem(template, "heartbeat");
  if (object == 0) {
    log_error("config json parser error: heartbeat");
    goto failure;
  }
  strobject = cJSON_GetStringValue(object);
  if (strobject == 0) {
    log_error("config json strobject null");
    goto failure;
  }
  config.ves.template.heartbeat = strdup(strobject);
  if (config.ves.template.heartbeat == 0) {
    log_error("config json strdup error");
    goto failure;
  }
  object = cJSON_GetObjectItem(template, "pm-data");
  if (object == 0) {
    log_error("config json parser error: pm-data");
    goto failure;
  }
  strobject = cJSON_GetStringValue(object);
  if (strobject == 0) {
    log_error("config json strobject null");
    goto failure;
  }
  config.ves.template.pm_data = strdup(strobject);
  if (config.ves.template.pm_data == 0) {
    log_error("config json strdup error");
    goto failure;
  }

  object = cJSON_GetObjectItem(top, "pnf-registration");
  if (object == 0) {
    log_error("config json parser error: pnf-registration");
    goto failure;
  }
  config.ves.pnf_registration = object->valueint;

  object = cJSON_GetObjectItem(top, "heartbeat-interval");
  if (object == 0) {
    log_error("config json parser error: heartbeat-interval");
    goto failure;
  }
  config.ves.heartbeat_interval = object->valueint;

  object = cJSON_GetObjectItem(top, "url");
  if (object == 0) {
    log_error("config json parser error: url");
    goto failure;
  }
  strobject = cJSON_GetStringValue(object);
  if (strobject == 0) {
    log_error("config json strobject null");
    goto failure;
  }
  config.ves.url = strdup(strobject);
  if (config.ves.url == 0) {
    log_error("config json strdup error");
    goto failure;
  }

  object = cJSON_GetObjectItem(top, "username");
  if (object == 0) {
    log_error("config json parser error: username");
    goto failure;
  }
  strobject = cJSON_GetStringValue(object);
  if (strobject == 0) {
    log_error("config json strobject null");
    goto failure;
  }
  config.ves.username = strdup(strobject);
  if (config.ves.username == 0) {
    log_error("config json strdup error");
    goto failure;
  }

  object = cJSON_GetObjectItem(top, "password");
  if (object == 0) {
    log_error("config json parser error: password");
    goto failure;
  }
  strobject = cJSON_GetStringValue(object);
  if (strobject == 0) {
    log_error("config json strobject null");
    goto failure;
  }
  config.ves.password = strdup(strobject);
  if (config.ves.password == 0) {
    log_error("config json strdup error");
    goto failure;
  }

  object = cJSON_GetObjectItem(top, "file-expiry");
  if (object == 0) {
    log_error("config json parser error: file-expiry");
    goto failure;
  }
  config.ves.file_expiry = object->valueint;

  object = cJSON_GetObjectItem(top, "pm-data-interval");
  if (object == 0) {
    log_error("config json parser error: pm-data-interval");
    goto failure;
  }
  config.ves.pm_data_interval = object->valueint;

  top = cJSON_GetObjectItem(cjson, "alarms");
  if (top == 0) {
    log_error("config json parse error: alarms");
    goto failure;
  }

  object = cJSON_GetObjectItem(top, "load-downlink-exceeded-warning-threshold");
  if (object == 0) {
    log_error("config json parser error: load-downlink-exceeded-warning-threshold");
    goto failure;
  }
  config.alarms.load_downlink_exceeded_warning_threshold = object->valueint;

  object = cJSON_GetObjectItem(top, "load-downlink-exceeded-warning-timeout");
  if (object == 0) {
    log_error("config json parser error: load-downlink-exceeded-warning-timeout");
    goto failure;
  }
  config.alarms.load_downlink_exceeded_warning_timeout = object->valueint;

  object = cJSON_GetObjectItem(top, "internal-connection-lost-timeout");
  if (object == 0) {
    log_error("config json parser error: internal-connection-lost-timeout");
    goto failure;
  }
  config.alarms.internal_connection_lost_timeout = object->valueint;

  top = cJSON_GetObjectItem(cjson, "telnet");
  if (top == 0) {
    log_error("config json parse error: telnet");
    goto failure;
  }
  object = cJSON_GetObjectItem(top, "host");
  if (object == 0) {
    log_error("config json parser error: host");
    goto failure;
  }
  strobject = cJSON_GetStringValue(object);
  if (strobject == 0) {
    log_error("config json strobject null");
    goto failure;
  }
  config.telnet.host = strdup(strobject);
  if (config.telnet.host == 0) {
    log_error("config json strdup error");
    goto failure;
  }

  object = cJSON_GetObjectItem(top, "port");
  if (object == 0) {
    log_error("config json parser error: port");
    goto failure;
  }
  config.telnet.port = object->valueint;

  top = cJSON_GetObjectItem(cjson, "info");
  if (top == 0) {
    log_error("config json parse error: info");
    goto failure;
  }
  object = cJSON_GetObjectItem(top, "gnb-du-id");
  if (object == 0) {
    log_error("config json parser error: gnb-du-id");
    goto failure;
  }
  config.info.gnb_du_id = object->valueint;

  object = cJSON_GetObjectItem(top, "gnb-cu-id");
  if (object == 0) {
    log_error("config json parser error: gnb-cu-id");
    goto failure;
  }
  config.info.gnb_cu_id = object->valueint;

  object = cJSON_GetObjectItem(top, "cell-local-id");
  if (object == 0) {
    log_error("config json parser error: cell-local-id");
    goto failure;
  }
  config.info.cell_local_id = object->valueint;

  object = cJSON_GetObjectItem(top, "node-id");
  if (object == 0) {
    log_error("config json parser error: node-id");
    goto failure;
  }
  strobject = cJSON_GetStringValue(object);
  if (strobject == 0) {
    log_error("config json strobject null");
    goto failure;
  }
  config.info.node_id = strdup(strobject);
  if (config.info.node_id == 0) {
    log_error("config json strdup error");
    goto failure;
  }

  object = cJSON_GetObjectItem(top, "location-name");
  if (object == 0) {
    log_error("config json parser error: location-name");
    goto failure;
  }
  strobject = cJSON_GetStringValue(object);
  if (strobject == 0) {
    log_error("config json strobject null");
    goto failure;
  }
  config.info.location_name = strdup(strobject);
  if (config.info.location_name == 0) {
    log_error("config json strdup error");
    goto failure;
  }

  object = cJSON_GetObjectItem(top, "managed-by");
  if (object == 0) {
    log_error("config json parser error: managed-by");
    goto failure;
  }
  strobject = cJSON_GetStringValue(object);
  if (strobject == 0) {
    log_error("config json strobject null");
    goto failure;
  }
  config.info.managed_by = strdup(strobject);
  if (config.info.managed_by == 0) {
    log_error("config json strdup error");
    goto failure;
  }

  object = cJSON_GetObjectItem(top, "managed-element-type");
  if (object == 0) {
    log_error("config json parser error: managed-element-type");
    goto failure;
  }
  strobject = cJSON_GetStringValue(object);
  if (strobject == 0) {
    log_error("config json strobject null");
    goto failure;
  }
  config.info.managed_element_type = strdup(strobject);
  if (config.info.managed_element_type == 0) {
    log_error("config json strdup error");
    goto failure;
  }

  object = cJSON_GetObjectItem(top, "model");
  if (object == 0) {
    log_error("config json parser error: model");
    goto failure;
  }
  strobject = cJSON_GetStringValue(object);
  if (strobject == 0) {
    log_error("config json strobject null");
    goto failure;
  }
  config.info.model = strdup(strobject);
  if (config.info.model == 0) {
    log_error("config json strdup error");
    goto failure;
  }

  object = cJSON_GetObjectItem(top, "unit-type");
  if (object == 0) {
    log_error("config json parser error: unit-type");
    goto failure;
  }
  strobject = cJSON_GetStringValue(object);
  if (strobject == 0) {
    log_error("config json strobject null");
    goto failure;
  }
  config.info.unit_type = strdup(strobject);
  if (config.info.unit_type == 0) {
    log_error("config json strdup error");
    goto failure;
  }

  cJSON_Delete(cjson);
  cjson = 0;
  config_ready = 1;
  return 0;

failure:
  free(fcontent);
  config_free(0);
  cJSON_Delete(cjson);
  return 1;
}

config_t *config_get()
{
  config_t *c = (config_t *)malloc(sizeof(config_t));
  if (!c) {
    return 0;
  }

  memset(c, 0, sizeof(config_t));

  c->log_level = config.log_level;

  c->software_version = strdup(config.software_version);
  if (c->software_version == 0) {
    log_error("software_version failed");
    goto failure;
  }

  c->network.host = strdup(config.network.host);
  if (c->network.host == 0) {
    log_error("network.host failed");
    goto failure;
  }

  c->network.username = strdup(config.network.username);
  if (c->network.username == 0) {
    log_error("network.username failed");
    goto failure;
  }
  c->network.password = strdup(config.network.password);
  if (c->network.password == 0) {
    log_error("network.password failed");
    goto failure;
  }
  c->network.netconf_port = config.network.netconf_port;
  c->network.sftp_port = config.network.sftp_port;

  c->ves.template.new_alarm = strdup(config.ves.template.new_alarm);
  if (c->ves.template.new_alarm == 0) {
    log_error("ves.template.new_alarm failed");
    goto failure;
  }
  c->ves.template.clear_alarm = strdup(config.ves.template.clear_alarm);
  if (c->ves.template.clear_alarm == 0) {
    log_error("ves.template.clear_alarm failed");
    goto failure;
  }
  c->ves.template.pnf_registration = strdup(config.ves.template.pnf_registration);
  if (c->ves.template.pnf_registration == 0) {
    log_error("ves.template.pnf_registration failed");
    goto failure;
  }
  c->ves.template.file_ready = strdup(config.ves.template.file_ready);
  if (c->ves.template.file_ready == 0) {
    log_error("ves.template.file_ready failed");
    goto failure;
  }
  c->ves.template.heartbeat = strdup(config.ves.template.heartbeat);
  if (c->ves.template.heartbeat == 0) {
    log_error("ves.template.heartbeat failed");
    goto failure;
  }
  c->ves.template.pm_data = strdup(config.ves.template.pm_data);
  if (c->ves.template.pm_data == 0) {
    log_error("ves.template.pm_data failed");
    goto failure;
  }
  c->ves.pnf_registration = config.ves.pnf_registration;
  c->ves.heartbeat_interval = config.ves.heartbeat_interval;
  c->ves.url = strdup(config.ves.url);
  if (c->ves.url == 0) {
    log_error("ves.url failed");
    goto failure;
  }
  c->ves.username = strdup(config.ves.username);
  if (c->ves.username == 0) {
    log_error("ves.username failed");
    goto failure;
  }
  c->ves.password = strdup(config.ves.password);
  if (c->ves.password == 0) {
    log_error("ves.password failed");
    goto failure;
  }
  c->ves.file_expiry = config.ves.file_expiry;
  c->ves.pm_data_interval = config.ves.pm_data_interval;

  c->alarms.internal_connection_lost_timeout = config.alarms.internal_connection_lost_timeout;
  c->alarms.load_downlink_exceeded_warning_threshold = config.alarms.load_downlink_exceeded_warning_threshold;
  c->alarms.load_downlink_exceeded_warning_timeout = config.alarms.load_downlink_exceeded_warning_timeout;

  c->telnet.host = strdup(config.telnet.host);
  if (c->telnet.host == 0) {
    log_error("telnet.host failed");
    goto failure;
  }
  c->telnet.port = config.telnet.port;

  c->info.gnb_du_id = config.info.gnb_du_id;
  c->info.gnb_cu_id = config.info.gnb_cu_id;
  c->info.cell_local_id = config.info.cell_local_id;
  c->info.node_id = strdup(config.info.node_id);
  if (c->info.node_id == 0) {
    log_error("info.node_id failed");
    goto failure;
  }
  c->info.location_name = strdup(config.info.location_name);
  if (c->info.location_name == 0) {
    log_error("info.location_name failed");
    goto failure;
  }
  c->info.managed_by = strdup(config.info.managed_by);
  if (c->info.managed_by == 0) {
    log_error("info.managed_by failed");
    goto failure;
  }
  c->info.managed_element_type = strdup(config.info.managed_element_type);
  if (c->info.managed_element_type == 0) {
    log_error("info.managed_element_type failed");
    goto failure;
  }
  c->info.model = strdup(config.info.model);
  if (c->info.model == 0) {
    log_error("info.model failed");
    goto failure;
  }
  c->info.unit_type = strdup(config.info.unit_type);
  if (c->info.unit_type == 0) {
    log_error("info.unit_type failed");
    goto failure;
  }

  return c;

failure:
  config_free(c);
  return 0;
}

void config_free(config_t *cconfig)
{
  if (cconfig == 0) {
    cconfig = &config;
  }

  free(cconfig->software_version);
  cconfig->software_version = 0;

  free(cconfig->network.host);
  cconfig->network.host = 0;
  free(cconfig->network.username);
  cconfig->network.username = 0;
  free(cconfig->network.password);
  cconfig->network.password = 0;

  free(cconfig->ves.template.new_alarm);
  cconfig->ves.template.new_alarm = 0;
  free(cconfig->ves.template.clear_alarm);
  cconfig->ves.template.clear_alarm = 0;
  free(cconfig->ves.template.pnf_registration);
  cconfig->ves.template.pnf_registration = 0;
  free(cconfig->ves.template.file_ready);
  cconfig->ves.template.file_ready = 0;
  free(cconfig->ves.template.heartbeat);
  cconfig->ves.template.heartbeat = 0;
  free(cconfig->ves.template.pm_data);
  cconfig->ves.template.pm_data = 0;
  free(cconfig->ves.url);
  cconfig->ves.url = 0;
  free(cconfig->ves.username);
  cconfig->ves.username = 0;
  free(cconfig->ves.password);
  cconfig->ves.password = 0;

  free(cconfig->telnet.host);
  cconfig->telnet.host = 0;

  free(cconfig->info.node_id);
  cconfig->info.node_id = 0;
  free(cconfig->info.location_name);
  cconfig->info.location_name = 0;
  free(cconfig->info.managed_by);
  cconfig->info.managed_by = 0;
  free(cconfig->info.managed_element_type);
  cconfig->info.managed_element_type = 0;
  free(cconfig->info.model);
  cconfig->info.model = 0;
  free(cconfig->info.unit_type);
  cconfig->info.unit_type = 0;
  config_ready = 0;

  if (cconfig != &config) {
    free(cconfig);
  }
}

void config_print(const config_t *cconfig)
{
  if (cconfig == 0) {
    cconfig = &config;
  }

  log("Config: ");
  log("- log_level: %d", cconfig->log_level);
  log("- software_version: %s", cconfig->software_version);
  log("- network.host: %s", cconfig->network.host);
  log("- network.username: %s", cconfig->network.username);
  log("- network.password: %s", cconfig->network.password);
  log("- network.netconf_port: %d", cconfig->network.netconf_port);
  log("- network.sftp_port: %d", cconfig->network.sftp_port);
  log("- ves.template.new_alarm: %s", cconfig->ves.template.new_alarm);
  log("- ves.template.clear_alarm: %s", cconfig->ves.template.clear_alarm);
  log("- ves.template.pnf_registration: %s", cconfig->ves.template.pnf_registration);
  log("- ves.template.file_ready: %s", cconfig->ves.template.file_ready);
  log("- ves.template.heartbeat: %s", cconfig->ves.template.heartbeat);
  log("- ves.template.pm_data: %s", cconfig->ves.template.pm_data);
  log("- ves.pnf_registration: %d", cconfig->ves.pnf_registration);
  log("- ves.heartbeat_interval: %d", cconfig->ves.heartbeat_interval);
  log("- ves.url: %s", cconfig->ves.url);
  log("- ves.username: %s", cconfig->ves.username);
  log("- ves.password: %s", cconfig->ves.password);
  log("- ves.file_expiry: %d", cconfig->ves.file_expiry);
  log("- ves.pm_data_interval: %d", cconfig->ves.pm_data_interval);
  log("- alarms.internal_connection_lost_timeout: %d", cconfig->alarms.internal_connection_lost_timeout);
  log("- alarms.load_downlink_exceeded_warning_threshold: %d", cconfig->alarms.load_downlink_exceeded_warning_threshold);
  log("- alarms.load_downlink_exceeded_warning_timeout: %d", cconfig->alarms.load_downlink_exceeded_warning_timeout);
  log("- telnet.host: %s", cconfig->telnet.host);
  log("- telnet.port: %d", cconfig->telnet.port);
  log("- info.gnb_du_id: %d", cconfig->info.gnb_du_id);
  log("- info.gnb_cu_id: %d", cconfig->info.gnb_cu_id);
  log("- info.cell_local_id: %d", cconfig->info.cell_local_id);
  log("- info.node_id: %s", cconfig->info.node_id);
  log("- info.location_name: %s", cconfig->info.location_name);
  log("- info.managed_by: %s", cconfig->info.managed_by);
  log("- info.managed_element_type: %s", cconfig->info.managed_element_type);
  log("- info.model: %s", cconfig->info.model);
  log("- info.unit_type: %s", cconfig->info.unit_type);
}
