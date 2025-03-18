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

#include <sys/types.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <libconfig.h>

#define TELNETSERVERCODE
#include "telnetsrv.h"

#include "openair2/RRC/NR/nr_rrc_defs.h"
#include "openair2/LAYER2/NR_MAC_gNB/nr_mac_gNB.h"
#include "openair2/RRC/NR/nr_rrc_config.h"
#include "openair2/LAYER2/NR_MAC_gNB/mac_proto.h"
#include "openair2/LAYER2/nr_rlc/nr_rlc_oai_api.h"
#include "common/utils/nr/nr_common.h"
#include "cmake_targets/ran_build/build/openair2/RRC/NR/MESSAGES/NR_ServingCellConfigCommon.h"
#include "openair1/PHY/defs_nr_UE.h"
#include "openair2/LAYER2/NR_MAC_UE/mac_defs.h"
#include "openair2/GNB_APP/gnb_config.h"
#include "common/ngran_types.h"
#include "openair2/GNB_APP/RRC_nr_paramsvalues.h"
#include "cmake_targets/ran_build/build/openair2/RRC/NR/MESSAGES/NR_InterFreqCarrierFreqInfo.h"
#include "openair3/NGAP/ngap_gNB_defs.h"
#include "common/ngran_types.h"
#include "openair2/E1AP/e1ap_common.h"

extern mplane_enable_global;
#define OAI_MPLANE

#define ERROR_MSG_RET(mSG, aRGS...) do { prnt("FAILURE: " mSG, ##aRGS); return 1; } while (0)

#define ISINITBWP "bwp3gpp:isInitialBwp"
//#define CYCLPREF  "bwp3gpp:cyclicPrefix"
#define NUMRBS    "bwp3gpp:numberOfRBs"
#define STARTRB   "bwp3gpp:startRB"
#define BWPSCS    "bwp3gpp:subCarrierSpacing"

#define SSBFREQ "nrcelldu3gpp:ssbFrequency"
#define ARFCNDL "nrcelldu3gpp:arfcnDL"
#define BWDL    "nrcelldu3gpp:bSChannelBwDL"
#define ARFCNUL "nrcelldu3gpp:arfcnUL"
#define BWUL    "nrcelldu3gpp:bSChannelBwUL"
#define PCI     "nrcelldu3gpp:nRPCI"
#define TAC     "nrcelldu3gpp:nRTAC"
#define MCC     "nrcelldu3gpp:mcc"
#define MNC     "nrcelldu3gpp:mnc"
#define SD      "nrcelldu3gpp:sd"
#define SST     "nrcelldu3gpp:sst"
#define SSB_PERIOD     "nrcelldu3gpp:ssb_period"
#define SSB_OFFSET     "nrcelldu3gpp:ssb_offset"

#define gNBId             "nrgnbcuup3gpp:gNBId"
#define CU_UP_ID          "nrgnbcuup3gpp:cu_up_id"
#define CUUP_MCC          "nrgnbcuup3gpp:mcc"
#define CUUP_MNC          "nrgnbcuup3gpp:mnc"
#define MNC_DIGIT_LENGTH  "nrgnbcuup3gpp:mnc_length"

#define PMAX "nrfreqrel3gpp:pMax"
#define CELLLOCALID "nrcellcu3gpp:cellLocalId"
#define CU_MCC    "nrcellcu3gpp:mcc"
#define CU_MNC "nrcellcu3gpp:mnc"
#define CU_SST    "nrcellcu3gpp:sst"
#define CU_SD "nrcellcu3gpp:sd"
#define NR_FREQ_REF "nrcellcu3gpp:nRFrequencyRef"
int flag = 1;
int node_id = 0;
int pMax = 0;
char nRFrequencyRef[50];

typedef struct b {
  long int dl;
  long int ul;
} b_t;

