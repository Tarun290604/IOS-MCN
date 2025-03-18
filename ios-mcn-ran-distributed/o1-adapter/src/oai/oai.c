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

#include "oai.h"
#include "alarms/alarms.h"
#include "common/log.h"
#include "netconf/netconf_data.h"
#include "pm_data/pm_data.h"
#include "ves/ves.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

static oai_data_t current_data = {0};
extern char *nodeType;

int oai_init()
{
  if (strcmp(nodeType, "du") == 0 || strcmp(nodeType, "gNB") == 0) {
    memset(&current_data, 0, sizeof(oai_data_t));
    current_data.nrcelldu.mcc = strdup("");
    current_data.nrcelldu.mnc = strdup("");

    current_data.device_data.gnbName = strdup("");
    current_data.device_data.vendor = strdup("");

    current_data.additional_data.frameType = strdup("");
  }

  if (strcmp(nodeType, "cu-cp") == 0) {
    memset(&current_data, 0, sizeof(oai_data_t));
    current_data.nrcellcu.mcc = strdup("");
    current_data.nrcellcu.mnc = strdup("");
    current_data.device_data.vendor = strdup("");
  }

  if (strcmp(nodeType, "cu-up") == 0) {
    memset(&current_data, 0, sizeof(oai_data_t));
    current_data.gnbcuup.mcc = strdup("");
    current_data.gnbcuup.mnc = strdup("");
    current_data.device_data.vendor = strdup("");
  }

  return 0;
}

int oai_free()
{
  if (strcmp(nodeType, "du") == 0 || strcmp(nodeType, "gNB") == 0) {
    free(current_data.nrcelldu.mcc);
    free(current_data.nrcelldu.mnc);

    free(current_data.device_data.gnbName);
    free(current_data.device_data.vendor);

    free(current_data.additional_data.frameType);
    free(current_data.additional_data.ues);

    memset(&current_data, 0, sizeof(oai_data_t));
  }

  if (strcmp(nodeType, "cu-cp") == 0) {
    free(current_data.nrcellcu.mcc);
    free(current_data.nrcellcu.mnc);
    // free(current_data.additional_data.vendor);
    memset(&current_data, 0, sizeof(oai_data_t));
  }

  if (strcmp(nodeType, "cu-up") == 0) {
    free(current_data.gnbcuup.mcc);
    free(current_data.gnbcuup.mnc);
    memset(&current_data, 0, sizeof(oai_data_t));
  }

  return 0;
}

