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

#define _GNU_SOURCE

#include "alarms.h"
#include "common/log.h"
#include "ves/ves.h"
#include "netconf/netconf_data.h"
#include <string.h>
#include <stdlib.h>

static int alarm_raise(alarm_t *alarm);
static int alarm_clear(alarm_t *alarm);

static alarm_t alarm_internal_connection_loss = {
    .alarm = "internalConnectionLoss",
    .severity = ALARM_SEVERITY_MAJOR,
    .type = ALARM_TYPE_COMMUNICATIONS_ALARM,
    .object_instance = 0,
    .state = ALARM_STATE_CLEARED,
    .timeout = 0,
};

static alarm_t alarm_load_downlink_exceeded_warning = {
    .alarm = "loadDownlinkExceededWarning",
    .severity = ALARM_SEVERITY_WARNING,
    .type = ALARM_TYPE_EQUIPMENT_ALARM,
    .object_instance = 0,
    .state = ALARM_STATE_CLEARED,
    .timeout = 0,
};

static const alarm_t *alarms[] = {&alarm_internal_connection_loss, &alarm_load_downlink_exceeded_warning, 0};

static const config_t *alarms_config = 0;
static int alarm_notification_id = 1;

int alarms_init(const config_t *config)
{
  int rc;

  if (config == 0) {
    log_error("no ves config");
    goto failed;
  }

  alarms_config = config;
  alarm_notification_id = 1;

  asprintf(&alarm_internal_connection_loss.object_instance, "ManagedElement=%s", alarms_config->info.node_id);
  if (alarm_internal_connection_loss.object_instance == 0) {
    log_error("asprintf failed");
    goto failed;
  }

  asprintf(&alarm_load_downlink_exceeded_warning.object_instance,
           "ManagedElement=%s,GNBDUFunction=%d,NRCellDU=0",
           alarms_config->info.node_id,
           alarms_config->info.gnb_du_id);
  if (alarm_load_downlink_exceeded_warning.object_instance == 0) {
    log_error("asprintf failed");
    goto failed;
  }

  rc = netconf_data_alarms_init(alarms);
  if (rc != 0) {
    log_error("netconf_data_alarms_init() error");
    goto failed;
  }

  return 0;
failed:
  alarms_free();
  return 1;
}

int alarms_free()
{
  alarm_t **alarm = (alarm_t **)alarms;
  while (*alarm) {
    free((*alarm)->object_instance);
    (*alarm)->object_instance = 0;
    (*alarm)->state = ALARM_STATE_CLEARED;
    (*alarm)->timeout = 0;
    alarm++;
  }

  alarms_config = 0;
  return 0;
}

const char *alarm_severity_to_str(const alarm_severity_t severity)
{
  switch (severity) {
    case ALARM_SEVERITY_CLEARED:
      return "CLEARED";
      break;

    case ALARM_SEVERITY_INDETERMINATE:
      return "INDETERMINATE";
      break;

    case ALARM_SEVERITY_WARNING:
      return "WARNING";
      break;

    case ALARM_SEVERITY_MINOR:
      return "MINOR";
      break;

    case ALARM_SEVERITY_MAJOR:
      return "MAJOR";
      break;

    case ALARM_SEVERITY_CRITICAL:
      return "CRITICAL";
      break;

    default:
      return "UNDEFINED";
      break;
  }
}

const char *alarm_type_to_str(const alarm_type_t type)
{
  switch (type) {
    case ALARM_TYPE_COMMUNICATIONS_ALARM:
      return "COMMUNICATIONS_ALARM";
      break;

    case ALARM_TYPE_QUALITY_OF_SERVICE_ALARM:
      return "QUALITY_OF_SERVICE_ALARM";
      break;

    case ALARM_TYPE_PROCESSING_ERROR_ALARM:
      return "PROCESSING_ERROR_ALARM";
      break;

    case ALARM_TYPE_EQUIPMENT_ALARM:
      return "EQUIPMENT_ALARM";
      break;

    case ALARM_TYPE_ENVIRONMENTAL_ALARM:
      return "ENVIRONMENTAL_ALARM";
      break;

    case ALARM_TYPE_INTEGRITY_VIOLATION:
      return "INTEGRITY_VIOLATION";
      break;

    case ALARM_TYPE_OPERATIONAL_VIOLATION:
      return "OPERATIONAL_VIOLATION";
      break;

    case ALARM_TYPE_PHYSICAL_VIOLATION:
      return "PHYSICAL_VIOLATION";
      break;

    case ALARM_TYPE_SECURITY_SERVICE_OR_MECHANISM_VIOLATION:
      return "SECURITY_SERVICE_OR_MECHANISM_VIOLATION";
      break;

    case ALARM_TYPE_TIME_DOMAIN_VIOLATION:
      return "TIME_DOMAIN_VIOLATION";
      break;

    default:
      return "UNDEFINED";
      break;
  }
}