static int get_stats(char *buf, int debug, telnet_printfunc_t prnt)
{
  if (buf)
    ERROR_MSG_RET("no parameter allowed\n");

  static ngran_node_t node_type;
  node_type = node_type?node_type:get_node_type();

  if(node_type == ngran_gNB_CUUP){
    const instance_t e1_inst = 0;
    const e1ap_upcp_inst_t *e1inst = getCxtE1(e1_inst);

    prnt("{\n");
    prnt("  \"o1-config\": {\n");
    prnt("     \"GNBCUUP\": {\n");
    prnt("      \""gNBId"\": %d,\n", e1inst->gnb_id);
    prnt("      \""CU_UP_ID"\": %d,\n", e1inst->cuup.setupReq.gNB_cu_up_id);
    prnt("      \""CUUP_MCC"\": \"%03d\",\n", e1inst->cuup.setupReq.plmn[0].id.mcc);
    prnt("      \""CUUP_MNC"\": \"%02d\",\n", e1inst->cuup.setupReq.plmn[0].id.mnc);
    prnt("      \""MNC_DIGIT_LENGTH"\": %d\n", e1inst->cuup.setupReq.plmn[0].id.mnc_digit_length);
    prnt("  }\n");
    prnt(" }\n");
    prnt("}\n");

  }
 if(node_type == ngran_gNB_CUCP || node_type == ngran_gNB_DU || node_type == ngran_gNB){
   const gNB_RRC_INST *rrc = RC.nrrrc[0];
   const gNB_RrcConfigurationReq cu = rrc->configuration;

   if(node_type == ngran_gNB_CUCP){
    if(flag == 1){
    node_id = rrc->node_id;
   // strcpy(nRFrequencyRef, "ManagedElement=oai");
    strcpy(nRFrequencyRef, "AName=JohnDoe");
    flag = 0;
    }
    prnt("{\n");
    prnt("  \"o1-config\": {\n");
    prnt("     \"NRFREQREL\": {\n");
    prnt("      \""PMAX"\": %d\n", pMax);
    prnt("     },\n");
    prnt("     \"NRCELLCU\": {\n");
    prnt("      \""CELLLOCALID"\": %d,\n", node_id);
    prnt("      \""CU_MCC"\": \"%03d\",\n", cu.mcc[0]);
    prnt("      \""CU_MNC"\": \"%02d\",\n", cu.mnc[0]);
    //prnt("      \""CU_SST"\": %d,\n",nssai->sst);
    //prnt("      \""CU_SD"\": %d,\n",nssai->sd);
    prnt("      \""CU_SST"\": %d,\n", 1);
    prnt("      \""CU_SD"\": %d,\n", 0);
    prnt("      \""NR_FREQ_REF"\": \"%s\"\n", nRFrequencyRef);
    prnt("     },\n");
    prnt("    \"device\": {\n");
    prnt("      \"gNBId\": %d,\n",node_id);
    prnt("      \"gnbCUName\": \"%s\"\n", rrc->node_name);
    prnt("    }\n");
    prnt("  },\n");
    prnt("  \"O1-Operational\": {\n");
    prnt("    \"NUM_DUS\": %d,\n",rrc->num_dus);
    prnt("    \"NUM_CUUPS\": %d,\n",rrc->num_cuups);
    prnt("    \"vendor\": \"OpenAirInterface\"\n");
    prnt("  }\n");
    prnt("}\n");

  }

  if(node_type == ngran_gNB_DU || node_type == ngran_gNB){
  const gNB_MAC_INST *mac = RC.nrmac[0];
  AssertFatal(mac != NULL, "need MAC\n");

  const f1ap_setup_req_t *sr = mac->f1_config.setup_req;
  const f1ap_served_cell_info_t *cell_info = &sr->cell[0].info;

  const NR_ServingCellConfigCommon_t *scc = mac->common_channels[0].ServingCellConfigCommon;
  const NR_FrequencyInfoDL_t *frequencyInfoDL = scc->downlinkConfigCommon->frequencyInfoDL;
  const NR_FrequencyInfoUL_t *frequencyInfoUL = scc->uplinkConfigCommon->frequencyInfoUL;
  frame_type_t frame_type = get_frame_type(*frequencyInfoDL->frequencyBandList.list.array[0], *scc->ssbSubcarrierSpacing);
  const NR_BWP_t *initialDL = &scc->downlinkConfigCommon->initialDownlinkBWP->genericParameters;
  const NR_BWP_t *initialUL = &scc->uplinkConfigCommon->initialUplinkBWP->genericParameters;

  int scs = initialDL->subcarrierSpacing;
  AssertFatal(scs == initialUL->subcarrierSpacing, "different SCS for UL/DL not supported!\n");
  int band = *frequencyInfoDL->frequencyBandList.list.array[0];
  frequency_range_t freq_range = *frequencyInfoDL->frequencyBandList.list.array[0] > 256 ? FR2 : FR1;
  int nrb = frequencyInfoDL->scs_SpecificCarrierList.list.array[0]->carrierBandwidth;
  AssertFatal(nrb == frequencyInfoUL->scs_SpecificCarrierList.list.array[0]->carrierBandwidth, "different BW for UL/DL not supported!\n");
  int bw_index = get_supported_band_index(scs, freq_range, nrb);
  int bw_mhz = get_supported_bw_mhz(band > 256 ? FR2 : FR1, bw_index);

  int num_ues = 0;
  UE_iterator((NR_UE_info_t **)mac->UE_info.list, it) {
    num_ues++;
  }

//  const NR_mac_stats_t mac_stats;
  const mac_stats_t *stat = &mac->mac_stats;
  static mac_stats_t last = {0};
  int diff_used = stat->used_prb_aggregate - last.used_prb_aggregate;
  int diff_total = stat->total_prb_aggregate - last.total_prb_aggregate;
  int load = diff_total > 0 ? 100 * diff_used / diff_total : 0;
  last = *stat;

  static struct timespec tp_last = {0};
  struct timespec tp_now;
  clock_gettime(CLOCK_MONOTONIC, &tp_now);
  size_t diff_msec = (tp_now.tv_sec - tp_last.tv_sec) * 1000 + (tp_now.tv_nsec - tp_last.tv_nsec) / 1000000;
  tp_last = tp_now;

  const int srb_flag = 0;
  const int rb_id = 1;
  static b_t last_total[MAX_MOBILES_PER_GNB] = {0};
  b_t thr[MAX_MOBILES_PER_GNB] = {0};
  int i = 0;
  {
    UE_iterator((NR_UE_info_t **)mac->UE_info.list, it) {
      nr_rlc_statistics_t rlc = {0};
      nr_rlc_get_statistics(it->rnti, srb_flag, rb_id, &rlc);
      // static var last_total: we might have old data, larger than what
      // reports RLC, leading to a huge number -> cut off to zero
      if (last_total[i].dl > rlc.txpdu_bytes)
        last_total[i].dl = rlc.txpdu_bytes;
      if (last_total[i].ul > rlc.rxpdu_bytes)
        last_total[i].ul = rlc.rxpdu_bytes;
      thr[i].dl = (rlc.txpdu_bytes - last_total[i].dl) * 8 / diff_msec;
      thr[i].ul = (rlc.rxpdu_bytes - last_total[i].ul) * 8 / diff_msec;
      last_total[i].dl = rlc.txpdu_bytes;
      last_total[i].ul = rlc.rxpdu_bytes;
      i++;
    }
  }

  prnt("{\n");
    prnt("  \"o1-config\": {\n");
    if(node_type == ngran_gNB_CUCP || node_type == ngran_gNB){
    prnt("     \"NRFREQREL\": {\n");
    prnt("      \""PMAX"\": %d\n", pMax);
    prnt("     },\n");
    prnt("     \"NRCELLCU\": {\n");
    prnt("      \""CELLLOCALID"\": %d,\n", node_id);
    prnt("      \""CU_MCC"\": \"%03d\",\n", cu.mcc[0]);
    prnt("      \""CU_MNC"\": \"%02d\",\n", cu.mnc[0]);
    //prnt("      \""CU_SST"\": %d,\n",nssai->sst);
    //prnt("      \""CU_SD"\": %d,\n",nssai->sd);
    prnt("      \""CU_SST"\": %d,\n", 1);
    prnt("      \""CU_SD"\": %d,\n", 0);
    prnt("      \""NR_FREQ_REF"\": \"%s\"\n", nRFrequencyRef);
    prnt("     },\n");
    }
    if(node_type == ngran_gNB_DU || node_type == ngran_gNB){
    prnt("    \"BWP\": {\n");
    prnt("      \"dl\": [{\n");
    prnt("        \"" ISINITBWP "\": true,\n");
    //prnt("      \"" CYCLPREF "\": %ld,\n", *initialDL->cyclicPrefix);
    prnt("        \"" NUMRBS "\": %ld,\n", NRRIV2BW(initialDL->locationAndBandwidth, MAX_BWP_SIZE));
    prnt("        \"" STARTRB "\": %ld,\n", NRRIV2PRBOFFSET(initialDL->locationAndBandwidth, MAX_BWP_SIZE));
    prnt("        \"" BWPSCS "\": %ld\n", scs);
    prnt("      }],\n");
    prnt("      \"ul\": [{\n");
    prnt("        \"" ISINITBWP "\": true,\n");
    //prnt("      \"" CYCLPREF "\": %ld,\n", *initialUL->cyclicPrefix);
    prnt("        \"" NUMRBS "\": %ld,\n", NRRIV2BW(initialUL->locationAndBandwidth, MAX_BWP_SIZE));
    prnt("        \"" STARTRB "\": %ld,\n", NRRIV2PRBOFFSET(initialUL->locationAndBandwidth, MAX_BWP_SIZE));
    prnt("        \"" BWPSCS "\": %ld\n", scs);
    prnt("      }]\n");
    prnt("    },\n");

    prnt("    \"NRCELLDU\": {\n");
    prnt("      \"" SSBFREQ "\": %ld,\n", *scc->downlinkConfigCommon->frequencyInfoDL->absoluteFrequencySSB);
    prnt("      \"" ARFCNDL "\": %ld,\n", frequencyInfoDL->absoluteFrequencyPointA);
    prnt("      \"" BWDL "\": %ld,\n", bw_mhz);
    prnt("      \"" ARFCNUL "\": %ld,\n", frequencyInfoUL->absoluteFrequencyPointA ? *frequencyInfoUL->absoluteFrequencyPointA : frequencyInfoDL->absoluteFrequencyPointA);
    prnt("      \"" BWUL "\": %ld,\n", bw_mhz);
    prnt("      \"" PCI "\": %ld,\n", *scc->physCellId);
    prnt("      \"" TAC "\": %ld,\n", *cell_info->tac);
    prnt("      \"" MCC "\": \"%03d\",\n", cell_info->plmn.mcc);
    prnt("      \"" MNC "\": \"%0*d\",\n", cell_info->plmn.mnc_digit_length, cell_info->plmn.mnc);
    prnt("      \"" SD  "\": %d,\n", cell_info->nssai->sd);
    prnt("      \"" SST "\": %d\n", cell_info->nssai->sst);
//    prnt("      \""SSB_PERIOD "\": %d,\n", *scc->ssb_periodicityServingCell);
//    prnt("      \""SSB_OFFSET "\": %d\n", mac->ssb_SubcarrierOffset);

    //prnt("      \"" TAC "\": %ld,\n", conf->tac);
   // prnt("      \"" MCC "\": \"%03d\",\n", conf->mcc[0]);
   // prnt("      \"" MNC "\": \"%0*d\",\n", conf->mnc_digit_length[0], conf->mnc[0]);
   // prnt("      \"" SD  "\": %d,\n", conf->sd);
   // prnt("      \"" SST "\": %d\n", conf->sst);
    prnt("    },\n");
    if(mplane_enable_global){
#ifdef OAI_MPLANE

    oai_oru_data_t *oru = &RC.ru[0]->openair0_cfg.split7.oru;

    prnt("  \"o-ru-stats\": {\n");
    prnt("    \"o-ran-uplane-conf\": [\n");
    prnt("      {\n");
    prnt("        \"tx-array-carrier:absolute-frequency-center\": %d,\n", oru->tx_array_carrier.arfcn_center);
    prnt("        \"tx-array-carrier:center-of-channel-bandwidth\": %ld,\n", oru->tx_array_carrier.center_channel_bw);
    prnt("        \"tx-array-carrier:channel-bandwidth\": %d,\n", oru->tx_array_carrier.channel_bw);
    prnt("        \"tx-array-carrier:active\": \"%s\",\n", oru->tx_array_carrier.ru_carrier);
    prnt("        \"tx-array-carrier:rw-duplex-scheme\": \"%s\",\n", oru->tx_array_carrier.rw_duplex_scheme);
    prnt("        \"tx-array-carrier:rw-type\": \"%s\",\n", oru->tx_array_carrier.rw_type);
    prnt("        \"tx-array-carrier:gain\": %.2f,\n", oru->tx_array_carrier.gain);
    prnt("        \"tx-array-carrier:downlink-radio-frame-offset\": %d,\n", oru->tx_array_carrier.dl_radio_frame_offset);
    prnt("        \"tx-array-carrier:downlink-sfn-offset\": %d\n", oru->tx_array_carrier.dl_sfn_offset);
    prnt("      },\n");
    prnt("      {\n");
    prnt("        \"rx-array-carrier:absolute-frequency-center\": %d,\n", oru->tx_array_carrier.arfcn_center);
    prnt("        \"rx-array-carrier:center-of-channel-bandwidth\": %ld,\n", oru->tx_array_carrier.center_channel_bw);
    prnt("        \"rx-array-carrier:channel-bandwidth\": %d,\n", oru->tx_array_carrier.channel_bw);
    prnt("        \"rx-array-carrier:active\": \"%s\",\n", oru->tx_array_carrier.ru_carrier);
    prnt("        \"rx-array-carrier:downlink-radio-frame-offset\": %d,\n", oru->tx_array_carrier.dl_radio_frame_offset);
    prnt("        \"rx-array-carrier:downlink-sfn-offset\": %d,\n", oru->tx_array_carrier.dl_sfn_offset);
    prnt("        \"rx-array-carrier:gain-correction\": %.2f,\n", oru->tx_array_carrier.gain_correction);
    prnt("        \"rx-array-carrier:n-ta-offset\": %d\n", oru->tx_array_carrier.n_ta_offset);
    prnt("      }\n");
    prnt("    ],\n");
    prnt("    \"delay-management\": {\n");
    prnt("      \"ru-delay-profile:t2a-min-up\": %d,\n", oru->delay_management.T2a_min_up);
    prnt("      \"ru-delay-profile:t2a-max-up\": %d,\n", oru->delay_management.T2a_max_up);
    prnt("      \"ru-delay-profile:t2a-min-cp-dl\": %d,\n", oru->delay_management.T2a_min_cp_dl);
    prnt("      \"ru-delay-profile:t2a-max-cp-dl\": %d,\n", oru->delay_management.T2a_max_cp_dl);
    prnt("      \"ru-delay-profile:tcp-adv-dl\": %d,\n", oru->delay_management.Tcp_adv_dl);
    prnt("      \"ru-delay-profile:ta3-min\": %d,\n", oru->delay_management.Ta3_min);
    prnt("      \"ru-delay-profile:ta3-max\": %d,\n", oru->delay_management.Ta3_max);
    prnt("      \"ru-delay-profile:t2a-min-cp-ul\": %d,\n", oru->delay_management.T2a_min_cp_ul);
    prnt("      \"ru-delay-profile:t2a-max-cp-ul\": %d\n", oru->delay_management.T2a_max_cp_ul);
    prnt("    }\n");
    prnt("  },\n");
#else
    Assertfatal(1==0,"Rebuild the code with OAI_MPLANE or run without --mplane");
#endif
    }

    prnt("    \"device\": {\n");
    prnt("      \"gnb_DU_Id\": %d,\n", sr->gNB_DU_id);
    prnt("      \"gNBId\": %d,\n",node_id);
    prnt("      \"gnbName\": \"%s\",\n", sr->gNB_DU_name);
    }
    if(node_type == ngran_gNB_CUCP || node_type == ngran_gNB){
    prnt("      \"gnbCUName\": \"%s\",\n", rrc->node_name);
   // prnt("      \"gnbId\": %d,\n", conf->cell_identity);
   // prnt("      \"gnbName\": \"%s\",\n", rrc->node_name);
    }
    if(node_type == ngran_gNB_DU || node_type == ngran_gNB){
    prnt("      \"vendor\": \"OpenAirInterface\"\n");
    prnt("    }\n");
    prnt("  },\n");

    prnt("  \"O1-Operational\": {\n");
    prnt("    \"frame-type\": \"%s\",\n", frame_type == TDD ? "tdd" : "fdd");
    prnt("    \"band-number\": %ld,\n", band);
    }
    if(node_type == ngran_gNB_CUCP || node_type == ngran_gNB){
    prnt("    \"NUM_DUS\": %d,\n",rrc->num_dus);
    prnt("    \"NUM_CUUPS\": %d,\n",rrc->num_cuups);
    }
    prnt("    \"num-ues\": %d,\n", num_ues);
    prnt("    \"ues\": [");
    {
      bool first = true;
      UE_iterator((NR_UE_info_t **)mac->UE_info.list, it) {
        if (!first) { prnt(", "); }
        prnt("%d", it->rnti);
        first = false;
      }
    }
    prnt("    ],\n");
    prnt("    \"load\": %d\n", load);
    prnt("  }\n");
  prnt("}\n");
  prnt("OK\n");
 }
}
  return 0;
}