int oai_data_feed(const oai_data_t *data)
{
  if (data == 0) {
    log_error("invalid data");
    goto failure;
  }

  ves_info_t ves_info = {0};
  pm_data_info_t pm_data_info = {0};
#define UPDATE_NONE 0x00000000
#define UPDATE_FULL 0xFFFFFFFF
#define UPDATE_BWP_DL 0x00000001
#define UPDATE_BWP_UL 0x00000002
#define UPDATE_NRCELLDU 0x00000004
#define UPDATE_NRCELLCU 0x00000005
#define UPDATE_GNBCUUP 0x00000006

  uint32_t update_type = UPDATE_NONE;

  if (strcmp(nodeType, "du") == 0 || strcmp(nodeType, "gNB") == 0) {
    if (data->device_data.gnbId != current_data.device_data.gnbId) {
      // gnbId changed

      update_type = UPDATE_FULL;
      asprintf(&ves_info.managed_element_id, "%d", data->device_data.gnbId);

      current_data.device_data.gnbId = data->device_data.gnbId;
    }

    if (strcmp(data->device_data.gnbName, current_data.device_data.gnbName)) {
      // gnbName changed

      update_type = UPDATE_FULL;

      free(current_data.device_data.gnbName);
      current_data.device_data.gnbName = strdup(data->device_data.gnbName);
    }

    if (strcmp(data->device_data.vendor, current_data.device_data.vendor)) {
      // vendor changed

      ves_info.vendor = strdup(data->device_data.vendor);
      pm_data_info.vendor = strdup(data->device_data.vendor);

      free(current_data.device_data.vendor);
      current_data.device_data.vendor = strdup(data->device_data.vendor);
    }

    if (memcmp(&data->bwp[0], &current_data.bwp[0], sizeof(oai_bwp_data_t))) {
      // downlink BWP changed

      update_type |= UPDATE_BWP_DL;

      current_data.bwp[0] = data->bwp[0];
    }

    if (memcmp(&data->bwp[1], &current_data.bwp[1], sizeof(oai_bwp_data_t))) {
      // uplink BWP changed

      update_type |= UPDATE_BWP_UL;

      current_data.bwp[1] = data->bwp[1];
    }

    if ((data->nrcelldu.ssbFrequency != current_data.nrcelldu.ssbFrequency)
        || (data->nrcelldu.arfcnDL != current_data.nrcelldu.arfcnDL)
        || (data->nrcelldu.bSChannelBwDL != current_data.nrcelldu.bSChannelBwDL)
        || (data->nrcelldu.arfcnUL != current_data.nrcelldu.arfcnUL)
        || (data->nrcelldu.bSChannelBwUL != current_data.nrcelldu.bSChannelBwUL)
        || (data->nrcelldu.nRPCI != current_data.nrcelldu.nRPCI) || (data->nrcelldu.nRTAC != current_data.nrcelldu.nRTAC)
        || strcmp(data->nrcelldu.mcc, current_data.nrcelldu.mcc) || strcmp(data->nrcelldu.mnc, current_data.nrcelldu.mnc)
        || (data->nrcelldu.sd != current_data.nrcelldu.sd) || (data->nrcelldu.sst != current_data.nrcelldu.sst)) {
      // nrcelldu changed

      update_type |= UPDATE_NRCELLDU;

      current_data.nrcelldu.ssbFrequency = data->nrcelldu.ssbFrequency;
      current_data.nrcelldu.arfcnDL = data->nrcelldu.arfcnDL;
      current_data.nrcelldu.bSChannelBwDL = data->nrcelldu.bSChannelBwDL;
      current_data.nrcelldu.arfcnUL = data->nrcelldu.arfcnUL;
      current_data.nrcelldu.bSChannelBwUL = data->nrcelldu.bSChannelBwUL;
      current_data.nrcelldu.nRPCI = data->nrcelldu.nRPCI;
      current_data.nrcelldu.nRTAC = data->nrcelldu.nRTAC;
      free(current_data.nrcelldu.mcc);
      current_data.nrcelldu.mcc = strdup(data->nrcelldu.mcc);
      free(current_data.nrcelldu.mnc);
      current_data.nrcelldu.mnc = strdup(data->nrcelldu.mnc);
      current_data.nrcelldu.sd = data->nrcelldu.sd;
      current_data.nrcelldu.sst = data->nrcelldu.sst;
    }
  }

  if (strcmp(nodeType, "cu-cp") == 0 || strcmp(nodeType, "gNB") == 0) {
    if (data->device_data.gnbId != current_data.device_data.gnbId) {
      // gNBId changed

      update_type = UPDATE_FULL;
      asprintf(&ves_info.managed_element_id, "%d", data->device_data.gnbId);

      current_data.device_data.gnbId = data->device_data.gnbId;
    }
    if ((data->nrcellcu.pMax != current_data.nrcellcu.pMax) || (data->nrcellcu.cellLocalId != current_data.nrcellcu.cellLocalId)) {
      update_type |= UPDATE_NRCELLCU;

      current_data.nrcellcu.pMax = data->nrcellcu.pMax;
      current_data.nrcellcu.cellLocalId = data->nrcellcu.cellLocalId;
    }
    if (strcmp(data->device_data.vendor, current_data.device_data.vendor)) {
      // vendor changed

      ves_info.vendor = strdup(data->device_data.vendor);
      pm_data_info.vendor = strdup(data->device_data.vendor);

      free(current_data.device_data.vendor);
      current_data.device_data.vendor = strdup(data->device_data.vendor);
    }
    if (data->device_data.gnbId != current_data.device_data.gnbId) {
      // gNBId changed

      update_type = UPDATE_FULL;
      asprintf(&ves_info.managed_element_id, "%d", data->device_data.gnbId);

      current_data.device_data.gnbId = data->device_data.gnbId;
    }
  }

  if (strcmp(nodeType, "cu-up") == 0) {
    if (strcmp(data->device_data.vendor, current_data.device_data.vendor)) {
      // vendor changed

      ves_info.vendor = strdup(data->device_data.vendor);
      pm_data_info.vendor = strdup(data->device_data.vendor);

      free(current_data.device_data.vendor);
      current_data.device_data.vendor = strdup(data->device_data.vendor);
    }
    if (data->gnbcuup.gNBId != current_data.gnbcuup.gNBId) {
      // gNBId changed

      update_type = UPDATE_FULL;
      asprintf(&ves_info.managed_element_id, "%d", data->gnbcuup.gNBId);

      current_data.gnbcuup.gNBId = data->gnbcuup.gNBId;
    }
  }

  int rc;
  if (update_type == UPDATE_FULL) {
    rc = netconf_data_update_full(data);
    if (rc) {
      log_error("netconf_data_update_full failed");
      goto failure;
    }
  } else {
    if (update_type & UPDATE_BWP_DL) {
      rc = netconf_data_update_bwp_dl(data);
      if (rc) {
        log_error("netconf_data_update_bwp_dl failed");
        goto failure;
      }
    }

    if (update_type & UPDATE_BWP_UL) {
      rc = netconf_data_update_bwp_ul(data);
      if (rc) {
        log_error("netconf_data_update_bwp_ul failed");
        goto failure;
      }
    }

    if (update_type & UPDATE_NRCELLDU) {
      rc = netconf_data_update_nrcelldu(data);
      if (rc) {
        log_error("netconf_data_update_nrcelldu failed");
        goto failure;
      }
    }

    if (update_type & UPDATE_NRCELLCU) {
      rc = netconf_data_update_nrcellcu(data);
      if (rc) {
        log_error("netconf_data_update_nrcellcu failed");
        goto failure;
      }
    }

    if (update_type & UPDATE_GNBCUUP) {
      rc = netconf_data_update_gnbcuup(data);
      if (rc) {
        log_error("netconf_data_update_gnbcuup failed");
        goto failure;
      }
    }
  }

  rc = ves_set_info(&ves_info);
  if(rc) {
      log_error("ves_set_info failed");
      goto failure;
  }

  if (strcmp(nodeType, "cu-cp") == 0) {
    alarms_data_t alarms_data = {
        .num_du = data->additional_data.num_dus, // To-Do alarms not yet implemented for cu
    };

    rc = alarms_data_feed(&alarms_data);
    if (rc) {
      log_error("alarms_data_feed failed");
      goto failure;
    }

    rc = pm_data_set_info(&pm_data_info);
    if (rc) {
      log_error("pm_data_set_info failed");
      goto failure;
    }
    free(ves_info.managed_element_id);
    free(ves_info.vendor);

    return 0;
  }

  if (strcmp(nodeType, "du") == 0 || strcmp(nodeType, "gNB") == 0) {
    long int ue_thp_dl = 0;
    long int ue_thp_ul = 0;
    for (int i = 0; i < data->additional_data.numUes; i++) {
      ue_thp_dl += data->additional_data.ues_thp[i].dl;
      ue_thp_ul += data->additional_data.ues_thp[i].ul;
    }

    pm_data_t pm_data = {
        .numUes = data->additional_data.numUes,
        .load = data->additional_data.load,
        .ue_thp_dl_sum = ue_thp_dl,
        .ue_thp_ul_sum = ue_thp_ul,
    };

    rc = pm_data_feed(&pm_data);
    if (rc) {
      log_error("pm_data_feed failed");
      goto failure;
    }

  alarms_data_t alarms_data = {
      .load = data->additional_data.load,
  };

  rc = alarms_data_feed(&alarms_data);
  if (rc) {
    log_error("alarms_data_feed failed");
    goto failure;
  }
  free(pm_data_info.vendor);
 }

  free(ves_info.managed_element_id);
  free(ves_info.vendor);

  return 0;

failure:
  free(pm_data_info.vendor);
  free(ves_info.managed_element_id);
  free(ves_info.vendor);

  return 1;
}
