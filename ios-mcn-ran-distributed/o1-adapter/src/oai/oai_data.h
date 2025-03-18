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

#pragma once

#include <stdint.h>

typedef struct oai_nrsectcarr_data {
  char *txDirection;
  int configuredMaxTxPower;
  int configuredMaxTxEIRP;
  int arfcnDL;
  int arfcnUL;
  int bSChannelBwDL;
  int bSChannelBwUL;
  char *sectorEquipmentFunctionRef;
} oai_nrsectcarr_data_t;

typedef struct oai_ues_thp {
  int rnti;
  int dl;
  int ul;
} oai_ues_thp_t;

typedef struct oai_additional_data {
  char *frameType;
  int bandNumber;
  int numUes;
  int *ues;
  int load;
  oai_ues_thp_t *ues_thp;
  int num_dus;
  int num_cuups;
  char vendor;
} oai_additional_data_t;

typedef struct oai_config_data {
  int gnb_DU_Id;
  char *gnbCUName;
  int gnbId;
  int gnbIdLength;
  char *mcc;
  char *mnc;
  char *gnbName;
  char *vendor;
} oai_device_data_t;

typedef struct oai_bwp_data {
  int isInitialBwp;
  int numberOfRBs;
  int startRB;
  int subCarrierSpacing;
} oai_bwp_data_t;

typedef struct oai_nrcelldu_data {
  int ssbFrequency;
  int arfcnDL;
  int bSChannelBwDL;
  int arfcnUL;
  int bSChannelBwUL;
  int nRPCI;
  int nRTAC;
  char *mcc;
  char *mnc;
  uint32_t sd;
  uint32_t sst;
  uint32_t ssb_period;
  uint32_t ssb_offset;
} oai_nrcelldu_data_t;

typedef struct array_carrier {
  uint32_t arfcn_center;
  uint32_t center_channel_bw;
  uint32_t channel_bw;
  char ru_carrier[10];
  char rw_duplex_scheme[10];
  char rw_type[10];
  uint8_t dl_radio_frame_offset;
  uint8_t dl_sfn_offset;
  uint8_t gain_correction;
  uint8_t gain;
  uint8_t nta_offset;
} array_carrier_t;

typedef struct delay_management {
  uint32_t T2a_min_up;
  uint32_t T2a_max_up;
  uint32_t T2a_min_cp_dl;
  uint32_t T2a_max_cp_dl;
  uint32_t Tcp_adv_dl;
  uint32_t Ta3_min;
  uint32_t Ta3_max;
  uint32_t T2a_min_cp_ul;
  uint32_t T2a_max_cp_ul;
} delay_management_t;

typedef struct oai_oru_data {
  array_carrier_t tx_array_carrier;
  array_carrier_t rx_array_carrier;
  delay_management_t delay_management;
} oai_oru_data_t;

typedef struct oai_nrsectorcarrier {
  int arfcnDL;
  int bSChannelBwDL;
  int arfcnUL;
  int bSChannelBwUL;
} oai_nrsectorcarrier_t;

typedef struct oai_nrcellcu_data {
  uint32_t pMax;
  int cellLocalId;
  char *mcc;
  char *mnc;
  uint32_t sd;
  uint32_t sst;
  uint32_t tac;
  uint32_t mnc_digit_length;
} oai_nrcellcu_data_t;

typedef struct oai_nrfreqrel_data {
  uint8_t tReselectionNRSfHigh;
  uint32_t tReselectionNR;
  uint8_t tReselectionNRSfMedium;
  uint32_t pMax;
  char *nRFrequencyRef;
  uint8_t cellReselectionSubPriority;
  uint32_t cellReselectionPriority;
  uint8_t qOffsetFreq;
  uint32_t threshXLowP;
  uint32_t threshXLowQ;
  uint32_t threshXHighQ;
  uint32_t threshXHighP;
  int32_t qQualMin;
  int32_t qRxLevMin;
} oai_nrfreqrel_data_t;

typedef struct oai_gnbcuup_data {
  uint64_t gNBCUUPId;
  uint32_t gNBId;
  char *mcc;
  char *mnc;
  int mnc_length;
} oai_gnbcuup_data_t;

typedef struct oai_data {
  oai_bwp_data_t bwp[2];
  oai_nrcelldu_data_t nrcelldu;
  oai_device_data_t device_data;
  oai_additional_data_t additional_data;
  oai_nrsectorcarrier_t nrSectorCarrier;
  oai_nrcellcu_data_t nrcellcu;
  oai_nrfreqrel_data_t nrfreqrel;
  oai_nrsectcarr_data_t nrsectcarr;
  oai_gnbcuup_data_t gnbcuup;
  oai_oru_data_t oru;
} oai_data_t;

oai_data_t *oai_data_parse_json(const char *json);
void oai_data_print(const oai_data_t *data);
oai_data_t *oai_data_parse_json(const char *json);
void oai_data_free(oai_data_t *data);