static int read_long(const char *buf, const char *end, const char *id, long *val)
{
  const char *curr = buf;
  while (isspace(*curr) && curr < end) // skip leading spaces
    curr++;
  int len = strlen(id);
  if (curr + len >= end)
    return -1;
  if (strncmp(curr, id, len) != 0) // check buf has id
    return -1;
  curr += len;
  while (isspace(*curr) && curr < end) // skip middle spaces
    curr++;
  if (curr >= end)
    return -1;
  int nread = sscanf(curr, "%ld", val);
  if (nread != 1)
    return -1;
  while (isdigit(*curr) && curr < end) // skip all digits read above
    curr++;
  if (curr > end)
    return -1;
  return curr - buf;
}

bool running = true; // in the beginning, the softmodem is started automatically
static int set_config(char *buf, int debug, telnet_printfunc_t prnt)
{
  if (!buf)
    ERROR_MSG_RET("need param: o1 config param1 val1 [param2 val2 ...]\n");
  if (running)
    ERROR_MSG_RET("cannot set parameters while L1 is running\n");
  const char *end = buf + strlen(buf);

  /* we need to update the following fields to change frequency and/or
   * bandwidth:
   * --gNBs.[0].servingCellConfigCommon.[0].absoluteFrequencySSB 620736            -> SSBFREQ
   * --gNBs.[0].servingCellConfigCommon.[0].dl_absoluteFrequencyPointA 620020      -> ARFCNDL
   * --gNBs.[0].servingCellConfigCommon.[0].dl_carrierBandwidth 51                 -> BWDL
   * --gNBs.[0].servingCellConfigCommon.[0].initialDLBWPlocationAndBandwidth 13750 -> NUMRBS + STARTRB
   * --gNBs.[0].servingCellConfigCommon.[0].ul_carrierBandwidth 51                 -> BWUL?
   * --gNBs.[0].servingCellConfigCommon.[0].initialULBWPlocationAndBandwidth 13750 -> ?
   */

  int processed = 0;
  int pos = 0;

  long ssbfreq;
  processed = read_long(buf + pos, end, SSBFREQ, &ssbfreq);
  if (processed < 0)
    ERROR_MSG_RET("could not read " SSBFREQ " at index %d\n", pos + processed);
  pos += processed;
  prnt("setting " SSBFREQ ":   %ld [len %d]\n", ssbfreq, pos);

  long arfcn;
  processed = read_long(buf + pos, end, ARFCNDL, &arfcn);
  if (processed < 0)
    ERROR_MSG_RET("could not read " ARFCNDL " at index %d\n", pos + processed);
  pos += processed;
  prnt("setting " ARFCNDL ":        %ld [len %d]\n", arfcn, pos);

  long bwdl;
  processed = read_long(buf + pos, end, BWDL, &bwdl);
  if (processed < 0)
    ERROR_MSG_RET("could not read " BWDL " at index %d\n", pos + processed);
  pos += processed;
  prnt("setting " BWDL ":  %ld [len %d]\n", bwdl, pos);

  long numrbs;
  processed = read_long(buf + pos, end, NUMRBS, &numrbs);
  if (processed < 0)
    ERROR_MSG_RET("could not read " NUMRBS " at index %d\n", pos + processed);
  pos += processed;
  prnt("setting " NUMRBS ":         %ld [len %d]\n", numrbs, pos);

  long startrb;
  processed = read_long(buf + pos, end, STARTRB, &startrb);
  if (processed < 0)
    ERROR_MSG_RET("could not read " STARTRB " at index %d\n", pos + processed);
  pos += processed;
  prnt("setting " STARTRB ":             %ld [len %d]\n", startrb, pos);

  int locationAndBandwidth = PRBalloc_to_locationandbandwidth0(numrbs, startrb, MAX_BWP_SIZE);
  prnt("inferred locationAndBandwidth:       %d\n", locationAndBandwidth);

  /*
  gNB_RRC_INST *rrc = RC.nrrrc[0];
  NR_ServingCellConfigCommon_t *scc = rrc->carrier.servingcellconfigcommon;
  NR_FrequencyInfoDL_t *frequencyInfoDL = scc->downlinkConfigCommon->frequencyInfoDL;
  NR_BWP_t *initialDL = &scc->downlinkConfigCommon->initialDownlinkBWP->genericParameters;
  NR_FrequencyInfoUL_t *frequencyInfoUL = scc->uplinkConfigCommon->frequencyInfoUL;
  NR_BWP_t *initialUL = &scc->uplinkConfigCommon->initialUplinkBWP->genericParameters;

  //--gNBs.[0].servingCellConfigCommon.[0].absoluteFrequencySSB 620736            -> SSBFREQ
  *scc->downlinkConfigCommon->frequencyInfoDL->absoluteFrequencySSB = ssbfreq;

  // --gNBs.[0].servingCellConfigCommon.[0].dl_absoluteFrequencyPointA 620020      -> ARFCNDL
  frequencyInfoDL->absoluteFrequencyPointA = arfcn;
  AssertFatal(frequencyInfoUL->absoluteFrequencyPointA == NULL, "only handle TDD\n");

  // --gNBs.[0].servingCellConfigCommon.[0].dl_carrierBandwidth 51                 -> BWDL
  frequencyInfoDL->scs_SpecificCarrierList.list.array[0]->carrierBandwidth = bwdl;

  // --gNBs.[0].servingCellConfigCommon.[0].initialDLBWPlocationAndBandwidth 13750 -> NUMRBS + STARTRB
  initialDL->locationAndBandwidth = locationAndBandwidth;

  // --gNBs.[0].servingCellConfigCommon.[0].ul_carrierBandwidth 51                 -> BWUL?
  // we assume the same BW as DL
  frequencyInfoUL->scs_SpecificCarrierList.list.array[0]->carrierBandwidth = bwdl;

  // --gNBs.[0].servingCellConfigCommon.[0].initialULBWPlocationAndBandwidth 13750 -> ?
  // we assume same locationAndBandwidth as DL
  initialUL->locationAndBandwidth = locationAndBandwidth;
  */

  prnt("OK\n");
  return 0;
}