void alarms_on_telnet_connected()
{
  log("telnet connected");
  if (alarm_internal_connection_loss.state == ALARM_STATE_RAISED) {
    alarm_internal_connection_loss.state = ALARM_STATE_CLEAR;
    alarm_internal_connection_loss.timeout = 0;
  } else if (alarm_internal_connection_loss.state == ALARM_STATE_RAISE) {
    alarm_internal_connection_loss.state = ALARM_STATE_CLEARED;
  }
}

void alarms_on_telnet_disconnected()
{
  log_error("telnet disconnected");
  if (alarm_internal_connection_loss.state == ALARM_STATE_CLEARED) {
    alarm_internal_connection_loss.state = ALARM_STATE_RAISE;
    alarm_internal_connection_loss.timeout = alarms_config->alarms.internal_connection_lost_timeout;
  } else if (alarm_internal_connection_loss.state == ALARM_STATE_CLEAR) {
    alarm_internal_connection_loss.state = ALARM_STATE_RAISED;
  }
}

int alarms_data_feed(const alarms_data_t *alarms_data)
{
  if (alarms_data == 0) {
    log_error("alarms_data is null");
    goto failed;
  }

  if (alarms_data->load > alarms_config->alarms.load_downlink_exceeded_warning_threshold) {
    if (alarm_load_downlink_exceeded_warning.state == ALARM_STATE_CLEARED) {
      alarm_load_downlink_exceeded_warning.state = ALARM_STATE_RAISE;
      alarm_load_downlink_exceeded_warning.timeout = alarms_config->alarms.load_downlink_exceeded_warning_timeout;
    } else if (alarm_load_downlink_exceeded_warning.state == ALARM_STATE_CLEAR) {
      alarm_load_downlink_exceeded_warning.state = ALARM_STATE_RAISED;
      alarm_load_downlink_exceeded_warning.timeout = alarms_config->alarms.load_downlink_exceeded_warning_timeout;
    }
  } else {
    if (alarm_load_downlink_exceeded_warning.state == ALARM_STATE_RAISED) {
      alarm_load_downlink_exceeded_warning.state = ALARM_STATE_CLEAR;
      alarm_load_downlink_exceeded_warning.timeout = alarms_config->alarms.load_downlink_exceeded_warning_timeout;
    } else if (alarm_load_downlink_exceeded_warning.state == ALARM_STATE_RAISE) {
      alarm_load_downlink_exceeded_warning.state = ALARM_STATE_CLEARED;
      alarm_load_downlink_exceeded_warning.timeout = alarms_config->alarms.load_downlink_exceeded_warning_timeout;
    }
  }

  return 0;

failed:
  return 1;
}

void alarms_loop()
{
  int rc;

  alarm_t **alarm = (alarm_t **)alarms;
  while (*alarm) {
    switch ((*alarm)->state) {
      case ALARM_STATE_CLEAR:
        if ((*alarm)->timeout == 0) {
          rc = alarm_clear(*alarm);
          if (rc != 0) {
            log_error("alarm_clear(%s) failed", (*alarm)->alarm);
          }
        } else {
          (*alarm)->timeout--;
        }
        break;

      case ALARM_STATE_RAISE:
        if ((*alarm)->timeout == 0) {
          rc = alarm_raise(*alarm);
          if (rc != 0) {
            log_error("alarm_raise(%s) failed", (*alarm)->alarm);
          }
        } else {
          (*alarm)->timeout--;
        }
        break;

      default:
        break;
    }

    alarm++;
  }
}

static int alarm_raise(alarm_t *alarm)
{
  int rc;

  alarm->state = ALARM_STATE_RAISED;

  ves_alarm_t ves_alarm = {
      .alarm = alarm->alarm,
      .severity = (char *)alarm_severity_to_str(alarm->severity),
      .type = (char *)alarm_type_to_str(alarm->type),
      .object_instance = alarm->object_instance,
      .notification_id = alarm_notification_id,
  };

  rc = ves_alarm_new_execute(&ves_alarm);
  if (rc != 0) {
    log_error("ves_alarm_new_execute() failed");
  }

  rc = netconf_data_update_alarm(alarm, alarm_notification_id);
  if (rc != 0) {
    log_error("netconf_data_update_alarm() failed");
  }

  alarm_notification_id++;

  return 0;
}

static int alarm_clear(alarm_t *alarm)
{
  int rc;

  alarm->state = ALARM_STATE_CLEARED;

  ves_alarm_t ves_alarm = {
      .alarm = alarm->alarm,
      .severity = (char *)alarm_severity_to_str(alarm->severity),
      .type = (char *)alarm_type_to_str(alarm->type),
      .object_instance = alarm->object_instance,
      .notification_id = alarm_notification_id,
  };

  rc = ves_alarm_clear_execute(&ves_alarm);
  if (rc != 0) {
    log_error("ves_alarm_clear_execute() failed");
  }

  rc = netconf_data_update_alarm(alarm, alarm_notification_id);
  if (rc != 0) {
    log_error("netconf_data_update_alarm() failed");
  }

  alarm_notification_id++;

  return 0;
}
