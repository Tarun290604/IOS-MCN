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

#include "oai_data.h"
#include "common/log.h"
#include <cjson/cJSON.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

extern char *nodeType;

oai_data_t *oai_data_parse_json(const char *json)
{
  cJSON *cjson = 0;

  oai_data_t *data = (oai_data_t *)malloc(sizeof(oai_data_t));
  if (data == 0) {
    log_error("oai_data_t malloc failed");
    goto failed;
  }

  memset(data, 0, sizeof(oai_data_t));

  cJSON *object;

  cjson = cJSON_Parse(json);
  if (cjson == 0) {
    log_error("cJSON parse failed");
    goto failed;
  }

  cJSON *o1 = cJSON_GetObjectItem(cjson, "o1-config");
  if (o1 == 0) {
    log_error("not found: o1-config");
    goto failed;
  }

  if (strcmp(nodeType, "cu-up") == 0) {
    cJSON *gnbcuup = cJSON_GetObjectItem(o1, "GNBCUUP");
    object = cJSON_GetObjectItem(gnbcuup, "nrgnbcuup3gpp:gNBId");
    if (object == 0) {
      log_error("not found: nrgnbcuup3gpp:gNBId");
      goto failed;
    }
    if (!cJSON_IsNumber(object)) {
      log_error("failed type: nrgnbcuup3gpp:gNBId");
      goto failed;
    }
    data->gnbcuup.gNBId = object->valueint;

    object = cJSON_GetObjectItem(gnbcuup, "nrgnbcuup3gpp:cu_up_id");
    if (object == 0) {
      log_error("not found: nrgnbcuup3gpp:cu_up_id");
      goto failed;
    }
    if (!cJSON_IsNumber(object)) {
      log_error("failed type: nrgnbcuup3gpp:cu_up_id");
      goto failed;
    }
    data->gnbcuup.gNBCUUPId = object->valueint;

    object = cJSON_GetObjectItem(gnbcuup, "nrgnbcuup3gpp:mcc");
    if (object == 0) {
      log_error("not found: nrgnbcuup3gpp:mcc");
      goto failed;
    }
    if (!cJSON_IsString(object)) {
      log_error("failed type: nrgnbcuup3gpp:mcc");
      goto failed;
    }
    data->gnbcuup.mcc = strdup(object->valuestring);

    object = cJSON_GetObjectItem(gnbcuup, "nrgnbcuup3gpp:mnc");
    if (object == 0) {
      log_error("not found: mnc");
      goto failed;
    }
    if (!cJSON_IsString(object)) {
      log_error("failed type: mnc");
      goto failed;
    }
    data->gnbcuup.mnc = strdup(object->valuestring);

    object = cJSON_GetObjectItem(gnbcuup, "nrgnbcuup3gpp:mnc_length");
    if (object == 0) {
      log_error("not found: nrgnbcuup3gpp:mnc_length");
      goto failed;
    }
    if (!cJSON_IsNumber(object)) {
      log_error("failed type: nrgnbcuup3gpp:mnc_length");
      goto failed;
    }
    data->gnbcuup.mnc_length = object->valueint;

    object = cJSON_GetObjectItem(gnbcuup, "vendor");
    if (object == 0) {
      log_error("not found: vendor");
      goto failed;
    }
    if (!cJSON_IsString(object)) {
      log_error("failed type: vendor");
      goto failed;
    }
    data->device_data.vendor = strdup(object->valuestring);

  }

  else {
    if (strcmp(nodeType, "cu-cp") == 0 || strcmp(nodeType, "gNB") == 0) {
      cJSON *nrfreqrel = cJSON_GetObjectItem(o1, "NRFREQREL");
      object = cJSON_GetObjectItem(nrfreqrel, "nrfreqrel3gpp:pMax");
      if (object == 0) {
        log_error("not found: nrfreqrel3gpp:pMax");
        goto failed;
      }
      if (!cJSON_IsNumber(object)) {
        log_error("failed type: nrfreqrel3gpp:pMax");
        goto failed;
      }
      data->nrfreqrel.pMax = object->valueint;

      cJSON *nrcellcu = cJSON_GetObjectItem(o1, "NRCELLCU");
      object = cJSON_GetObjectItem(nrcellcu, "nrcellcu3gpp:cellLocalId");
      if (object == 0) {
        log_error("not found: nrcellcu3gpp:cellLocalId");
        goto failed;
      }
      if (!cJSON_IsNumber(object)) {
        log_error("failed type: nrcellcu3gpp:cellLocalId");
        goto failed;
      }
      data->nrcellcu.cellLocalId = object->valueint;

      object = cJSON_GetObjectItem(nrcellcu, "nrcellcu3gpp:mcc");
      if (object == 0) {
        log_error("not found: mcc");
        goto failed;
      }
      if (!cJSON_IsString(object)) {
        log_error("failed type: mcc");
        goto failed;
      }
      data->nrcellcu.mcc = strdup(object->valuestring);

      object = cJSON_GetObjectItem(nrcellcu, "nrcellcu3gpp:mnc");
      if (object == 0) {
        log_error("not found: mnc");
        goto failed;
      }
      if (!cJSON_IsString(object)) {
        log_error("failed type: mnc");
        goto failed;
      }
      data->nrcellcu.mnc = strdup(object->valuestring);

      object = cJSON_GetObjectItem(nrcellcu, "nrcellcu3gpp:sst");
      if (object == 0) {
        log_error("not found: sst");
        goto failed;
      }
      if (!cJSON_IsNumber(object)) {
        log_error("failed type: sst");
        goto failed;
      }
      data->nrcellcu.sst = object->valueint;

      object = cJSON_GetObjectItem(nrcellcu, "nrcellcu3gpp:sd");
      if (object == 0) {
        log_error("not found: sd");
        goto failed;
      }
      if (!cJSON_IsNumber(object)) {
        log_error("failed type: sd");
        goto failed;
      }
      data->nrcellcu.sd = object->valueint;
    }

    if (strcmp(nodeType, "du") == 0 || strcmp(nodeType, "gNB") == 0) {
      cJSON *bwp = cJSON_GetObjectItem(o1, "BWP");
      if (bwp == 0) {
        log_error("not found: BWP");
        goto failed;
      }

      {
        cJSON *bwp_dl = cJSON_GetObjectItem(bwp, "dl");
        if (bwp_dl == 0) {
          log_error("not found: dl");
          goto failed;
        }

        if (!cJSON_IsArray(bwp_dl)) {
          log_error("bwp_dl not array");
          goto failed;
        }

        cJSON *bwpElement = cJSON_GetArrayItem(bwp_dl, 0);

        object = cJSON_GetObjectItem(bwpElement, "bwp3gpp:isInitialBwp");
        if (object == 0) {
          log_error("not found: bwp3gpp:isInitialBwp");
          goto failed;
        }
        if (!cJSON_IsBool(object)) {
          log_error("failed type: bwp3gpp:isInitialBwp");
          goto failed;
        }
        data->bwp[0].isInitialBwp = cJSON_IsTrue(object);

        object = cJSON_GetObjectItem(bwpElement, "bwp3gpp:numberOfRBs");
        if (object == 0) {
          log_error("not found: bwp3gpp:numberOfRBs");
          goto failed;
        }
        if (!cJSON_IsNumber(object)) {
          log_error("failed type: bwp3gpp:numberOfRBs");
          goto failed;
        }
        data->bwp[0].numberOfRBs = object->valueint;

        object = cJSON_GetObjectItem(bwpElement, "bwp3gpp:startRB");
        if (object == 0) {
          log_error("not found: bwp3gpp:startRB");
          goto failed;
        }
        if (!cJSON_IsNumber(object)) {
          log_error("failed type: bwp3gpp:startRB");
          goto failed;
        }
        data->bwp[0].startRB = object->valueint;

        object = cJSON_GetObjectItem(bwpElement, "bwp3gpp:subCarrierSpacing");
        if (object == 0) {
          log_error("not found: bwp3gpp:subCarrierSpacing");
          goto failed;
        }
        if (!cJSON_IsNumber(object)) {
          log_error("failed type: bwp3gpp:subCarrierSpacing");
          goto failed;
        }
        switch (object->valueint) {
          case 0:
            data->bwp[0].subCarrierSpacing = 15;
            break;

          case 1:
            data->bwp[0].subCarrierSpacing = 30;
            break;

          case 2:
            data->bwp[0].subCarrierSpacing = 60;
            break;

          case 3:
            data->bwp[0].subCarrierSpacing = 120;
            break;

          default:
            data->bwp[0].subCarrierSpacing = object->valueint;
        }
      }

      {
        cJSON *bwp_ul = cJSON_GetObjectItem(bwp, "ul");
        if (bwp_ul == 0) {
          log_error("not found: ul");
          goto failed;
        }

        if (!cJSON_IsArray(bwp_ul)) {
          log_error("bwp_ul not array");
          goto failed;
        }

        cJSON *bwpElement = cJSON_GetArrayItem(bwp_ul, 0);

        object = cJSON_GetObjectItem(bwpElement, "bwp3gpp:isInitialBwp");
        if (object == 0) {
          log_error("not found: bwp3gpp:isInitialBwp");
          goto failed;
        }
        if (!cJSON_IsBool(object)) {
          log_error("failed type: bwp3gpp:isInitialBwp");
          goto failed;
        }
        data->bwp[1].isInitialBwp = cJSON_IsTrue(object);

        object = cJSON_GetObjectItem(bwpElement, "bwp3gpp:numberOfRBs");
        if (object == 0) {
          log_error("not found: bwp3gpp:numberOfRBs");
          goto failed;
        }
        if (!cJSON_IsNumber(object)) {
          log_error("failed type: bwp3gpp:numberOfRBs");
          goto failed;
        }
        data->bwp[1].numberOfRBs = object->valueint;

        object = cJSON_GetObjectItem(bwpElement, "bwp3gpp:startRB");
        if (object == 0) {
          log_error("not found: bwp3gpp:startRB");
          goto failed;
        }
        if (!cJSON_IsNumber(object)) {
          log_error("failed type: bwp3gpp:startRB");
          goto failed;
        }
        data->bwp[1].startRB = object->valueint;

        object = cJSON_GetObjectItem(bwpElement, "bwp3gpp:subCarrierSpacing");
        if (object == 0) {
          log_error("not found: bwp3gpp:subCarrierSpacing");
          goto failed;
        }
        if (!cJSON_IsNumber(object)) {
          log_error("failed type: bwp3gpp:subCarrierSpacing");
          goto failed;
        }
        switch (object->valueint) {
          case 0:
            data->bwp[1].subCarrierSpacing = 15;
            break;

          case 1:
            data->bwp[1].subCarrierSpacing = 30;
            break;

          case 2:
            data->bwp[1].subCarrierSpacing = 60;
            break;

          case 3:
            data->bwp[1].subCarrierSpacing = 120;
            break;

          default:
            data->bwp[1].subCarrierSpacing = object->valueint;
        }
      }

      cJSON *nrcelldu = cJSON_GetObjectItem(o1, "NRCELLDU");
      if (nrcelldu == 0) {
        log_error("not found: NRCELLDU");
        goto failed;
      }

      object = cJSON_GetObjectItem(nrcelldu, "nrcelldu3gpp:ssbFrequency");
      if (object == 0) {
        log_error("not found: nrcelldu3gpp:ssbFrequency");
        goto failed;
      }
      if (!cJSON_IsNumber(object)) {
        log_error("failed type: nrcelldu3gpp:ssbFrequency");
        goto failed;
      }
      data->nrcelldu.ssbFrequency = object->valueint;

      object = cJSON_GetObjectItem(nrcelldu, "nrcelldu3gpp:arfcnDL");
      if (object == 0) {
        log_error("not found: nrcelldu3gpp:arfcnDL");
        goto failed;
      }
      if (!cJSON_IsNumber(object)) {
        log_error("failed type: nrcelldu3gpp:arfcnDL");
        goto failed;
      }
      data->nrcelldu.arfcnDL = object->valueint;

      object = cJSON_GetObjectItem(nrcelldu, "nrcelldu3gpp:bSChannelBwDL");
      if (object == 0) {
        log_error("not found: nrcelldu3gpp:bSChannelBwDL");
        goto failed;
      }
      if (!cJSON_IsNumber(object)) {
        log_error("failed type: nrcelldu3gpp:bSChannelBwDL");
        goto failed;
      }
      data->nrcelldu.bSChannelBwDL = object->valueint;

      object = cJSON_GetObjectItem(nrcelldu, "nrcelldu3gpp:arfcnUL");
      if (object == 0) {
        log_error("not found: nrcelldu3gpp:arfcnUL");
        goto failed;
      }
      if (!cJSON_IsNumber(object)) {
        log_error("failed type: nrcelldu3gpp:arfcnUL");
        goto failed;
      }
      data->nrcelldu.arfcnUL = object->valueint;

      object = cJSON_GetObjectItem(nrcelldu, "nrcelldu3gpp:bSChannelBwUL");
      if (object == 0) {
        log_error("not found: nrcelldu3gpp:bSChannelBwUL");
        goto failed;
      }
      if (!cJSON_IsNumber(object)) {
        log_error("failed type: nrcelldu3gpp:bSChannelBwUL");
        goto failed;
      }
      data->nrcelldu.bSChannelBwUL = object->valueint;

      object = cJSON_GetObjectItem(nrcelldu, "nrcelldu3gpp:nRPCI");
      if (object == 0) {
        log_error("not found: nrcelldu3gpp:nRPCI");
        goto failed;
      }
      if (!cJSON_IsNumber(object)) {
        log_error("failed type: nrcelldu3gpp:nRPCI");
        goto failed;
      }
      data->nrcelldu.nRPCI = object->valueint;

      object = cJSON_GetObjectItem(nrcelldu, "nrcelldu3gpp:nRTAC");
      if (object == 0) {
        log_error("not found: nrcelldu3gpp:nRTAC");
        goto failed;
      }
      if (!cJSON_IsNumber(object)) {
        log_error("failed type: nrcelldu3gpp:nRTAC");
        goto failed;
      }
      data->nrcelldu.nRTAC = object->valueint;

      object = cJSON_GetObjectItem(nrcelldu, "nrcelldu3gpp:mcc");
      if (object == 0) {
        log_error("not found: nrcelldu3gpp:mcc");
        goto failed;
      }
      if (!cJSON_IsString(object)) {
        log_error("failed type: nrcelldu3gpp:mcc");
        goto failed;
      }
      data->nrcelldu.mcc = strdup(object->valuestring);

      object = cJSON_GetObjectItem(nrcelldu, "nrcelldu3gpp:mnc");
      if (object == 0) {
        log_error("not found: nrcelldu3gpp:mnc");
        goto failed;
      }
      if (!cJSON_IsString(object)) {
        log_error("failed type: nrcelldu3gpp:mnc");
        goto failed;
      }
      data->nrcelldu.mnc = strdup(object->valuestring);

      object = cJSON_GetObjectItem(nrcelldu, "nrcelldu3gpp:sd");
      if (object == 0) {
        log_error("not found: nrcelldu3gpp:sd");
        goto failed;
      }
      if (!cJSON_IsNumber(object)) {
        log_error("failed type: nrcelldu3gpp:sd");
        goto failed;
      }
      data->nrcelldu.sd = object->valueint;

      object = cJSON_GetObjectItem(nrcelldu, "nrcelldu3gpp:sst");
      if (object == 0) {
        log_error("not found: nrcelldu3gpp:sst");
        goto failed;
      }
      if (!cJSON_IsNumber(object)) {
        log_error("failed type: nrcelldu3gpp:sst");
        goto failed;
      }
      data->nrcelldu.sst = object->valueint;
    }

    cJSON *device = cJSON_GetObjectItem(o1, "device");
    if (device == 0) {
      log_error("not found: device");
      goto failed;
    }

    if (strcmp(nodeType, "du") == 0 || strcmp(nodeType, "gNB") == 0) {
      object = cJSON_GetObjectItem(device, "gnbId");
      if (object == 0) {
        log_error("not found: gnbId");
        goto failed;
      }
      if (!cJSON_IsNumber(object)) {
        log_error("failed type: gnbId");
        goto failed;
      }
      data->device_data.gnbId = object->valueint;

      object = cJSON_GetObjectItem(device, "gnbName");
      if (object == 0) {
        log_error("not found: gnbName");
        goto failed;
      }
      if (!cJSON_IsString(object)) {
        log_error("failed type: gnbName");
        goto failed;
      }
      data->device_data.gnbName = strdup(object->valuestring);
    }
    if (strcmp(nodeType, "cu-cp") == 0 || strcmp(nodeType, "gNB") == 0) {
      object = cJSON_GetObjectItem(device, "gNBId");
      if (object == 0) {
        log_error("not found: gNBId");
        goto failed;
      }
      if (!cJSON_IsNumber(object)) {
        log_error("failed type: gNBId");
        goto failed;
      }
      data->device_data.gnbId = object->valueint;

      object = cJSON_GetObjectItem(device, "gnbCUName");
      if (object == 0) {
        log_error("not found: gnbCUName");
        goto failed;
      }
      if (!cJSON_IsString(object)) {
        log_error("failed type: gnbCUName");
        goto failed;
      }
      data->device_data.gnbCUName = strdup(object->valuestring);
    }
    if (strcmp(nodeType, "du") == 0 || strcmp(nodeType, "gNB") == 0) {
      object = cJSON_GetObjectItem(device, "vendor");
      if (object == 0) {
        log_error("not found: vendor");
        goto failed;
      }
      if (!cJSON_IsString(object)) {
        log_error("failed type: vendor");
        goto failed;
      }
      data->device_data.vendor = strdup(object->valuestring);
    }

    cJSON *additional = cJSON_GetObjectItem(cjson, "O1-Operational");
    if (additional == 0) {
      log_error("not found: O1-Operational");
      goto failed;
    }
    if (strcmp(nodeType, "du") == 0 || strcmp(nodeType, "gNB") == 0) {
      object = cJSON_GetObjectItem(additional, "frame-type");
      if (object == 0) {
        log_error("not found: frame-type");
        goto failed;
      }
      if (!cJSON_IsString(object)) {
        log_error("failed type: frame-type");
        goto failed;
      }
      data->additional_data.frameType = strdup(object->valuestring);

      object = cJSON_GetObjectItem(additional, "band-number");
      if (object == 0) {
        log_error("not found: band-number");
        goto failed;
      }
      if (!cJSON_IsNumber(object)) {
        log_error("failed type: band-number");
        goto failed;
      }
      data->additional_data.bandNumber = object->valueint;

      object = cJSON_GetObjectItem(additional, "ues");
      if (object == 0) {
        log_error("not found: ues");
        goto failed;
      }

      if (!cJSON_IsArray(object)) {
        log_error("not array: ues");
        goto failed;
      }
    }
    if (strcmp(nodeType, "cu-cp") == 0 || strcmp(nodeType, "gNB") == 0) {
      object = cJSON_GetObjectItem(additional, "num_dus");
      if (object == 0) {
        log_error("not found: num_dus");
        goto failed;
      }
      if (!cJSON_IsNumber(object)) {
        log_error("failed type: num_dus");
        goto failed;
      }
      data->additional_data.num_dus = object->valueint;

      object = cJSON_GetObjectItem(additional, "num_cuups");
      if (object == 0) {
        log_error("not found: num_cuups");
        goto failed;
      }
      if (!cJSON_IsNumber(object)) {
        log_error("failed type: num_cuups");
        goto failed;
      }
      data->additional_data.num_cuups = object->valueint;

      object = cJSON_GetObjectItem(additional, "vendor");
      if (object == 0) {
        log_error("not found: vendor");
        goto failed;
      }
      if (!cJSON_IsString(object)) {
        log_error("failed type: vendor");
        goto failed;
      }
      data->device_data.vendor = strdup(object->valuestring);
    }

    if (strcmp(nodeType, "du") == 0 || strcmp(nodeType, "gNB") == 0) {
      data->additional_data.numUes = cJSON_GetArraySize(object);
      data->additional_data.ues = (int *)malloc(data->additional_data.numUes * sizeof(int));
      if (!data->additional_data.ues) {
        log_error("malloc failed: ues");
        goto failed;
      }
      for (int i = 0; i < data->additional_data.numUes; i++) {
        cJSON *ue = cJSON_GetArrayItem(object, i);
        if (ue == 0) {
          log_error("not found: ues[i]");
          goto failed;
        }
        if (!cJSON_IsNumber(ue)) {
          log_error("failed type: ues[i]");
          goto failed;
        }
        data->additional_data.ues[i] = ue->valueint;
      }

      object = cJSON_GetObjectItem(additional, "load");
      if (object == 0) {
        log_error("not found: load");
        goto failed;
      }
      if (!cJSON_IsNumber(object)) {
        log_error("failed type: load");
        goto failed;
      }
      data->additional_data.load = object->valueint;

      object = cJSON_GetObjectItem(additional, "ues-thp");
      data->additional_data.ues_thp = (oai_ues_thp_t *)malloc(data->additional_data.numUes * sizeof(oai_ues_thp_t));
      if (!data->additional_data.ues_thp) {
        log_error("malloc failed: ues_thp");
        goto failed;
      }
      for (int i = 0; i < data->additional_data.numUes; i++) {
        cJSON *thpUe = cJSON_GetArrayItem(object, i);
        if (thpUe == 0) {
          log_error("not found: thpUe");
          goto failed;
        }

        cJSON *objectItem = cJSON_GetObjectItem(thpUe, "rnti");
        if (!cJSON_IsNumber(objectItem)) {
          log_error("failed type: rnti");
          goto failed;
        }
        data->additional_data.ues_thp[i].rnti = objectItem->valueint;

        objectItem = cJSON_GetObjectItem(thpUe, "dl");
        if (!cJSON_IsNumber(objectItem)) {
          log_error("failed type: dl");
          goto failed;
        }
        data->additional_data.ues_thp[i].dl = objectItem->valueint;

        objectItem = cJSON_GetObjectItem(thpUe, "ul");
        if (!cJSON_IsNumber(objectItem)) {
          log_error("failed type: ul");
          goto failed;
        }
        data->additional_data.ues_thp[i].ul = objectItem->valueint;
      }
      cJSON *oru = cJSON_GetObjectItem(o1, "o-ru-stats");
      if (oru == 0) {
        log_error("not found: o-ru-stats");
        goto failed;
      }
      {
        cJSON *o_ran_uplane_conf = cJSON_GetObjectItem(oru, "o-ran-uplane-conf");

        if (o_ran_uplane_conf == 0) {
          log_error("not found: o-ran-uplane-conf");
          goto failed;
        }

        if (!cJSON_IsArray(o_ran_uplane_conf)) {
          log_error("o-ran-uplane-conf not array");
          goto failed;
        }

        cJSON *o_ran_uplane_conf_element = cJSON_GetArrayItem(o_ran_uplane_conf, 0);

        object = cJSON_GetObjectItem(o_ran_uplane_conf_element, "tx-array-carrier:absolute-frequency-center");
        if (object == 0) {
          log_error("not found: tx-array-carrier:absolute-frequency-center");
          goto failed;
        }
        if (!cJSON_IsNumber(object)) {
          log_error("failed type:  tx-array-carrier:absolute-frequency-center");
          goto failed;
        }
        data->oru.tx_array_carrier.arfcn_center = object->valueint;

        object = cJSON_GetObjectItem(o_ran_uplane_conf_element, "tx-array-carrier:center-of-channel-bandwidth");
        if (object == 0) {
          log_error("not found: tx-array-carrier:center-of-channel-bandwidth");
          goto failed;
        }
        if (!cJSON_IsNumber(object)) {
          log_error("failed type:  tx-array-carrier:center-of-channel-bandwidth");
          goto failed;
        }
        data->oru.tx_array_carrier.center_channel_bw = (uint32_t)object->valuedouble;

        object = cJSON_GetObjectItem(o_ran_uplane_conf_element, "tx-array-carrier:channel-bandwidth");
        if (object == 0) {
          log_error("not found: tx-array-carrier:channel-bandwidth");
          goto failed;
        }
        if (!cJSON_IsNumber(object)) {
          log_error("failed type:  tx-array-carrier:channel-bandwidth");
          goto failed;
        }
        data->oru.tx_array_carrier.channel_bw = object->valueint;

        object = cJSON_GetObjectItem(o_ran_uplane_conf_element, "tx-array-carrier:active");
        if (object == 0) {
          log_error("not found: tx-array-carrier:active");
          goto failed;
        }
        if (!cJSON_IsString(object)) {
          log_error("failed type: tx-array-carrier:active");
          goto failed;
        }
        if (strcmp(object->valuestring, "INACTIVE") == 0 || strcmp(object->valuestring, "SLEEP") == 0
            || strcmp(object->valuestring, "ACTIVE") == 0)
          strcpy(data->oru.tx_array_carrier.ru_carrier, object->valuestring);
        else
          printf("INAVALID RU CARRIER STATE\n");

        object = cJSON_GetObjectItem(o_ran_uplane_conf_element, "tx-array-carrier:rw-duplex-scheme");
        if (object == 0) {
          log_error("not found: tx-array-carrier:rw-duplex-scheme");
          goto failed;
        }
        if (!cJSON_IsString(object)) {
          log_error("failed type: tx-array-carrier:rw-duplex-scheme");
          goto failed;
        }
        if (strcmp(object->valuestring, "TDD") == 0 || strcmp(object->valuestring, "FDD") == 0)
          strcpy(data->oru.tx_array_carrier.rw_duplex_scheme, object->valuestring);
        else
          printf("INAVALID RU CARRIER DUPLEX SCHEME TYPE\n");

        object = cJSON_GetObjectItem(o_ran_uplane_conf_element, "tx-array-carrier:rw-type");
        if (object == 0) {
          log_error("not found: tx-array-carrier:rw-type");
          goto failed;
        }
        if (!cJSON_IsString(object)) {
          log_error("failed type: tx-array-carrier:rw-type");
          goto failed;
        }
        if (strcmp(object->valuestring, "LTE") == 0 || strcmp(object->valuestring, "NR") == 0)
          strcpy(data->oru.tx_array_carrier.rw_type, object->valuestring);
        else
          printf("INVALID RU CARRIER DUPLEX TYPE\n");

        object = cJSON_GetObjectItem(o_ran_uplane_conf_element, "tx-array-carrier:gain");
        if (object == 0) {
          log_error("not found: tx-array-carrier:gain");
          goto failed;
        }
        if (!cJSON_IsNumber(object)) {
          log_error("failed type:  tx-array-carrier:gain");
          goto failed;
        }
        data->oru.tx_array_carrier.gain = object->valueint;

        object = cJSON_GetObjectItem(o_ran_uplane_conf_element, "tx-array-carrier:downlink-radio-frame-offset");
        if (object == 0) {
          log_error("not found: tx-array-carrier:downlink-radio-frame-offset");
          goto failed;
        }
        if (!cJSON_IsNumber(object)) {
          log_error("failed type:  tx-array-carrier:downlink-radio-frame-offset");
          goto failed;
        }
        data->oru.tx_array_carrier.dl_radio_frame_offset = object->valueint;

        object = cJSON_GetObjectItem(o_ran_uplane_conf_element, "tx-array-carrier:downlink-sfn-offset");
        if (object == 0) {
          log_error("not found: tx-array-carrier:downlink-sfn-offset");
          goto failed;
        }
        if (!cJSON_IsNumber(object)) {
          log_error("failed type:  tx-array-carrier:downlink-sfn-offset");
          goto failed;
        }
        data->oru.tx_array_carrier.dl_sfn_offset = object->valueint;

        o_ran_uplane_conf_element = cJSON_GetArrayItem(o_ran_uplane_conf, 1);

        object = cJSON_GetObjectItem(o_ran_uplane_conf_element, "rx-array-carrier:absolute-frequency-center");
        if (object == 0) {
          log_error("not found: rx-array-carrier:absolute-frequency-center");
          goto failed;
        }
        if (!cJSON_IsNumber(object)) {
          log_error("failed type:  rx-array-carrier:absolute-frequency-center");
          goto failed;
        }
        data->oru.rx_array_carrier.arfcn_center = object->valueint;

        object = cJSON_GetObjectItem(o_ran_uplane_conf_element, "rx-array-carrier:center-of-channel-bandwidth");
        if (object == 0) {
          log_error("not found: rx-array-carrier:center-of-channel-bandwidth");
          goto failed;
        }
        if (!cJSON_IsNumber(object)) {
          log_error("failed type:  rx-array-carrier:center-of-channel-bandwidth");
          goto failed;
        }
        data->oru.rx_array_carrier.center_channel_bw = (uint32_t)object->valuedouble;

        object = cJSON_GetObjectItem(o_ran_uplane_conf_element, "rx-array-carrier:channel-bandwidth");
        if (object == 0) {
          log_error("not found: rx-array-carrier:channel-bandwidth");
          goto failed;
        }
        if (!cJSON_IsNumber(object)) {
          log_error("failed type:  rx-array-carrier:channel-bandwidth");
          goto failed;
        }
        data->oru.rx_array_carrier.channel_bw = object->valueint;

        object = cJSON_GetObjectItem(o_ran_uplane_conf_element, "rx-array-carrier:active");
        if (object == 0) {
          log_error("not found: rx-array-carrier:active");
          goto failed;
        }
        if (!cJSON_IsString(object)) {
          log_error("failed type: rx-array-carrier:active");
          goto failed;
        }
        if (strcmp(object->valuestring, "INACTIVE") == 0 || strcmp(object->valuestring, "SLEEP") == 0
            || strcmp(object->valuestring, "ACTIVE") == 0)
          strcpy(data->oru.rx_array_carrier.ru_carrier, object->valuestring);
        else
          printf("INAVALID RU CARRIER STATE\n");

        object = cJSON_GetObjectItem(o_ran_uplane_conf_element, "rx-array-carrier:downlink-radio-frame-offset");
        if (object == 0) {
          log_error("not found: rx-array-carrier:downlink-radio-frame-offset");
          goto failed;
        }
        if (!cJSON_IsNumber(object)) {
          log_error("failed type:  rx-array-carrier:downlink-radio-frame-offset");
          goto failed;
        }
        data->oru.rx_array_carrier.dl_radio_frame_offset = object->valueint;

        object = cJSON_GetObjectItem(o_ran_uplane_conf_element, "rx-array-carrier:downlink-sfn-offset");
        if (object == 0) {
          log_error("not found: rx-array-carrier:downlink-sfn-offset");
          goto failed;
        }
        if (!cJSON_IsNumber(object)) {
          log_error("failed type:  rx-array-carrier:downlink-sfn-offset");
          goto failed;
        }
        data->oru.rx_array_carrier.dl_sfn_offset = object->valueint;

        object = cJSON_GetObjectItem(o_ran_uplane_conf_element, "rx-array-carrier:gain-correction");
        if (object == 0) {
          log_error("not found: rx-array-carrier:gain-correction");
          goto failed;
        }
        if (!cJSON_IsNumber(object)) {
          log_error("failed type:  rx-array-carrier:gain-correction");
          goto failed;
        }
        data->oru.rx_array_carrier.gain_correction = object->valueint;

        object = cJSON_GetObjectItem(o_ran_uplane_conf_element, "rx-array-carrier:n-ta-offset");
        if (object == 0) {
          log_error("not found: rx-array-carrier:n-ta-offset");
          goto failed;
        }
        if (!cJSON_IsNumber(object)) {
          log_error("failed type:  rx-array-carrier:n-ta-offset");
          goto failed;
        }
        data->oru.rx_array_carrier.nta_offset = object->valueint;
      }
      {
        cJSON *delay_management = cJSON_GetObjectItem(oru, "delay-management");

        if (delay_management == 0) {
          log_error("not found: delay-management");
          goto failed;
        }

        object = cJSON_GetObjectItem(delay_management, "ru-delay-profile:t2a-min-up");
        if (object == 0) {
          log_error("not found: ru-delay-profile:t2a-min-up");
          goto failed;
        }
        if (!cJSON_IsNumber(object)) {
          log_error("failed type:  ru-delay-profile:t2a-min-up");
          goto failed;
        }
        data->oru.delay_management.T2a_min_up = object->valueint;

        object = cJSON_GetObjectItem(delay_management, "ru-delay-profile:t2a-max-up");
        if (object == 0) {
          log_error("not found: ru-delay-profile:t2a-max-up");
          goto failed;
        }
        if (!cJSON_IsNumber(object)) {
          log_error("failed type:  ru-delay-profile:t2a-max-up");
          goto failed;
        }
        data->oru.delay_management.T2a_max_up = object->valueint;

        object = cJSON_GetObjectItem(delay_management, "ru-delay-profile:t2a-min-cp-dl");
        if (object == 0) {
          log_error("not found: ru-delay-profile:t2a-min-cp-dl");
          goto failed;
        }
        if (!cJSON_IsNumber(object)) {
          log_error("failed type:  ru-delay-profile:t2a-min-cp-dl");
          goto failed;
        }
        data->oru.delay_management.T2a_min_cp_dl = object->valueint;

        object = cJSON_GetObjectItem(delay_management, "ru-delay-profile:t2a-max-cp-dl");
        if (object == 0) {
          log_error("not found: ru-delay-profile:t2a-max-cp-dl");
          goto failed;
        }
        if (!cJSON_IsNumber(object)) {
          log_error("failed type:  ru-delay-profile:t2a-max-cp-dl");
          goto failed;
        }
        data->oru.delay_management.T2a_max_cp_dl = object->valueint;

        object = cJSON_GetObjectItem(delay_management, "ru-delay-profile:tcp-adv-dl");
        if (object == 0) {
          log_error("not found: ru-delay-profile:tcp-adv-dl");
          goto failed;
        }
        if (!cJSON_IsNumber(object)) {
          log_error("failed type:  ru-delay-profile:tcp-adv-dl");
          goto failed;
        }
        data->oru.delay_management.Tcp_adv_dl = object->valueint;

        object = cJSON_GetObjectItem(delay_management, "ru-delay-profile:ta3-min");
        if (object == 0) {
          log_error("not found: ru-delay-profile:ta3-min");
          goto failed;
        }
        if (!cJSON_IsNumber(object)) {
          log_error("failed type:  ru-delay-profile:ta3-min");
          goto failed;
        }
        data->oru.delay_management.Ta3_min = object->valueint;

        object = cJSON_GetObjectItem(delay_management, "ru-delay-profile:ta3-max");
        if (object == 0) {
          log_error("not found: ru-delay-profile:ta3-max");
          goto failed;
        }
        if (!cJSON_IsNumber(object)) {
          log_error("failed type:  ru-delay-profile:ta3-max");
          goto failed;
        }
        data->oru.delay_management.Ta3_max = object->valueint;

        object = cJSON_GetObjectItem(delay_management, "ru-delay-profile:t2a-min-cp-ul");
        if (object == 0) {
          log_error("not found: ru-delay-profile:t2a-min-cp-ul");
          goto failed;
        }
        if (!cJSON_IsNumber(object)) {
          log_error("failed type:  ru-delay-profile:t2a-min-cp-ul");
          goto failed;
        }
        data->oru.delay_management.T2a_min_cp_ul = object->valueint;

        object = cJSON_GetObjectItem(delay_management, "ru-delay-profile:t2a-max-cp-ul");
        if (object == 0) {
          log_error("not found: ru-delay-profile:t2a-max-cp-ul");
          goto failed;
        }
        if (!cJSON_IsNumber(object)) {
          log_error("failed type:  ru-delay-profile:t2a-max-cp-ul");
          goto failed;
        }
        data->oru.delay_management.T2a_max_cp_ul = object->valueint;
      }
    }
  }
  cJSON_Delete(cjson);
  return data;

failed:
  oai_data_free(data);
  cJSON_Delete(cjson);

  return 0;
}