static int set_cu_config(char *buf, int debug, telnet_printfunc_t prnt)
{
  if (!buf)
    ERROR_MSG_RET("need param: o1 config param1 val1 [param2 val2 ...]\n");
  if (running)
    ERROR_MSG_RET("cannot set parameters while L1 is running\n");
  const char *end = buf + strlen(buf);
  int processed = 0;
  int pos = 0;
  long pMax;

  processed = read_long(buf + pos, end, PMAX, &pMax);
  if (processed < 0)
    ERROR_MSG_RET("could not read " PMAX " at index %d\n", pos + processed);
  pos += processed;

  const gNB_MAC_INST *mac = RC.nrmac[0];
  const NR_ServingCellConfigCommon_t *scc = mac->common_channels[0].ServingCellConfigCommon;
  *scc->uplinkConfigCommon->frequencyInfoUL->p_Max = pMax;

  prnt("OK\n");
  return 0;
}

//extern int8_t threequarter_fs;
//extern openair0_config_t openair0_cfg[MAX_CARDS];
static int set_bwconfig(char *buf, int debug, telnet_printfunc_t prnt)
{
  if (running)
    ERROR_MSG_RET("cannot set parameters while L1 is running\n");
  if (!buf)
    ERROR_MSG_RET("need param: o1 bwconfig <BW>\n");

  char *end = NULL;
  if (NULL != (end = strchr(buf, '\n')))
    *end = 0;
  if (NULL != (end = strchr(buf, '\r')))
    *end = 0;

  gNB_MAC_INST *mac = RC.nrmac[0];
  NR_ServingCellConfigCommon_t *scc = mac->common_channels[0].ServingCellConfigCommon;
  NR_FrequencyInfoDL_t *frequencyInfoDL = scc->downlinkConfigCommon->frequencyInfoDL;
  NR_BWP_t *initialDL = &scc->downlinkConfigCommon->initialDownlinkBWP->genericParameters;
  NR_FrequencyInfoUL_t *frequencyInfoUL = scc->uplinkConfigCommon->frequencyInfoUL;
  NR_BWP_t *initialUL = &scc->uplinkConfigCommon->initialUplinkBWP->genericParameters;
  if (strcmp(buf, "40") == 0) {
    *scc->downlinkConfigCommon->frequencyInfoDL->absoluteFrequencySSB = 641280;
    frequencyInfoDL->absoluteFrequencyPointA = 640008;
    AssertFatal(frequencyInfoUL->absoluteFrequencyPointA == NULL, "only handle TDD\n");
    frequencyInfoDL->scs_SpecificCarrierList.list.array[0]->carrierBandwidth = 106;
    initialDL->locationAndBandwidth = 28875;
    frequencyInfoUL->scs_SpecificCarrierList.list.array[0]->carrierBandwidth = 106;
    initialUL->locationAndBandwidth = 28875;
    //threequarter_fs = 1;
    //openair0_cfg[0].threequarter_fs = 1;
  } else if (strcmp(buf, "20") == 0) {
    *scc->downlinkConfigCommon->frequencyInfoDL->absoluteFrequencySSB = 641280;
    frequencyInfoDL->absoluteFrequencyPointA = 640596;
    AssertFatal(frequencyInfoUL->absoluteFrequencyPointA == NULL, "only handle TDD\n");
    frequencyInfoDL->scs_SpecificCarrierList.list.array[0]->carrierBandwidth = 51;
    initialDL->locationAndBandwidth = 13750;
    frequencyInfoUL->scs_SpecificCarrierList.list.array[0]->carrierBandwidth = 51;
    initialUL->locationAndBandwidth = 13750;
    //threequarter_fs = 0;
    //openair0_cfg[0].threequarter_fs = 0;
  }else if (strcmp(buf, "100") == 0) {
    *scc->downlinkConfigCommon->frequencyInfoDL->absoluteFrequencySSB = 646668;
    frequencyInfoDL->absoluteFrequencyPointA = 643392;
    AssertFatal(frequencyInfoUL->absoluteFrequencyPointA == NULL, "only handle TDD\n");
    frequencyInfoDL->scs_SpecificCarrierList.list.array[0]->carrierBandwidth = 273;
    initialDL->locationAndBandwidth = 1099;
    frequencyInfoUL->scs_SpecificCarrierList.list.array[0]->carrierBandwidth = 273;
    initialUL->locationAndBandwidth = 1099;
    //threequarter_fs = 0;
    //openair0_cfg[0].threequarter_fs = 0;
  }else if (strcmp(buf, "60") == 0) {
    *scc->downlinkConfigCommon->frequencyInfoDL->absoluteFrequencySSB = 621984;
    frequencyInfoDL->absoluteFrequencyPointA = 620040;
    AssertFatal(frequencyInfoUL->absoluteFrequencyPointA == NULL, "only handle TDD\n");
    frequencyInfoDL->scs_SpecificCarrierList.list.array[0]->carrierBandwidth = 162;
    initialDL->locationAndBandwidth = 31624;
    frequencyInfoUL->scs_SpecificCarrierList.list.array[0]->carrierBandwidth = 162;
    initialUL->locationAndBandwidth = 31624;
    //threequarter_fs = 0;
    //openair0_cfg[0].threequarter_fs = 0;
  }

  else {
    ERROR_MSG_RET("unhandled option %s\n", buf);
  }

  free(RC.nrmac[0]->sched_ctrlCommon);
  RC.nrmac[0]->sched_ctrlCommon = NULL;


  free_MIB_NR(mac->common_channels[0].mib);
  mac->common_channels[0].mib = get_new_MIB_NR(scc);

  // due to outrightly CRAZY memory handling in get_SIB1_NR(), we need to set
  // some structures to zero to prevent that we shoot ourselves into the foot
  //struct NR_SIB1 *xyz = rrc->carrier.siblock1->message.choice.c1->choice.systemInformationBlockType1;
  //xyz->servingCellConfigCommon = NULL;
  //free_SIB1_NR(rrc->carrier.siblock1);
  const f1ap_served_cell_info_t *info = &mac->f1_config.setup_req->cell[0].info;
  /*
  NR_BCCH_DL_SCH_Message_t *sib1 = get_SIB1_NR(scc, &info->plmn, into->nr_cellid, *info.tac);
  rrc->carrier.SIB1 = calloc(NR_MAX_SIB_LENGTH / 8, sizeof(*rrc->carrier.SIB1));
  AssertFatal(rrc->carrier.SIB1 != NULL, "out of memory\n");
  rrc->carrier.sizeof_SIB1 = encode_SIB1_NR(sib1, rrc->carrier.SIB1, NR_MAX_SIB_LENGTH / 8);
  rrc->carrier.siblock1 = sib1;
  */
  nr_mac_configure_sib1(mac, &info->plmn, info->nr_cellid, *info->tac);

  //nr_mac_config_scc(mac, scc, &mac->radio_config);

  prnt("OK\n");
  return 0;
}

/*
 * This is just a function for demonstration purposes, instead of the on-the-fly changes.
 * We change the parameters inside the configuration file, and then assert the gNB.
 * So when the gNB restarts/crashes, the confs from O1 will be kept.
 * We also need such functionality in a normal scenario.
 * 
 * To make it work 
 * 1.) just have a file named nr.conf in the same dir as nr-softmodem,
 * or change the nr.conf to match your conf file wherever it is used in the function below.
 * In the future we should get the name and path of the conf file automatically.
 * 
 * 2.) Change the {"bwconfig", "", set_bwconfig}, to {"bwconfig", "", set_bwconfig_demo},
 */
static int set_bwconfig_demo(char *buf, int debug, telnet_printfunc_t prnt)
{
  if (!buf)
    ERROR_MSG_RET("need param: o1 bwconfig <BW>\n");

  char *end = NULL;
  if (NULL != (end = strchr(buf, '\n')))
    *end = 0;
  if (NULL != (end = strchr(buf, '\r')))
    *end = 0;

  config_t cfg;
  config_init(&cfg);
  if (!config_read_file(&cfg, "nr.conf")) {
    fprintf(stderr, "Error reading configuration file: %s\n", config_error_text(&cfg));
    config_destroy(&cfg);
    return 1;
  }

  int absoluteFrequencySSB;
  int dl_absoluteFrequencyPointA;
  int dl_carrierBandwidth;
  int initialDLBWPlocationAndBandwidth;
  int ul_carrierBandwidth;
  int initialULBWPlocationAndBandwidth;

  if (strcmp(buf, "40") == 0) {
    if (config_lookup_int(&cfg, "gNBs.[0].servingCellConfigCommon.[0].absoluteFrequencySSB", &absoluteFrequencySSB)) {
      printf("absoluteFrequencySSB: %d\n", absoluteFrequencySSB);
      absoluteFrequencySSB = 641280;
      config_setting_t *setting = config_lookup(&cfg, "gNBs.[0].servingCellConfigCommon.[0].absoluteFrequencySSB");
      config_setting_set_int(setting, absoluteFrequencySSB);
      if (config_write_file(&cfg, "nr.conf")) {
        printf("Updated absoluteFrequencySSB: %d\n", absoluteFrequencySSB);
      } else {
        fprintf(stderr, "Error writing configuration file.\n");
      }
    } else {
      fprintf(stderr, "Error: Could not read absoluteFrequencySSB from the configuration file.\n");
    }

    if (config_lookup_int(&cfg, "gNBs.[0].servingCellConfigCommon.[0].dl_absoluteFrequencyPointA", &dl_absoluteFrequencyPointA)) {
      printf("dl_absoluteFrequencyPointA: %d\n", dl_absoluteFrequencyPointA);
      dl_absoluteFrequencyPointA = 640008;
      config_setting_t *setting = config_lookup(&cfg, "gNBs.[0].servingCellConfigCommon.[0].dl_absoluteFrequencyPointA");
      config_setting_set_int(setting, dl_absoluteFrequencyPointA);
      if (config_write_file(&cfg, "nr.conf")) {
        printf("Updated dl_absoluteFrequencyPointA: %d\n", dl_absoluteFrequencyPointA);
      } else {
        fprintf(stderr, "Error writing configuration file.\n");
      }
    } else {
      fprintf(stderr, "Error: Could not read dl_absoluteFrequencyPointA from the configuration file.\n");
    }

    if (config_lookup_int(&cfg, "gNBs.[0].servingCellConfigCommon.[0].dl_carrierBandwidth", &dl_carrierBandwidth)) {
      printf("dl_carrierBandwidth: %d\n", dl_carrierBandwidth);
      dl_carrierBandwidth = 106;
      config_setting_t *setting = config_lookup(&cfg, "gNBs.[0].servingCellConfigCommon.[0].dl_carrierBandwidth");
      config_setting_set_int(setting, dl_carrierBandwidth);
      if (config_write_file(&cfg, "nr.conf")) {
        printf("Updated dl_carrierBandwidth: %d\n", dl_carrierBandwidth);
      } else {
        fprintf(stderr, "Error writing configuration file.\n");
      }
    } else {
      fprintf(stderr, "Error: Could not read dl_carrierBandwidth from the configuration file.\n");
    }

    if (config_lookup_int(&cfg,
                          "gNBs.[0].servingCellConfigCommon.[0].initialDLBWPlocationAndBandwidth",
                          &initialDLBWPlocationAndBandwidth)) {
      printf("initialDLBWPlocationAndBandwidth: %d\n", initialDLBWPlocationAndBandwidth);
      initialDLBWPlocationAndBandwidth = 28875;
      config_setting_t *setting = config_lookup(&cfg, "gNBs.[0].servingCellConfigCommon.[0].initialDLBWPlocationAndBandwidth");
      config_setting_set_int(setting, initialDLBWPlocationAndBandwidth);
      if (config_write_file(&cfg, "nr.conf")) {
        printf("Updated initialDLBWPlocationAndBandwidth: %d\n", initialDLBWPlocationAndBandwidth);
      } else {
        fprintf(stderr, "Error writing configuration file.\n");
      }
    } else {
      fprintf(stderr, "Error: Could not read initialDLBWPlocationAndBandwidth from the configuration file.\n");
    }

    if (config_lookup_int(&cfg, "gNBs.[0].servingCellConfigCommon.[0].ul_carrierBandwidth", &ul_carrierBandwidth)) {
      printf("ul_carrierBandwidth: %d\n", ul_carrierBandwidth);
      ul_carrierBandwidth = 106;
      config_setting_t *setting = config_lookup(&cfg, "gNBs.[0].servingCellConfigCommon.[0].ul_carrierBandwidth");
      config_setting_set_int(setting, ul_carrierBandwidth);
      if (config_write_file(&cfg, "nr.conf")) {
        printf("Updated ul_carrierBandwidth: %d\n", ul_carrierBandwidth);
      } else {
        fprintf(stderr, "Error writing configuration file.\n");
      }
    } else {
      fprintf(stderr, "Error: Could not read ul_carrierBandwidth from the configuration file.\n");
    }

    if (config_lookup_int(&cfg,
                          "gNBs.[0].servingCellConfigCommon.[0].initialULBWPlocationAndBandwidth",
                          &initialULBWPlocationAndBandwidth)) {
      printf("initialULBWPlocationAndBandwidth: %d\n", initialULBWPlocationAndBandwidth);
      initialULBWPlocationAndBandwidth = 28875;
      config_setting_t *setting = config_lookup(&cfg, "gNBs.[0].servingCellConfigCommon.[0].initialULBWPlocationAndBandwidth");
      config_setting_set_int(setting, initialULBWPlocationAndBandwidth);
      if (config_write_file(&cfg, "nr.conf")) {
        printf("Updated initialULBWPlocationAndBandwidth: %d\n", initialULBWPlocationAndBandwidth);
      } else {
        fprintf(stderr, "Error writing configuration file.\n");
      }
    } else {
      fprintf(stderr, "Error: Could not read initialULBWPlocationAndBandwidth from the configuration file.\n");
    }

    config_destroy(&cfg);
    AssertFatal(false, "Restarting...\n");

  } else if (strcmp(buf, "20") == 0) {
    if (config_lookup_int(&cfg, "gNBs.[0].servingCellConfigCommon.[0].absoluteFrequencySSB", &absoluteFrequencySSB)) {
      printf("absoluteFrequencySSB: %d\n", absoluteFrequencySSB);
      absoluteFrequencySSB = 620736;
      config_setting_t *setting = config_lookup(&cfg, "gNBs.[0].servingCellConfigCommon.[0].absoluteFrequencySSB");
      config_setting_set_int(setting, absoluteFrequencySSB);
      if (config_write_file(&cfg, "nr.conf")) {
        printf("Updated absoluteFrequencySSB: %d\n", absoluteFrequencySSB);
      } else {
        fprintf(stderr, "Error writing configuration file.\n");
      }
    } else {
      fprintf(stderr, "Error: Could not read absoluteFrequencySSB from the configuration file.\n");
    }

    if (config_lookup_int(&cfg, "gNBs.[0].servingCellConfigCommon.[0].dl_absoluteFrequencyPointA", &dl_absoluteFrequencyPointA)) {
      printf("dl_absoluteFrequencyPointA: %d\n", dl_absoluteFrequencyPointA);
      dl_absoluteFrequencyPointA = 620020;
      config_setting_t *setting = config_lookup(&cfg, "gNBs.[0].servingCellConfigCommon.[0].dl_absoluteFrequencyPointA");
      config_setting_set_int(setting, dl_absoluteFrequencyPointA);
      if (config_write_file(&cfg, "nr.conf")) {
        printf("Updated dl_absoluteFrequencyPointA: %d\n", dl_absoluteFrequencyPointA);
      } else {
        fprintf(stderr, "Error writing configuration file.\n");
      }
    } else {
      fprintf(stderr, "Error: Could not read dl_absoluteFrequencyPointA from the configuration file.\n");
    }

    if (config_lookup_int(&cfg, "gNBs.[0].servingCellConfigCommon.[0].dl_carrierBandwidth", &dl_carrierBandwidth)) {
      printf("dl_carrierBandwidth: %d\n", dl_carrierBandwidth);
      dl_carrierBandwidth = 51;
      config_setting_t *setting = config_lookup(&cfg, "gNBs.[0].servingCellConfigCommon.[0].dl_carrierBandwidth");
      config_setting_set_int(setting, dl_carrierBandwidth);
      if (config_write_file(&cfg, "nr.conf")) {
        printf("Updated dl_carrierBandwidth: %d\n", dl_carrierBandwidth);
      } else {
        fprintf(stderr, "Error writing configuration file.\n");
      }
    } else {
      fprintf(stderr, "Error: Could not read dl_carrierBandwidth from the configuration file.\n");
    }

    if (config_lookup_int(&cfg,
                          "gNBs.[0].servingCellConfigCommon.[0].initialDLBWPlocationAndBandwidth",
                          &initialDLBWPlocationAndBandwidth)) {
      printf("initialDLBWPlocationAndBandwidth: %d\n", initialDLBWPlocationAndBandwidth);
      initialDLBWPlocationAndBandwidth = 13750;
      config_setting_t *setting = config_lookup(&cfg, "gNBs.[0].servingCellConfigCommon.[0].initialDLBWPlocationAndBandwidth");
      config_setting_set_int(setting, initialDLBWPlocationAndBandwidth);
      if (config_write_file(&cfg, "nr.conf")) {
        printf("Updated initialDLBWPlocationAndBandwidth: %d\n", initialDLBWPlocationAndBandwidth);
      } else {
        fprintf(stderr, "Error writing configuration file.\n");
      }
    } else {
      fprintf(stderr, "Error: Could not read initialDLBWPlocationAndBandwidth from the configuration file.\n");
    }

    if (config_lookup_int(&cfg, "gNBs.[0].servingCellConfigCommon.[0].ul_carrierBandwidth", &ul_carrierBandwidth)) {
      printf("ul_carrierBandwidth: %d\n", ul_carrierBandwidth);
      ul_carrierBandwidth = 51;
      config_setting_t *setting = config_lookup(&cfg, "gNBs.[0].servingCellConfigCommon.[0].ul_carrierBandwidth");
      config_setting_set_int(setting, ul_carrierBandwidth);
      if (config_write_file(&cfg, "nr.conf")) {
        printf("Updated ul_carrierBandwidth: %d\n", ul_carrierBandwidth);
      } else {
        fprintf(stderr, "Error writing configuration file.\n");
      }
    } else {
      fprintf(stderr, "Error: Could not read ul_carrierBandwidth from the configuration file.\n");
    }

    if (config_lookup_int(&cfg,
                          "gNBs.[0].servingCellConfigCommon.[0].initialULBWPlocationAndBandwidth",
                          &initialULBWPlocationAndBandwidth)) {
      printf("initialULBWPlocationAndBandwidth: %d\n", initialULBWPlocationAndBandwidth);
      initialULBWPlocationAndBandwidth = 13750;
      config_setting_t *setting = config_lookup(&cfg, "gNBs.[0].servingCellConfigCommon.[0].initialULBWPlocationAndBandwidth");
      config_setting_set_int(setting, initialULBWPlocationAndBandwidth);
      if (config_write_file(&cfg, "nr.conf")) {
        printf("Updated initialULBWPlocationAndBandwidth: %d\n", initialULBWPlocationAndBandwidth);
      } else {
        fprintf(stderr, "Error writing configuration file.\n");
      }
    } else {
      fprintf(stderr, "Error: Could not read initialULBWPlocationAndBandwidth from the configuration file.\n");
    }

    config_destroy(&cfg);
    AssertFatal(false, "Restarting...\n");

    }else if (strcmp(buf, "100") == 0) {
    if (config_lookup_int(&cfg, "gNBs.[0].servingCellConfigCommon.[0].absoluteFrequencySSB", &absoluteFrequencySSB)) {
      printf("absoluteFrequencySSB: %d\n", absoluteFrequencySSB);
      absoluteFrequencySSB = 646668;
      config_setting_t *setting = config_lookup(&cfg, "gNBs.[0].servingCellConfigCommon.[0].absoluteFrequencySSB");
      config_setting_set_int(setting, absoluteFrequencySSB);
      if (config_write_file(&cfg, "nr.conf")) {
        printf("Updated absoluteFrequencySSB: %d\n", absoluteFrequencySSB);
      } else {
        fprintf(stderr, "Error writing configuration file.\n");
      }
    } else {
      fprintf(stderr, "Error: Could not read absoluteFrequencySSB from the configuration file.\n");
    }

    if (config_lookup_int(&cfg, "gNBs.[0].servingCellConfigCommon.[0].dl_absoluteFrequencyPointA", &dl_absoluteFrequencyPointA)) {
      printf("dl_absoluteFrequencyPointA: %d\n", dl_absoluteFrequencyPointA);
      dl_absoluteFrequencyPointA = 643392;
      config_setting_t *setting = config_lookup(&cfg, "gNBs.[0].servingCellConfigCommon.[0].dl_absoluteFrequencyPointA");
      config_setting_set_int(setting, dl_absoluteFrequencyPointA);
      if (config_write_file(&cfg, "nr.conf")) {
        printf("Updated dl_absoluteFrequencyPointA: %d\n", dl_absoluteFrequencyPointA);
      } else {
        fprintf(stderr, "Error writing configuration file.\n");
      }
    } else {
      fprintf(stderr, "Error: Could not read dl_absoluteFrequencyPointA from the configuration file.\n");
    }

    if (config_lookup_int(&cfg, "gNBs.[0].servingCellConfigCommon.[0].dl_carrierBandwidth", &dl_carrierBandwidth)) {
      printf("dl_carrierBandwidth: %d\n", dl_carrierBandwidth);
      dl_carrierBandwidth = 273;
      config_setting_t *setting = config_lookup(&cfg, "gNBs.[0].servingCellConfigCommon.[0].dl_carrierBandwidth");
      config_setting_set_int(setting, dl_carrierBandwidth);
      if (config_write_file(&cfg, "nr.conf")) {
        printf("Updated dl_carrierBandwidth: %d\n", dl_carrierBandwidth);
      } else {
        fprintf(stderr, "Error writing configuration file.\n");
      }
    } else {
      fprintf(stderr, "Error: Could not read dl_carrierBandwidth from the configuration file.\n");
    }

    if (config_lookup_int(&cfg,
                          "gNBs.[0].servingCellConfigCommon.[0].initialDLBWPlocationAndBandwidth",
                          &initialDLBWPlocationAndBandwidth)) {
      printf("initialDLBWPlocationAndBandwidth: %d\n", initialDLBWPlocationAndBandwidth);
      initialDLBWPlocationAndBandwidth = 32086;
      config_setting_t *setting = config_lookup(&cfg, "gNBs.[0].servingCellConfigCommon.[0].initialDLBWPlocationAndBandwidth");
      config_setting_set_int(setting, initialDLBWPlocationAndBandwidth);
      if (config_write_file(&cfg, "nr.conf")) {
        printf("Updated initialDLBWPlocationAndBandwidth: %d\n", initialDLBWPlocationAndBandwidth);
} else {
        fprintf(stderr, "Error writing configuration file.\n");
      }
    } else {
      fprintf(stderr, "Error: Could not read initialDLBWPlocationAndBandwidth from the configuration file.\n");
    }

    if (config_lookup_int(&cfg, "gNBs.[0].servingCellConfigCommon.[0].ul_carrierBandwidth", &ul_carrierBandwidth)) {
      printf("ul_carrierBandwidth: %d\n", ul_carrierBandwidth);
      ul_carrierBandwidth = 273;
      config_setting_t *setting = config_lookup(&cfg, "gNBs.[0].servingCellConfigCommon.[0].ul_carrierBandwidth");
      config_setting_set_int(setting, ul_carrierBandwidth);
      if (config_write_file(&cfg, "nr.conf")) {
        printf("Updated ul_carrierBandwidth: %d\n", ul_carrierBandwidth);
      } else {
        fprintf(stderr, "Error writing configuration file.\n");
      }
    } else {
      fprintf(stderr, "Error: Could not read ul_carrierBandwidth from the configuration file.\n");
    }

    if (config_lookup_int(&cfg,
                          "gNBs.[0].servingCellConfigCommon.[0].initialULBWPlocationAndBandwidth",
                          &initialULBWPlocationAndBandwidth)) {
      printf("initialULBWPlocationAndBandwidth: %d\n", initialULBWPlocationAndBandwidth);
      initialULBWPlocationAndBandwidth = 32086;
      config_setting_t *setting = config_lookup(&cfg, "gNBs.[0].servingCellConfigCommon.[0].initialULBWPlocationAndBandwidth");
      config_setting_set_int(setting, initialULBWPlocationAndBandwidth);
      if (config_write_file(&cfg, "nr.conf")) {
        printf("Updated initialULBWPlocationAndBandwidth: %d\n", initialULBWPlocationAndBandwidth);
      } else {
        fprintf(stderr, "Error writing configuration file.\n");
      }
    } else {
      fprintf(stderr, "Error: Could not read initialULBWPlocationAndBandwidth from the configuration file.\n");
    }

    config_destroy(&cfg);
    AssertFatal(false, "Restarting...\n");
    }else if (strcmp(buf, "60") == 0) {
    if (config_lookup_int(&cfg, "gNBs.[0].servingCellConfigCommon.[0].absoluteFrequencySSB", &absoluteFrequencySSB)) {
      printf("absoluteFrequencySSB: %d\n", absoluteFrequencySSB);
      absoluteFrequencySSB = 621984;
      config_setting_t *setting = config_lookup(&cfg, "gNBs.[0].servingCellConfigCommon.[0].absoluteFrequencySSB");
      config_setting_set_int(setting, absoluteFrequencySSB);
      if (config_write_file(&cfg, "nr.conf")) {
        printf("Updated absoluteFrequencySSB: %d\n", absoluteFrequencySSB);
      } else {
        fprintf(stderr, "Error writing configuration file.\n");
      }
    } else {
      fprintf(stderr, "Error: Could not read absoluteFrequencySSB from the configuration file.\n");
    }

    if (config_lookup_int(&cfg, "gNBs.[0].servingCellConfigCommon.[0].dl_absoluteFrequencyPointA", &dl_absoluteFrequencyPointA)) {
      printf("dl_absoluteFrequencyPointA: %d\n", dl_absoluteFrequencyPointA);
      dl_absoluteFrequencyPointA = 620040;
      config_setting_t *setting = config_lookup(&cfg, "gNBs.[0].servingCellConfigCommon.[0].dl_absoluteFrequencyPointA");
      config_setting_set_int(setting, dl_absoluteFrequencyPointA);
      if (config_write_file(&cfg, "nr.conf")) {
        printf("Updated dl_absoluteFrequencyPointA: %d\n", dl_absoluteFrequencyPointA);
      } else {
        fprintf(stderr, "Error writing configuration file.\n");
      }
    } else {
      fprintf(stderr, "Error: Could not read dl_absoluteFrequencyPointA from the configuration file.\n");
    }

    if (config_lookup_int(&cfg, "gNBs.[0].servingCellConfigCommon.[0].dl_carrierBandwidth", &dl_carrierBandwidth)) {
      printf("dl_carrierBandwidth: %d\n", dl_carrierBandwidth);
      dl_carrierBandwidth = 162;
      config_setting_t *setting = config_lookup(&cfg, "gNBs.[0].servingCellConfigCommon.[0].dl_carrierBandwidth");
      config_setting_set_int(setting, dl_carrierBandwidth);
      if (config_write_file(&cfg, "nr.conf")) {
        printf("Updated dl_carrierBandwidth: %d\n", dl_carrierBandwidth);
      } else {
        fprintf(stderr, "Error writing configuration file.\n");
      }
    } else {
      fprintf(stderr, "Error: Could not read dl_carrierBandwidth from the configuration file.\n");
    }

    if (config_lookup_int(&cfg,
                          "gNBs.[0].servingCellConfigCommon.[0].initialDLBWPlocationAndBandwidth",
                          &initialDLBWPlocationAndBandwidth)) {
      printf("initialDLBWPlocationAndBandwidth: %d\n", initialDLBWPlocationAndBandwidth);
      initialDLBWPlocationAndBandwidth = 31624;
      config_setting_t *setting = config_lookup(&cfg, "gNBs.[0].servingCellConfigCommon.[0].initialDLBWPlocationAndBandwidth");
      config_setting_set_int(setting, initialDLBWPlocationAndBandwidth);
      if (config_write_file(&cfg, "nr.conf")) {
        printf("Updated initialDLBWPlocationAndBandwidth: %d\n", initialDLBWPlocationAndBandwidth);
} else {
        fprintf(stderr, "Error writing configuration file.\n");
      }
    } else {
      fprintf(stderr, "Error: Could not read initialDLBWPlocationAndBandwidth from the configuration file.\n");
    }

    if (config_lookup_int(&cfg, "gNBs.[0].servingCellConfigCommon.[0].ul_carrierBandwidth", &ul_carrierBandwidth)) {
      printf("ul_carrierBandwidth: %d\n", ul_carrierBandwidth);
      ul_carrierBandwidth = 162;
      config_setting_t *setting = config_lookup(&cfg, "gNBs.[0].servingCellConfigCommon.[0].ul_carrierBandwidth");
      config_setting_set_int(setting, ul_carrierBandwidth);
      if (config_write_file(&cfg, "nr.conf")) {
        printf("Updated ul_carrierBandwidth: %d\n", ul_carrierBandwidth);
      } else {
        fprintf(stderr, "Error writing configuration file.\n");
      }
    } else {
      fprintf(stderr, "Error: Could not read ul_carrierBandwidth from the configuration file.\n");
    }

    if (config_lookup_int(&cfg,
                          "gNBs.[0].servingCellConfigCommon.[0].initialULBWPlocationAndBandwidth",
                          &initialULBWPlocationAndBandwidth)) {
      printf("initialULBWPlocationAndBandwidth: %d\n", initialULBWPlocationAndBandwidth);
      initialULBWPlocationAndBandwidth = 31624;
      config_setting_t *setting = config_lookup(&cfg, "gNBs.[0].servingCellConfigCommon.[0].initialULBWPlocationAndBandwidth");
      config_setting_set_int(setting, initialULBWPlocationAndBandwidth);
      if (config_write_file(&cfg, "nr.conf")) {
        printf("Updated initialULBWPlocationAndBandwidth: %d\n", initialULBWPlocationAndBandwidth);
      } else {
        fprintf(stderr, "Error writing configuration file.\n");
      }
    } else {
      fprintf(stderr, "Error: Could not read initialULBWPlocationAndBandwidth from the configuration file.\n");
    }

    config_destroy(&cfg);
    AssertFatal(false, "Restarting...\n");
    }else {
    ERROR_MSG_RET("unhandled option %s\n", buf);
  }
  return 0;
}