void oai_data_print(const oai_data_t *data)
{
  log("==========================\n");
  log("BWP[Downlink].isInitialBwp: %d\n", data->bwp[0].isInitialBwp);
  log("BWP[Downlink].numberOfRBs: %d\n", data->bwp[0].numberOfRBs);
  log("BWP[Downlink].startRB: %d\n", data->bwp[0].startRB);
  log("BWP[Downlink].subCarrierSpacing: %d\n", data->bwp[0].subCarrierSpacing);
  log("\n");
  log("BWP[Uplink].isInitialBwp: %d\n", data->bwp[1].isInitialBwp);
  log("BWP[Uplink].numberOfRBs: %d\n", data->bwp[1].numberOfRBs);
  log("BWP[Uplink].startRB: %d\n", data->bwp[1].startRB);
  log("BWP[Uplink].subCarrierSpacing: %d\n", data->bwp[1].subCarrierSpacing);
  log("\n");
  log("NRCELLDU.ssbFrequency: %d\n", data->nrcelldu.ssbFrequency);
  log("NRCELLDU.arfcnDL: %d\n", data->nrcelldu.arfcnDL);
  log("NRCELLDU.bSChannelBwDL: %d\n", data->nrcelldu.bSChannelBwDL);
  log("NRCELLDU.arfcnUL: %d\n", data->nrcelldu.arfcnUL);
  log("NRCELLDU.bSChannelBwUL: %d\n", data->nrcelldu.bSChannelBwUL);
  log("NRCELLDU.nRPCI: %d\n", data->nrcelldu.nRPCI);
  log("NRCELLDU.nRTAC: %d\n", data->nrcelldu.nRTAC);
  log("\n");
  log("NRCELLDU.mcc: %s\n", data->nrcelldu.mcc);
  log("NRCELLDU.mnc: %s\n", data->nrcelldu.mnc);
  log("NRCELLDU.sd: %d\n", data->nrcelldu.sd);
  log("NRCELLDU.sst: %d\n", data->nrcelldu.sst);
  log("\n");
  log("DEVICE.gnbId: %d\n", data->device_data.gnbId);
  log("DEVICE.gnbName: %s\n", data->device_data.gnbName);
  log("DEVICE.vendor: %s\n", data->device_data.vendor);

  log("\n");
  log("ADDITIONAL.frameType: %s\n", data->additional_data.frameType);
  log("ADDITIONAL.bandNumber: %d\n", data->additional_data.bandNumber);
  log("ADDITIONAL.UEs[%d]: \n", data->additional_data.numUes);
  for (int i = 0; i < data->additional_data.numUes; i++) {
    log(" - [%d]: %d\n", i, data->additional_data.ues[i]);
  }
  log("ADDITIONAL.load: %d\n", data->additional_data.load);
  log("ADDITIONAL.UEs-THP[%d]: \n", data->additional_data.numUes);
  for (int i = 0; i < data->additional_data.numUes; i++) {
    log(" - rnti[%d]: %d\n", i, data->additional_data.ues_thp[i].rnti);
    log(" - dl[%d]: %d\n", i, data->additional_data.ues_thp[i].dl);
    log(" - ul[%d]: %d\n", i, data->additional_data.ues_thp[i].ul);
  }

  log("\n");
}