static int set_pMaxconfig(char *buf, int debug, telnet_printfunc_t prnt)
{
  if (!buf)
    ERROR_MSG_RET("need param: o1 pMax \n");

  const gNB_MAC_INST *mac = RC.nrmac[0];
  const NR_ServingCellConfigCommon_t *scc = mac->common_channels[0].ServingCellConfigCommon;


  char *end = NULL;
  if (NULL != (end = strchr(buf, '\n')))
    *end = 0;

  if (NULL != (end = strchr(buf, '\r'))){
    printf("%s\n", buf);
    *end = 0;
  }
  if(atoi(buf) <= 30 && atoi(buf) >= -33)
    *scc->uplinkConfigCommon->frequencyInfoUL->p_Max = atoi(buf);

  else
     printf("unknown pMax value%s\n", buf);

  prnt("OK\n");

  return 0;

}

static int set_gNBId(char *buf, int debug, telnet_printfunc_t prnt)
{
  if (!buf)
    ERROR_MSG_RET("need param: o1 gNBId \n");

  char *end = NULL;
  if (NULL != (end = strchr(buf, '\n')))
    *end = 0;

  if (NULL != (end = strchr(buf, '\r'))){
    printf("%s\n", buf);
    *end = 0;
  }

  node_id = atoi(buf);
  printf("node_id is %d\n",node_id);
  prnt("OK\n");

  return 0;

}