void oai_data_free(oai_data_t *data)
{
  if (strcmp(nodeType, "du") == 0 || strcmp(nodeType, "gNB") == 0) {
    free(data->nrcelldu.mcc);
    data->nrcelldu.mcc = 0;
    free(data->nrcelldu.mnc);
    data->nrcelldu.mnc = 0;

    free(data->device_data.gnbName);
    data->device_data.gnbName = 0;
    free(data->device_data.vendor);
    data->device_data.vendor = 0;

    free(data->additional_data.frameType);
    data->additional_data.frameType = 0;

    free(data->additional_data.ues);
    data->additional_data.ues = 0;

    free(data->additional_data.ues_thp);
    data->additional_data.ues_thp = 0;

    free(data);
  }
  if (strcmp(nodeType, "cu-cp") == 0 || strcmp(nodeType, "gNB") == 0) {
    free(data->nrcellcu.mcc);
    data->nrcellcu.mcc = 0;
    free(data->nrcellcu.mnc);
    data->nrcellcu.mnc = 0;

    free(data);
  }
  if (strcmp(nodeType, "cu-up") == 0) {
    free(data->gnbcuup.mcc);
    data->gnbcuup.mcc = 0;
    free(data->gnbcuup.mnc);
    data->gnbcuup.mnc = 0;

    free(data);
  }
}