static int set_nRFrequencyRef(char *buf, int debug, telnet_printfunc_t prnt)
{
  if (!buf)
    ERROR_MSG_RET("need param: o1 nRFrequencyRef \n");

  char *end = NULL;
  if (NULL != (end = strchr(buf, '\n')))
    *end = 0;

  if (NULL != (end = strchr(buf, '\r'))){
    printf("%s\n", buf);
    *end = 0;
  }

  // nRFrequencyRef = buf;
  strcpy(nRFrequencyRef, buf);
  printf(" nRFrequencyRef is %s\n", nRFrequencyRef);
  prnt("OK\n");

  return 0;

}

extern int stop_L1L2(module_id_t gnb_id);
static int stop_modem(char *buf, int debug, telnet_printfunc_t prnt)
{
  if (!running)
    ERROR_MSG_RET("cannot stop, nr-softmodem not running\n");

  /* make UEs out of sync and wait 20ms to ensure no PUCCH is scheduled. After
   * a restart, the frame/slot numbers will be different, which "confuses" the
   * scheduler, which has many PUCCH structures filled with expected frame/slot
   * combinations that won't happen. */
  // const gNB_MAC_INST *mac = RC.nrmac[0];
  // UE_iterator((NR_UE_info_t **)mac->UE_info.list, it) {
  //   it->UE_sched_ctrl.rrc_processing_timer = 1000;
  // }
  // usleep(50000);

  // stop_L1L2(0);
  // running = false;
  // prnt("OK\n");
  return 0;
}

extern int start_L1L2(module_id_t gnb_id);
static int start_modem(char *buf, int debug, telnet_printfunc_t prnt)
{
  if (running)
    ERROR_MSG_RET("cannot start, nr-softmodem already running\n");
  start_L1L2(0);
  running = true;
  prnt("OK\n");
  return 0;
}

static telnetshell_cmddef_t o1cmds[] = {
  {"stats", "", get_stats},
  {"config", "[]", set_config},
  {"config", "[]", set_cu_config},
  {"bwconfig", "", set_bwconfig},
  {"pMax", "", set_pMaxconfig},
  {"gnbid", "", set_gNBId},
  {"nRFrequencyRef", "", set_nRFrequencyRef},
  {"stop_modem", "", stop_modem},
  {"start_modem", "", start_modem},
  {"", "", NULL},
};

static telnetshell_vardef_t o1vars[] = {
  {"", 0, 0, NULL}
};

void add_o1_cmds(void) {
  add_telnetcmd("o1", o1vars, o1cmds);
}
