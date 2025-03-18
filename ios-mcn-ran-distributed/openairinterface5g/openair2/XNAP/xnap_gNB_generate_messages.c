/* Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
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

#include <stdio.h>
#include "intertask_interface.h"
#include "xnap_common.h"
#include "xnap_gNB_task.h"
#include "xnap_gNB_generate_messages.h"
#include "XNAP_ProtocolIE-Field.h"
#include "XNAP_GlobalgNB-ID.h"
#include "XNAP_ServedCells-NR-Item.h"
#include "XNAP_ServedCellInformation-NR.h"
#include "XNAP_NRFrequencyBandItem.h"
#include "xnap_gNB_itti_messaging.h"
#include "XNAP_ServedCells-NR.h"
#include "assertions.h"
#include "conversions.h"
#include "XNAP_BroadcastPLMNinTAISupport-Item.h"
#include "XNAP_TAISupport-Item.h"
#include "XNAP_GlobalAMF-Region-Information.h"
#include "XNAP_TargetCellList-Item.h"
#include "XNAP_GlobalAMF-Region-Information.h"
#include "XNAP_NRModeInfoFDD.h"
#include "XNAP_NRModeInfoTDD.h"
#include "openair2/RRC/NR/nr_rrc_defs.h"
#include "xnap_gNB_defs.h"
#include "XNAP_PDUSessionResourcesToBeSetup-Item.h"
#include "XNAP_QoSFlowsToBeSetup-Item.h"
#include "XNAP_LastVisitedCell-Item.h"
#include "XNAP_GTPtunnelTransportLayerInformation.h"
#include "XNAP_NonDynamic5QIDescriptor.h"
#include "XNAP_Dynamic5QIDescriptor.h"
#include "xnap_ids.h"
#include "NR_HandoverCommand.h"
#include "NR_CellGroupConfig.h"
#include "NR_RRCReconfiguration-IEs.h"
#include "NR_SpCellConfig.h"
#include "NR_ReconfigurationWithSync.h"
#include "NR_DL-DCCH-Message.h"
#include "SIMULATION/TOOLS/sim.h"
#include "NR_RACH-ConfigGeneric.h"
#include "XNAP_QoSFlowsAdmitted-Item.h"
#include "XNAP_PDUSessionResourcesAdmitted-Item.h"

int xnap_gNB_generate_xn_setup_request(sctp_assoc_t assoc_id, xnap_setup_req_t *req)
{
  XNAP_XnAP_PDU_t pdu;
  XNAP_XnSetupRequest_t *out;
  XNAP_XnSetupRequest_IEs_t *ie;
  XNAP_BroadcastPLMNinTAISupport_Item_t *e_BroadcastPLMNinTAISupport_ItemIE;
  XNAP_TAISupport_Item_t *TAISupport_ItemIEs;
  XNAP_S_NSSAI_t *e_S_NSSAI_ItemIE;
  XNAP_GlobalAMF_Region_Information_t *e_GlobalAMF_Region_Information_ItemIEs;
  XNAP_ServedCells_NR_Item_t *servedCellMember;
  XNAP_NRFrequencyBandItem_t *nrfreqbanditemul;
  XNAP_NRFrequencyBandItem_t *nrfreqbanditemdl;
  XNAP_NRFrequencyBandItem_t *nrfreqbanditem;
  XNAP_PLMN_Identity_t *plmn;
  uint8_t *buffer;
  uint32_t len;
  int ret = 0;

  /* Message Type */
  memset(&pdu, 0, sizeof(pdu));
  pdu.present = XNAP_XnAP_PDU_PR_initiatingMessage;
  pdu.choice.initiatingMessage = (XNAP_InitiatingMessage_t *)calloc(1, sizeof(XNAP_InitiatingMessage_t));
  pdu.choice.initiatingMessage->procedureCode = XNAP_ProcedureCode_id_xnSetup;
  pdu.choice.initiatingMessage->criticality = XNAP_Criticality_reject;
  pdu.choice.initiatingMessage->value.present = XNAP_InitiatingMessage__value_PR_XnSetupRequest;
  out = &pdu.choice.initiatingMessage->value.choice.XnSetupRequest;

  /* mandatory */
  /* Global NG-RAN Node ID */
  ie = (XNAP_XnSetupRequest_IEs_t *)calloc(1, sizeof(XNAP_XnSetupRequest_IEs_t));
  ie->id = XNAP_ProtocolIE_ID_id_GlobalNG_RAN_node_ID;
  ie->criticality = XNAP_Criticality_reject;
  ie->value.present = XNAP_XnSetupRequest_IEs__value_PR_GlobalNG_RANNode_ID;
  ie->value.choice.GlobalNG_RANNode_ID.present = XNAP_GlobalNG_RANNode_ID_PR_gNB;
  ie->value.choice.GlobalNG_RANNode_ID.choice.gNB = (XNAP_GlobalgNB_ID_t *)calloc(1, sizeof(XNAP_GlobalgNB_ID_t));
  MCC_MNC_TO_PLMNID(req->info.plmn.mcc,
                    req->info.plmn.mnc,
                    req->info.plmn.mnc_digit_length,
                    &ie->value.choice.GlobalNG_RANNode_ID.choice.gNB->plmn_id);
  ie->value.choice.GlobalNG_RANNode_ID.choice.gNB->gnb_id.present = XNAP_GNB_ID_Choice_PR_gnb_ID;
  MACRO_GNB_ID_TO_BIT_STRING(req->gNB_id, &ie->value.choice.GlobalNG_RANNode_ID.choice.gNB->gnb_id.choice.gnb_ID); // 28 bits
  LOG_I(XNAP,
        "%lu -> %02x%02x%02x\n",
        req->gNB_id,
        ie->value.choice.GlobalNG_RANNode_ID.choice.gNB->gnb_id.choice.gnb_ID.buf[0],
        ie->value.choice.GlobalNG_RANNode_ID.choice.gNB->gnb_id.choice.gnb_ID.buf[1],
        ie->value.choice.GlobalNG_RANNode_ID.choice.gNB->gnb_id.choice.gnb_ID.buf[2]);
  asn1cSeqAdd(&out->protocolIEs.list, ie);

  /* mandatory */
  /* TAI Support list */
  ie = (XNAP_XnSetupRequest_IEs_t *)calloc(1, sizeof(XNAP_XnSetupRequest_IEs_t));
  ie->id = XNAP_ProtocolIE_ID_id_TAISupport_list;
  ie->criticality = XNAP_Criticality_reject;
  ie->value.present = XNAP_XnSetupRequest_IEs__value_PR_TAISupport_List;
  {
    TAISupport_ItemIEs = (XNAP_TAISupport_Item_t *)calloc(1, sizeof(XNAP_TAISupport_Item_t));
    INT24_TO_OCTET_STRING(req->tai_support, &TAISupport_ItemIEs->tac);
    {
      for (int j = 0; j < 1; j++) {
        e_BroadcastPLMNinTAISupport_ItemIE =
            (XNAP_BroadcastPLMNinTAISupport_Item_t *)calloc(1, sizeof(XNAP_BroadcastPLMNinTAISupport_Item_t));
        MCC_MNC_TO_PLMNID(req->info.plmn.mcc,
                          req->info.plmn.mnc,
                          req->info.plmn.mnc_digit_length,
                          &e_BroadcastPLMNinTAISupport_ItemIE->plmn_id);
        for (int k = 0; k < 1; k++) {
          e_S_NSSAI_ItemIE = (XNAP_S_NSSAI_t *)calloc(1, sizeof(XNAP_S_NSSAI_t));
          e_S_NSSAI_ItemIE->sst.size = 1; // OCTET STRING(SIZE(1))
          e_S_NSSAI_ItemIE->sst.buf = calloc(e_S_NSSAI_ItemIE->sst.size, sizeof(OCTET_STRING_t));
          e_S_NSSAI_ItemIE->sst.buf[0] = 1;
          asn1cSeqAdd(&e_BroadcastPLMNinTAISupport_ItemIE->tAISliceSupport_List.list, e_S_NSSAI_ItemIE);
        }
        asn1cSeqAdd(&TAISupport_ItemIEs->broadcastPLMNs.list, e_BroadcastPLMNinTAISupport_ItemIE);
      }
    }
    asn1cSeqAdd(&ie->value.choice.TAISupport_List.list, TAISupport_ItemIEs);
  }
  asn1cSeqAdd(&out->protocolIEs.list, ie);

  /* mandatory */
  /* List of Served Cells NR */
  ie = (XNAP_XnSetupRequest_IEs_t *)calloc(1, sizeof(XNAP_XnSetupRequest_IEs_t));
  ie->id = XNAP_ProtocolIE_ID_id_List_of_served_cells_NR;
  ie->criticality = XNAP_Criticality_reject;
  ie->value.present = XNAP_XnSetupRequest_IEs__value_PR_ServedCells_NR;
  {
    servedCellMember = (XNAP_ServedCells_NR_Item_t *)calloc(1, sizeof(XNAP_ServedCells_NR_Item_t));
    {
      servedCellMember->served_cell_info_NR.nrPCI = req->info.nr_pci; // long
      MCC_MNC_TO_PLMNID(req->info.plmn.mcc,
                        req->info.plmn.mnc,
                        req->info.plmn.mnc_digit_length,
                        &servedCellMember->served_cell_info_NR.cellID.plmn_id); // octet string
      NR_CELL_ID_TO_BIT_STRING(req->info.nr_cellid,
                               &servedCellMember->served_cell_info_NR.cellID.nr_CI); // bit string
      INT24_TO_OCTET_STRING(req->info.tac, &servedCellMember->served_cell_info_NR.tac); // octet string
      for (int k = 0; k < 1; k++) {
        plmn = (XNAP_PLMN_Identity_t *)calloc(1, sizeof(XNAP_PLMN_Identity_t));
        {
          MCC_MNC_TO_PLMNID(req->info.plmn.mcc, req->info.plmn.mnc, req->info.plmn.mnc_digit_length, plmn);
          asn1cSeqAdd(&servedCellMember->served_cell_info_NR.broadcastPLMN.list, plmn);
        }
      }
      if (req->info.mode == XNAP_MODE_FDD) { // FDD
        const xnap_fdd_info_t *fdd = &req->info.fdd;
        servedCellMember->served_cell_info_NR.nrModeInfo.present = XNAP_NRModeInfo_PR_fdd;
        servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd =
            (XNAP_NRModeInfoFDD_t *)calloc(1, sizeof(XNAP_NRModeInfoFDD_t));
        servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd->ulNRFrequencyInfo.nrARFCN = fdd->ul_freqinfo.arfcn;
        for (int j = 0; j < 1; j++) { // fdd ul number of available freq_Bands = 1
          nrfreqbanditemul = (XNAP_NRFrequencyBandItem_t *)calloc(1, sizeof(XNAP_NRFrequencyBandItem_t));
          nrfreqbanditemul->nr_frequency_band = fdd->ul_freqinfo.band;
          asn1cSeqAdd(&servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd->ulNRFrequencyInfo.frequencyBand_List.list,
                      nrfreqbanditemul);
        }
        servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd->dlNRFrequencyInfo.nrARFCN = fdd->dl_freqinfo.arfcn;
        for (int j = 0; j < 1; j++) { ////fdd dl number of available freq_Bands = 1
          nrfreqbanditemdl = (XNAP_NRFrequencyBandItem_t *)calloc(1, sizeof(XNAP_NRFrequencyBandItem_t));
          nrfreqbanditemdl->nr_frequency_band = fdd->dl_freqinfo.band;
          asn1cSeqAdd(&servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd->ulNRFrequencyInfo.frequencyBand_List.list,
                      nrfreqbanditemdl);
        }
        switch (fdd->ul_tbw.scs) {
          case 15:
            servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd->ulNRTransmissonBandwidth.nRSCS = XNAP_NRSCS_scs15;
            break;
          case 30:
            servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd->ulNRTransmissonBandwidth.nRSCS = XNAP_NRSCS_scs30;
            break;
          case 60:
            servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd->ulNRTransmissonBandwidth.nRSCS = XNAP_NRSCS_scs60;
            break;
          case 120:
            servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd->ulNRTransmissonBandwidth.nRSCS = XNAP_NRSCS_scs120;
            break;
        }
        switch (fdd->ul_tbw.nrb) {
          case 11:
            servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd->ulNRTransmissonBandwidth.nRNRB = XNAP_NRNRB_nrb11;
            break;
          case 18:
            servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd->ulNRTransmissonBandwidth.nRNRB = XNAP_NRNRB_nrb18;
            break;
          case 24:
            servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd->ulNRTransmissonBandwidth.nRNRB = XNAP_NRNRB_nrb24;
            break;
          case 78:
            servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd->ulNRTransmissonBandwidth.nRNRB = XNAP_NRNRB_nrb78;
            break;
          case 106:
            servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd->ulNRTransmissonBandwidth.nRNRB = XNAP_NRNRB_nrb106;
            break;
          case 162:
            servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd->ulNRTransmissonBandwidth.nRNRB = XNAP_NRNRB_nrb162;
            break;
          case 217:
            servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd->ulNRTransmissonBandwidth.nRNRB = XNAP_NRNRB_nrb217;
            break;
          case 273:
            servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd->ulNRTransmissonBandwidth.nRNRB = XNAP_NRNRB_nrb273;
            break;
          default:
            AssertFatal(0, "Failed: Check value for N_RB_DL/N_RB_UL");
            break;
        }
        switch (fdd->dl_tbw.scs) {
          case 15:
            servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd->dlNRTransmissonBandwidth.nRSCS = XNAP_NRSCS_scs15;
            break;
          case 30:
            servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd->dlNRTransmissonBandwidth.nRSCS = XNAP_NRSCS_scs30;
            break;
          case 60:
            servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd->dlNRTransmissonBandwidth.nRSCS = XNAP_NRSCS_scs60;
            break;
          case 120:
            servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd->dlNRTransmissonBandwidth.nRSCS = XNAP_NRSCS_scs120;
            break;
        }
        switch (fdd->dl_tbw.nrb) {
          case 11:
            servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd->dlNRTransmissonBandwidth.nRNRB = XNAP_NRNRB_nrb11;
            break;
          case 18:
            servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd->dlNRTransmissonBandwidth.nRNRB = XNAP_NRNRB_nrb18;
            break;
          case 24:
            servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd->dlNRTransmissonBandwidth.nRNRB = XNAP_NRNRB_nrb24;
            break;
          case 78:
            servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd->dlNRTransmissonBandwidth.nRNRB = XNAP_NRNRB_nrb78;
            break;
          case 106:
            servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd->dlNRTransmissonBandwidth.nRNRB = XNAP_NRNRB_nrb106;
            break;
          case 162:
            servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd->dlNRTransmissonBandwidth.nRNRB = XNAP_NRNRB_nrb162;
            break;
          case 217:
            servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd->dlNRTransmissonBandwidth.nRNRB = XNAP_NRNRB_nrb217;
            break;
          case 273:
            servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd->dlNRTransmissonBandwidth.nRNRB = XNAP_NRNRB_nrb273;
            break;
          default:
            AssertFatal(0, "Failed: Check value for N_RB_DL/N_RB_UL");
            break;
        }
      } else if (req->info.mode == XNAP_MODE_TDD) { // TDD
        const xnap_tdd_info_t *tdd = &req->info.tdd;
        servedCellMember->served_cell_info_NR.nrModeInfo.present = XNAP_NRModeInfo_PR_tdd;
        servedCellMember->served_cell_info_NR.nrModeInfo.choice.tdd =
            (XNAP_NRModeInfoTDD_t *)calloc(1, sizeof(XNAP_NRModeInfoTDD_t));
        servedCellMember->served_cell_info_NR.nrModeInfo.choice.tdd->nrFrequencyInfo.nrARFCN = tdd->freqinfo.arfcn;
        for (int j = 0; j < 1; j++) { // number of available bands = j = 1
          nrfreqbanditem = (XNAP_NRFrequencyBandItem_t *)calloc(1, sizeof(XNAP_NRFrequencyBandItem_t));
          nrfreqbanditem->nr_frequency_band = tdd->freqinfo.band;
          asn1cSeqAdd(&servedCellMember->served_cell_info_NR.nrModeInfo.choice.tdd->nrFrequencyInfo.frequencyBand_List.list,
                      nrfreqbanditem);
        }
        switch (tdd->tbw.scs) {
          case 0:
            servedCellMember->served_cell_info_NR.nrModeInfo.choice.tdd->nrTransmissonBandwidth.nRSCS = XNAP_NRSCS_scs15;
            break;
          case 1:
            servedCellMember->served_cell_info_NR.nrModeInfo.choice.tdd->nrTransmissonBandwidth.nRSCS = XNAP_NRSCS_scs30;
            break;
          case 2:
            servedCellMember->served_cell_info_NR.nrModeInfo.choice.tdd->nrTransmissonBandwidth.nRSCS = XNAP_NRSCS_scs60;
            break;
          case 3:
            servedCellMember->served_cell_info_NR.nrModeInfo.choice.tdd->nrTransmissonBandwidth.nRSCS = XNAP_NRSCS_scs120;
            break;
        }
        switch (tdd->tbw.nrb) {
          case 11:
            servedCellMember->served_cell_info_NR.nrModeInfo.choice.tdd->nrTransmissonBandwidth.nRNRB = XNAP_NRNRB_nrb11;
            break;
          case 18:
            servedCellMember->served_cell_info_NR.nrModeInfo.choice.tdd->nrTransmissonBandwidth.nRNRB = XNAP_NRNRB_nrb18;
            break;
          case 24:
            servedCellMember->served_cell_info_NR.nrModeInfo.choice.tdd->nrTransmissonBandwidth.nRNRB = XNAP_NRNRB_nrb24;
            break;
          case 78:
            servedCellMember->served_cell_info_NR.nrModeInfo.choice.tdd->nrTransmissonBandwidth.nRNRB = XNAP_NRNRB_nrb78;
            break;
          case 106:
            servedCellMember->served_cell_info_NR.nrModeInfo.choice.tdd->nrTransmissonBandwidth.nRNRB = XNAP_NRNRB_nrb106;
            break;
          case 162:
            servedCellMember->served_cell_info_NR.nrModeInfo.choice.tdd->nrTransmissonBandwidth.nRNRB = XNAP_NRNRB_nrb162;
            break;
          case 217:
            servedCellMember->served_cell_info_NR.nrModeInfo.choice.tdd->nrTransmissonBandwidth.nRNRB = XNAP_NRNRB_nrb217;
            break;
          case 273:
            servedCellMember->served_cell_info_NR.nrModeInfo.choice.tdd->nrTransmissonBandwidth.nRNRB = XNAP_NRNRB_nrb273;
            break;
          default:
            AssertFatal(0, "Failed: Check value for N_RB_DL/N_RB_UL");
            break;
        }
      }
      // Setting MTC to 0 now. Will be handled later.
      INT8_TO_OCTET_STRING(0, &servedCellMember->served_cell_info_NR.measurementTimingConfiguration);
      servedCellMember->served_cell_info_NR.connectivitySupport.eNDC_Support = 1;
    }
    asn1cSeqAdd(&ie->value.choice.ServedCells_NR.list, servedCellMember);
  }
  asn1cSeqAdd(&out->protocolIEs.list, ie);

  /* mandatory */
  /* AMF Region Information */
  ie = (XNAP_XnSetupRequest_IEs_t *)calloc(1, sizeof(XNAP_XnSetupRequest_IEs_t));
  ie->id = XNAP_ProtocolIE_ID_id_AMF_Region_Information;
  ie->criticality = XNAP_Criticality_reject;
  ie->value.present = XNAP_XnSetupRequest_IEs__value_PR_AMF_Region_Information;
  {
    e_GlobalAMF_Region_Information_ItemIEs =
        (XNAP_GlobalAMF_Region_Information_t *)calloc(1, sizeof(XNAP_GlobalAMF_Region_Information_t));

    MCC_MNC_TO_PLMNID(req->info.plmn.mcc,
                      req->info.plmn.mnc,
                      req->info.plmn.mnc_digit_length,
                      &e_GlobalAMF_Region_Information_ItemIEs->plmn_ID);
    e_GlobalAMF_Region_Information_ItemIEs->amf_region_id.size = 1;
    e_GlobalAMF_Region_Information_ItemIEs->amf_region_id.buf =
        calloc(1, e_GlobalAMF_Region_Information_ItemIEs->amf_region_id.size);
    e_GlobalAMF_Region_Information_ItemIEs->amf_region_id.buf[0] = 80; // TODO: Hardcoded for now
    e_GlobalAMF_Region_Information_ItemIEs->amf_region_id.bits_unused = 0;

    asn1cSeqAdd(&ie->value.choice.AMF_Region_Information.list, e_GlobalAMF_Region_Information_ItemIEs);
  }
  asn1cSeqAdd(&out->protocolIEs.list, ie);

  if (xnap_gNB_encode_pdu(&pdu, &buffer, &len) < 0) {
    LOG_E(XNAP, "Failed to encode Xn setup request\n");
    return -1;
  }
  xnap_gNB_itti_send_sctp_data_req(assoc_id, buffer, len, 0);
  return ret;
}

int xnap_gNB_generate_xn_setup_failure(sctp_assoc_t assoc_id, xnap_setup_failure_t *fail)
{
  XNAP_XnAP_PDU_t pdu;
  XNAP_XnSetupFailure_t *out;
  XNAP_XnSetupFailure_IEs_t *ie;
  uint8_t *buffer;
  uint32_t len;
  int ret = 0;

  /* Message Type */
  memset(&pdu, 0, sizeof(pdu));
  pdu.present = XNAP_XnAP_PDU_PR_unsuccessfulOutcome;
  pdu.choice.unsuccessfulOutcome = (XNAP_UnsuccessfulOutcome_t *)calloc(1, sizeof(XNAP_UnsuccessfulOutcome_t));
  pdu.choice.unsuccessfulOutcome->procedureCode = XNAP_ProcedureCode_id_xnSetup;
  pdu.choice.unsuccessfulOutcome->criticality = XNAP_Criticality_reject;
  pdu.choice.unsuccessfulOutcome->value.present = XNAP_UnsuccessfulOutcome__value_PR_XnSetupFailure;
  out = &pdu.choice.unsuccessfulOutcome->value.choice.XnSetupFailure;

  /* mandatory */
  /* Cause */
  ie = (XNAP_XnSetupFailure_IEs_t *)calloc(1, sizeof(XNAP_XnSetupFailure_IEs_t));
  ie->id = XNAP_ProtocolIE_ID_id_Cause;
  ie->criticality = XNAP_Criticality_ignore;
  ie->value.present = XNAP_XnSetupFailure_IEs__value_PR_Cause;
  xnap_gNB_set_cause(&ie->value.choice.Cause, fail->cause_type, fail->cause_value);
  asn1cSeqAdd(&out->protocolIEs.list, ie);

  if (xnap_gNB_encode_pdu(&pdu, &buffer, &len) < 0) {
    LOG_E(XNAP, "Failed to encode Xn setup failure\n");
    return -1;
  }
  xnap_gNB_itti_send_sctp_data_req(assoc_id, buffer, len, 0);
  return ret;
}

int xnap_gNB_set_cause(XNAP_Cause_t *cause_p, XNAP_Cause_PR cause_type, long cause_value)
{
  DevAssert(cause_p != NULL);
  switch (cause_type) {
    case XNAP_CAUSE_RADIO_NETWORK:
      cause_p->present = XNAP_Cause_PR_radioNetwork;
      cause_p->choice.radioNetwork = cause_value;
      break;
    case XNAP_CAUSE_TRANSPORT:
      cause_p->present = XNAP_Cause_PR_transport;
      cause_p->choice.transport = cause_value;
      break;
    case XNAP_CAUSE_PROTOCOL:
      cause_p->present = XNAP_Cause_PR_protocol;
      cause_p->choice.protocol = cause_value;
      break;
    case XNAP_CAUSE_MISC:
      cause_p->present = XNAP_Cause_PR_misc;
      cause_p->choice.misc = cause_value;
      break;
    case XNAP_CAUSE_NOTHING:
    default:
      cause_p->present = XNAP_Cause_PR_NOTHING;
      break;
  }
  return 0;
}

int xnap_gNB_generate_xn_setup_response(sctp_assoc_t assoc_id, xnap_setup_resp_t *resp)
{
  XNAP_XnAP_PDU_t pdu;
  uint8_t *buffer = NULL;
  uint32_t len = 0;
  int ret = 0;
  XNAP_XnSetupResponse_t *out;
  XNAP_XnSetupResponse_IEs_t *ie;
  XNAP_PLMN_Identity_t *plmn;
  XNAP_BroadcastPLMNinTAISupport_Item_t *e_BroadcastPLMNinTAISupport_ItemIE;
  XNAP_TAISupport_Item_t *TAISupport_ItemIEs;
  XNAP_S_NSSAI_t *e_S_NSSAI_ItemIE;
  XNAP_ServedCells_NR_Item_t *servedCellMember;
  XNAP_NRFrequencyBandItem_t *nrfreqbanditemul;
  XNAP_NRFrequencyBandItem_t *nrfreqbanditemdl;
  XNAP_NRFrequencyBandItem_t *nrfreqbanditem;

  /* Message Type */
  memset(&pdu, 0, sizeof(pdu));
  pdu.present = XNAP_XnAP_PDU_PR_successfulOutcome;
  pdu.choice.successfulOutcome = (XNAP_SuccessfulOutcome_t *)calloc(1, sizeof(XNAP_SuccessfulOutcome_t));
  pdu.choice.successfulOutcome->procedureCode = XNAP_ProcedureCode_id_xnSetup;
  pdu.choice.successfulOutcome->criticality = XNAP_Criticality_reject;
  pdu.choice.successfulOutcome->value.present = XNAP_SuccessfulOutcome__value_PR_XnSetupResponse;
  out = &pdu.choice.successfulOutcome->value.choice.XnSetupResponse;

  /* mandatory */
  /* Global NG-RAN Node ID */
  ie = (XNAP_XnSetupResponse_IEs_t *)calloc(1, sizeof(XNAP_XnSetupResponse_IEs_t));
  ie->id = XNAP_ProtocolIE_ID_id_GlobalNG_RAN_node_ID;
  ie->criticality = XNAP_Criticality_reject;
  ie->value.present = XNAP_XnSetupResponse_IEs__value_PR_GlobalNG_RANNode_ID;
  ie->value.choice.GlobalNG_RANNode_ID.present = XNAP_GlobalNG_RANNode_ID_PR_gNB;
  ie->value.choice.GlobalNG_RANNode_ID.choice.gNB = (XNAP_GlobalgNB_ID_t *)calloc(1, sizeof(XNAP_GlobalgNB_ID_t));
  MCC_MNC_TO_PLMNID(resp->info.plmn.mcc,
                    resp->info.plmn.mnc,
                    resp->info.plmn.mnc_digit_length,
                    &ie->value.choice.GlobalNG_RANNode_ID.choice.gNB->plmn_id);
  ie->value.choice.GlobalNG_RANNode_ID.choice.gNB->gnb_id.present = XNAP_GNB_ID_Choice_PR_gnb_ID;
  MACRO_GNB_ID_TO_BIT_STRING(resp->gNB_id, &ie->value.choice.GlobalNG_RANNode_ID.choice.gNB->gnb_id.choice.gnb_ID);
  LOG_I(XNAP,
        "%ld -> %02x%02x%02x\n",
        resp->gNB_id,
        ie->value.choice.GlobalNG_RANNode_ID.choice.gNB->gnb_id.choice.gnb_ID.buf[0],
        ie->value.choice.GlobalNG_RANNode_ID.choice.gNB->gnb_id.choice.gnb_ID.buf[1],
        ie->value.choice.GlobalNG_RANNode_ID.choice.gNB->gnb_id.choice.gnb_ID.buf[2]);
  asn1cSeqAdd(&out->protocolIEs.list, ie);

  /* mandatory */
  /* TAI Support list */
  ie = (XNAP_XnSetupResponse_IEs_t *)calloc(1, sizeof(XNAP_XnSetupResponse_IEs_t));
  ie->id = XNAP_ProtocolIE_ID_id_TAISupport_list;
  ie->criticality = XNAP_Criticality_reject;
  ie->value.present = XNAP_XnSetupResponse_IEs__value_PR_TAISupport_List;
  for (int i = 0; i < 1; i++) {
    TAISupport_ItemIEs = (XNAP_TAISupport_Item_t *)calloc(1, sizeof(XNAP_TAISupport_Item_t));
    INT24_TO_OCTET_STRING(resp->tai_support, &TAISupport_ItemIEs->tac);
    {
      for (int j = 0; j < 1; j++) {
        e_BroadcastPLMNinTAISupport_ItemIE =
            (XNAP_BroadcastPLMNinTAISupport_Item_t *)calloc(1, sizeof(XNAP_BroadcastPLMNinTAISupport_Item_t));
        MCC_MNC_TO_PLMNID(resp->info.plmn.mcc,
                          resp->info.plmn.mnc,
                          resp->info.plmn.mnc_digit_length,
                          &e_BroadcastPLMNinTAISupport_ItemIE->plmn_id);
        {
          for (int k = 0; k < 1; k++) {
            e_S_NSSAI_ItemIE = (XNAP_S_NSSAI_t *)calloc(1, sizeof(XNAP_S_NSSAI_t));
            e_S_NSSAI_ItemIE->sst.size = 1; // OCTET STRING(SIZE(1))
            e_S_NSSAI_ItemIE->sst.buf = calloc(e_S_NSSAI_ItemIE->sst.size, sizeof(OCTET_STRING_t));
            e_S_NSSAI_ItemIE->sst.buf[0] = 1;

            asn1cSeqAdd(&e_BroadcastPLMNinTAISupport_ItemIE->tAISliceSupport_List.list, e_S_NSSAI_ItemIE);
          }
        }
        asn1cSeqAdd(&TAISupport_ItemIEs->broadcastPLMNs.list, e_BroadcastPLMNinTAISupport_ItemIE);
      }
    }
    asn1cSeqAdd(&ie->value.choice.TAISupport_List.list, TAISupport_ItemIEs);
  }
  asn1cSeqAdd(&out->protocolIEs.list, ie);

  /* mandatory */
  /* List of Served Cells NR */
  ie = (XNAP_XnSetupResponse_IEs_t *)calloc(1, sizeof(XNAP_XnSetupResponse_IEs_t));
  ie->id = XNAP_ProtocolIE_ID_id_List_of_served_cells_NR;
  ie->criticality = XNAP_Criticality_reject;
  ie->value.present = XNAP_XnSetupResponse_IEs__value_PR_ServedCells_NR;
  {
    servedCellMember = (XNAP_ServedCells_NR_Item_t *)calloc(1, sizeof(XNAP_ServedCells_NR_Item_t));
    servedCellMember->served_cell_info_NR.nrPCI = resp->info.nr_pci; // long
    MCC_MNC_TO_PLMNID(resp->info.plmn.mcc,
                      resp->info.plmn.mnc,
                      resp->info.plmn.mnc_digit_length,
                      &servedCellMember->served_cell_info_NR.cellID.plmn_id); // octet string
    NR_CELL_ID_TO_BIT_STRING(resp->info.nr_cellid,
                             &servedCellMember->served_cell_info_NR.cellID.nr_CI); // bit string
    INT24_TO_OCTET_STRING(resp->info.tac, &servedCellMember->served_cell_info_NR.tac); // octet string
    for (int k = 0; k < 1; k++) {
      plmn = (XNAP_PLMN_Identity_t *)calloc(1, sizeof(XNAP_PLMN_Identity_t));
      {
        MCC_MNC_TO_PLMNID(resp->info.plmn.mcc, resp->info.plmn.mnc, resp->info.plmn.mnc_digit_length, plmn);
        asn1cSeqAdd(&servedCellMember->served_cell_info_NR.broadcastPLMN.list, plmn);
      }
    }
    if (resp->info.mode == XNAP_MODE_FDD) { // FDD
      const xnap_fdd_info_t *fdd = &resp->info.fdd;
      servedCellMember->served_cell_info_NR.nrModeInfo.present = XNAP_NRModeInfo_PR_fdd;
      servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd = (XNAP_NRModeInfoFDD_t *)calloc(1, sizeof(XNAP_NRModeInfoFDD_t));
      servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd->ulNRFrequencyInfo.nrARFCN = fdd->ul_freqinfo.arfcn;
      for (int j = 0; j < 1; j++) { // fdd ul number of available freq_Bands = 1
        nrfreqbanditemul = (XNAP_NRFrequencyBandItem_t *)calloc(1, sizeof(XNAP_NRFrequencyBandItem_t));
        nrfreqbanditemul->nr_frequency_band = fdd->ul_freqinfo.band;
        asn1cSeqAdd(&servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd->ulNRFrequencyInfo.frequencyBand_List.list,
                    nrfreqbanditemul);
      }
      servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd->dlNRFrequencyInfo.nrARFCN = fdd->dl_freqinfo.arfcn;
      for (int j = 0; j < 1; j++) { ////fdd dl number of available freq_Bands = 1
        nrfreqbanditemdl = (XNAP_NRFrequencyBandItem_t *)calloc(1, sizeof(XNAP_NRFrequencyBandItem_t));
        nrfreqbanditemdl->nr_frequency_band = fdd->dl_freqinfo.band;
        asn1cSeqAdd(&servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd->ulNRFrequencyInfo.frequencyBand_List.list,
                    nrfreqbanditemdl);
      }
      switch (fdd->ul_tbw.scs) {
        case 15:
          servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd->ulNRTransmissonBandwidth.nRSCS = XNAP_NRSCS_scs15;
          break;
        case 30:
          servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd->ulNRTransmissonBandwidth.nRSCS = XNAP_NRSCS_scs30;
          break;
        case 60:
          servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd->ulNRTransmissonBandwidth.nRSCS = XNAP_NRSCS_scs60;
          break;
        case 120:
          servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd->ulNRTransmissonBandwidth.nRSCS = XNAP_NRSCS_scs120;
          break;
      }
      switch (fdd->ul_tbw.nrb) {
        case 0:
          servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd->ulNRTransmissonBandwidth.nRNRB = XNAP_NRNRB_nrb11;
          break;
        case 1:
          servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd->ulNRTransmissonBandwidth.nRNRB = XNAP_NRNRB_nrb18;
          break;
        case 2:
          servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd->ulNRTransmissonBandwidth.nRNRB = XNAP_NRNRB_nrb24;
          break;
        case 11:
          servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd->ulNRTransmissonBandwidth.nRNRB = XNAP_NRNRB_nrb78;
          break;
        case 14:
          servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd->ulNRTransmissonBandwidth.nRNRB = XNAP_NRNRB_nrb106;
          break;
        case 21:
          servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd->ulNRTransmissonBandwidth.nRNRB = XNAP_NRNRB_nrb162;
          break;
        case 24:
          servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd->ulNRTransmissonBandwidth.nRNRB = XNAP_NRNRB_nrb217;
          break;
        case 28:
          servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd->ulNRTransmissonBandwidth.nRNRB = XNAP_NRNRB_nrb273;
          break;
        default:
          AssertFatal(0, "Failed: Check value for N_RB_DL/N_RB_UL");
          break;
      }
      switch (fdd->dl_tbw.scs) {
        case 15:
          servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd->dlNRTransmissonBandwidth.nRSCS = XNAP_NRSCS_scs15;
          break;
        case 30:
          servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd->dlNRTransmissonBandwidth.nRSCS = XNAP_NRSCS_scs30;
          break;
        case 60:
          servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd->dlNRTransmissonBandwidth.nRSCS = XNAP_NRSCS_scs60;
          break;
        case 120:
          servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd->dlNRTransmissonBandwidth.nRSCS = XNAP_NRSCS_scs120;
          break;
      }
      switch (fdd->dl_tbw.nrb) {
        case 0:
          servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd->dlNRTransmissonBandwidth.nRNRB = XNAP_NRNRB_nrb11;
          break;
        case 1:
          servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd->dlNRTransmissonBandwidth.nRNRB = XNAP_NRNRB_nrb18;
          break;
        case 2:
          servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd->dlNRTransmissonBandwidth.nRNRB = XNAP_NRNRB_nrb24;
          break;
        case 11:
          servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd->dlNRTransmissonBandwidth.nRNRB = XNAP_NRNRB_nrb78;
          break;
        case 14:
          servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd->dlNRTransmissonBandwidth.nRNRB = XNAP_NRNRB_nrb106;
          break;
        case 21:
          servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd->dlNRTransmissonBandwidth.nRNRB = XNAP_NRNRB_nrb162;
          break;
        case 24:
          servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd->dlNRTransmissonBandwidth.nRNRB = XNAP_NRNRB_nrb217;
          break;
        case 28:
          servedCellMember->served_cell_info_NR.nrModeInfo.choice.fdd->dlNRTransmissonBandwidth.nRNRB = XNAP_NRNRB_nrb273;
          break;
        default:
          AssertFatal(0, "Failed: Check value for N_RB_DL/N_RB_UL"); // TODO: Add all values or function to convert
          break;
      }
    } else if (resp->info.mode == XNAP_MODE_TDD) { // TDD
      const xnap_tdd_info_t *tdd = &resp->info.tdd;
      servedCellMember->served_cell_info_NR.nrModeInfo.present = XNAP_NRModeInfo_PR_tdd;
      servedCellMember->served_cell_info_NR.nrModeInfo.choice.tdd = (XNAP_NRModeInfoTDD_t *)calloc(1, sizeof(XNAP_NRModeInfoTDD_t));
      servedCellMember->served_cell_info_NR.nrModeInfo.choice.tdd->nrFrequencyInfo.nrARFCN = tdd->freqinfo.arfcn;
      for (int j = 0; j < 1; j++) { // number of available bands = j = 1
        nrfreqbanditem = (XNAP_NRFrequencyBandItem_t *)calloc(1, sizeof(XNAP_NRFrequencyBandItem_t));
        nrfreqbanditem->nr_frequency_band = tdd->freqinfo.band;
        asn1cSeqAdd(&servedCellMember->served_cell_info_NR.nrModeInfo.choice.tdd->nrFrequencyInfo.frequencyBand_List.list,
                    nrfreqbanditem);
      }
      switch (tdd->tbw.scs) {
        case 0:
          servedCellMember->served_cell_info_NR.nrModeInfo.choice.tdd->nrTransmissonBandwidth.nRSCS = XNAP_NRSCS_scs15;
          break;
        case 1:
          servedCellMember->served_cell_info_NR.nrModeInfo.choice.tdd->nrTransmissonBandwidth.nRSCS = XNAP_NRSCS_scs30;
          break;
        case 2:
          servedCellMember->served_cell_info_NR.nrModeInfo.choice.tdd->nrTransmissonBandwidth.nRSCS = XNAP_NRSCS_scs60;
          break;
        case 3:
          servedCellMember->served_cell_info_NR.nrModeInfo.choice.tdd->nrTransmissonBandwidth.nRSCS = XNAP_NRSCS_scs120;
          break;
      }
      switch (tdd->tbw.nrb) {
        case 11:
          servedCellMember->served_cell_info_NR.nrModeInfo.choice.tdd->nrTransmissonBandwidth.nRNRB = XNAP_NRNRB_nrb11;
          break;
        case 18:
          servedCellMember->served_cell_info_NR.nrModeInfo.choice.tdd->nrTransmissonBandwidth.nRNRB = XNAP_NRNRB_nrb18;
          break;
        case 24:
          servedCellMember->served_cell_info_NR.nrModeInfo.choice.tdd->nrTransmissonBandwidth.nRNRB = XNAP_NRNRB_nrb24;
          break;
        case 78:
          servedCellMember->served_cell_info_NR.nrModeInfo.choice.tdd->nrTransmissonBandwidth.nRNRB = XNAP_NRNRB_nrb78;
          break;
        case 106:
          servedCellMember->served_cell_info_NR.nrModeInfo.choice.tdd->nrTransmissonBandwidth.nRNRB = XNAP_NRNRB_nrb106;
          break;
        case 162:
          servedCellMember->served_cell_info_NR.nrModeInfo.choice.tdd->nrTransmissonBandwidth.nRNRB = XNAP_NRNRB_nrb162;
          break;
        case 217:
          servedCellMember->served_cell_info_NR.nrModeInfo.choice.tdd->nrTransmissonBandwidth.nRNRB = XNAP_NRNRB_nrb217;
          break;
        case 273:
          servedCellMember->served_cell_info_NR.nrModeInfo.choice.tdd->nrTransmissonBandwidth.nRNRB = XNAP_NRNRB_nrb273;
          break;
        default:
          AssertFatal(0, "Failed: Check value for N_RB_DL/N_RB_UL");
          break;
      }
    }
    // Setting MTC to 0 now. Will be handled later.
    INT8_TO_OCTET_STRING(0, &servedCellMember->served_cell_info_NR.measurementTimingConfiguration);
    servedCellMember->served_cell_info_NR.connectivitySupport.eNDC_Support = 1;
    asn1cSeqAdd(&ie->value.choice.ServedCells_NR.list, servedCellMember);
  }
  asn1cSeqAdd(&out->protocolIEs.list, ie);

  if (xnap_gNB_encode_pdu(&pdu, &buffer, &len) < 0) {
    LOG_E(XNAP, "Failed to encode Xn setup response\n");
    return -1;
  }
  xnap_gNB_itti_send_sctp_data_req(assoc_id, buffer, len, 0);
  return ret;
}

int xnap_gNB_generate_xn_handover_request(sctp_assoc_t assoc_id, xnap_handover_req_t *xnap_handover_req)
{
  XNAP_XnAP_PDU_t pdu;
  XNAP_HandoverRequest_t *xnhandoverreq;
  XNAP_HandoverRequest_IEs_t *ie;
  XNAP_PDUSessionResourcesToBeSetup_Item_t *pdu_session_resources;
  XNAP_QoSFlowsToBeSetup_Item_t *qos_flows;
  XNAP_LastVisitedCell_Item_t *lastVisitedCell_Item;
  uint8_t *buffer;
  uint32_t len;
  int ret = 0;

  /* Message Type */
  memset(&pdu, 0, sizeof(pdu));
  pdu.present = XNAP_XnAP_PDU_PR_initiatingMessage;
  pdu.choice.initiatingMessage = (XNAP_InitiatingMessage_t *)calloc(1, sizeof(XNAP_InitiatingMessage_t));
  pdu.choice.initiatingMessage->procedureCode = XNAP_ProcedureCode_id_handoverPreparation;
  pdu.choice.initiatingMessage->criticality = XNAP_Criticality_reject;
  pdu.choice.initiatingMessage->value.present = XNAP_InitiatingMessage__value_PR_HandoverRequest;
  xnhandoverreq = &pdu.choice.initiatingMessage->value.choice.HandoverRequest;

  /* mandatory */
  /* Source NG-RAN node UE XnAP ID reference */
  ie = (XNAP_HandoverRequest_IEs_t *)calloc(1, sizeof(XNAP_HandoverRequest_IEs_t));
  ie->id = XNAP_ProtocolIE_ID_id_sourceNG_RANnodeUEXnAPID;
  ie->criticality = XNAP_Criticality_reject;
  ie->value.present = XNAP_HandoverRequest_IEs__value_PR_NG_RANnodeUEXnAPID;
  ie->value.choice.NG_RANnodeUEXnAPID = xnap_handover_req->s_ng_node_ue_xnap_id; //// value to be added.
  asn1cSeqAdd(&xnhandoverreq->protocolIEs.list, ie);

  /* mandatory */
  /* Cause */
  ie = (XNAP_HandoverRequest_IEs_t *)calloc(1, sizeof(XNAP_HandoverRequest_IEs_t));
  ie->id = XNAP_ProtocolIE_ID_id_Cause;
  ie->criticality = XNAP_Criticality_ignore;
  ie->value.present = XNAP_HandoverRequest_IEs__value_PR_Cause;
  ie->value.choice.Cause.present = XNAP_Cause_PR_radioNetwork;
  ie->value.choice.Cause.choice.radioNetwork = 1; // Xnap_CauseRadioNetwork_handover_desirable_for_radio_reasons;
  asn1cSeqAdd(&xnhandoverreq->protocolIEs.list, ie);

  /* mandatory */
  /* Target Cell Global ID */
  ie = (XNAP_HandoverRequest_IEs_t *)calloc(1, sizeof(XNAP_HandoverRequest_IEs_t));
  ie->id = XNAP_ProtocolIE_ID_id_targetCellGlobalID;
  ie->criticality = XNAP_Criticality_reject;
  ie->value.present = XNAP_HandoverRequest_IEs__value_PR_Target_CGI;
  ie->value.choice.Target_CGI.present = XNAP_Target_CGI_PR_nr;
  ie->value.choice.Target_CGI.choice.nr = (XNAP_NR_CGI_t *)calloc(1, sizeof(XNAP_NR_CGI_t));
  MCC_MNC_TO_PLMNID(xnap_handover_req->plmn_id.mcc, /// correct
                    xnap_handover_req->plmn_id.mnc,
                    xnap_handover_req->plmn_id.mnc_digit_length,
                    &ie->value.choice.Target_CGI.choice.nr->plmn_id);
  NR_CELL_ID_TO_BIT_STRING(xnap_handover_req->target_cgi.cgi,
                           &ie->value.choice.Target_CGI.choice.nr->nr_CI); // bit string
  asn1cSeqAdd(&xnhandoverreq->protocolIEs.list, ie);

  /* mandatory */
  /* GUAMI */
  ie = (XNAP_HandoverRequest_IEs_t *)calloc(1, sizeof(XNAP_HandoverRequest_IEs_t));
  ie->id = XNAP_ProtocolIE_ID_id_GUAMI;
  ie->criticality = XNAP_Criticality_reject;
  ie->value.present = XNAP_HandoverRequest_IEs__value_PR_GUAMI;
  MCC_MNC_TO_PLMNID(xnap_handover_req->plmn_id.mcc,
                    xnap_handover_req->plmn_id.mnc,
                    xnap_handover_req->plmn_id.mnc_digit_length,
                    &ie->value.choice.GUAMI.plmn_ID);
  AMF_REGION_TO_BIT_STRING(xnap_handover_req->guami.amf_region_id, &ie->value.choice.GUAMI.amf_region_id);
  AMF_SETID_TO_BIT_STRING(xnap_handover_req->guami.amf_set_id, &ie->value.choice.GUAMI.amf_set_id);
  AMF_POINTER_TO_BIT_STRING(xnap_handover_req->guami.amf_pointer, &ie->value.choice.GUAMI.amf_pointer);
  asn1cSeqAdd(&xnhandoverreq->protocolIEs.list, ie);

  /* mandatory */
  /* UE context Information */
  ie = (XNAP_HandoverRequest_IEs_t *)calloc(1, sizeof(XNAP_HandoverRequest_IEs_t));
  ie->id = XNAP_ProtocolIE_ID_id_UEContextInfoHORequest;
  ie->criticality = XNAP_Criticality_reject;
  ie->value.present = XNAP_HandoverRequest_IEs__value_PR_UEContextInfoHORequest;
  {
    /* NG-C UE associated Signalling reference - AMF UE NGAP ID */
    asn_uint642INTEGER(&ie->value.choice.UEContextInfoHORequest.ng_c_UE_reference, xnap_handover_req->ue_context.ngc_ue_sig_ref);

    /* Signalling TNL association address at source NG-C side - CP Transport Layer Information */
    ie->value.choice.UEContextInfoHORequest.cp_TNL_info_source.present = XNAP_CPTransportLayerInformation_PR_endpointIPAddress;
    TRANSPORT_LAYER_ADDRESS_IPv4_TO_BIT_STRING(
        xnap_handover_req->ue_context.tnl_ip_source,
        &ie->value.choice.UEContextInfoHORequest.cp_TNL_info_source.choice.endpointIPAddress);

    /* AS Security */
    uint8_t AsSecurityKey[32] = {0};
    memcpy(AsSecurityKey,
           xnap_handover_req->ue_context.as_security_key_ranstar,
           sizeof(xnap_handover_req->ue_context.as_security_key_ranstar));
    KENB_STAR_TO_BIT_STRING(xnap_handover_req->ue_context.as_security_key_ranstar,
                            &ie->value.choice.UEContextInfoHORequest.securityInformation.key_NG_RAN_Star);
    if (xnap_handover_req->ue_context.as_security_ncc >= 0) {
      ie->value.choice.UEContextInfoHORequest.securityInformation.ncc = xnap_handover_req->ue_context.as_security_ncc;
    } else {
      ie->value.choice.UEContextInfoHORequest.securityInformation.ncc = 1;
    }

    /* UESecurityCapabilities */
    ENCRALG_TO_BIT_STRING(xnap_handover_req->ue_context.security_capabilities.nRencryption_algorithms,
                          &ie->value.choice.UEContextInfoHORequest.ueSecurityCapabilities.nr_EncyptionAlgorithms);
    INTPROTALG_TO_BIT_STRING(xnap_handover_req->ue_context.security_capabilities.nRintegrity_algorithms,
                             &ie->value.choice.UEContextInfoHORequest.ueSecurityCapabilities.nr_IntegrityProtectionAlgorithms);
    ENCRALG_TO_BIT_STRING(xnap_handover_req->ue_context.security_capabilities.eUTRAencryption_algorithms,
                          &ie->value.choice.UEContextInfoHORequest.ueSecurityCapabilities.e_utra_EncyptionAlgorithms);
    INTPROTALG_TO_BIT_STRING(xnap_handover_req->ue_context.security_capabilities.eUTRAintegrity_algorithms,
                             &ie->value.choice.UEContextInfoHORequest.ueSecurityCapabilities.e_utra_IntegrityProtectionAlgorithms);

    /* RRC Context */
    OCTET_STRING_fromBuf(&ie->value.choice.UEContextInfoHORequest.rrc_Context,
                         (char *)xnap_handover_req->ue_context.rrc_buffer,
                         xnap_handover_req->ue_context.rrc_buffer_size);

    /* UE AMBR */
    UEAGMAXBITRTD_TO_ASN_PRIMITIVES(xnap_handover_req->ue_context.ue_ambr.br_dl,
                                    &ie->value.choice.UEContextInfoHORequest.ue_AMBR.dl_UE_AMBR);
    UEAGMAXBITRTU_TO_ASN_PRIMITIVES(xnap_handover_req->ue_context.ue_ambr.br_ul,
                                    &ie->value.choice.UEContextInfoHORequest.ue_AMBR.ul_UE_AMBR);

    /* PDU session resources to be setup list */
    for (int i = 0; i < xnap_handover_req->ue_context.pdusession_tobe_setup_list.num_pdu; i++) {
      pdu_session_resources =
          (XNAP_PDUSessionResourcesToBeSetup_Item_t *)calloc(1, sizeof(XNAP_PDUSessionResourcesToBeSetup_Item_t));
      /* PDU Session id */
      pdu_session_resources->pduSessionId = xnap_handover_req->ue_context.pdusession_tobe_setup_list.pdu[i].pdusession_id;
      /* SSNSAI */
      INT8_TO_OCTET_STRING(xnap_handover_req->ue_context.pdusession_tobe_setup_list.pdu[i].snssai.sst,
                           &pdu_session_resources->s_NSSAI.sst);
      /* UP TNL Information */
      pdu_session_resources->uL_NG_U_TNLatUPF.present = XNAP_UPTransportLayerInformation_PR_gtpTunnel;
      pdu_session_resources->uL_NG_U_TNLatUPF.choice.gtpTunnel =
          (XNAP_GTPtunnelTransportLayerInformation_t *)calloc(1, sizeof(XNAP_GTPtunnelTransportLayerInformation_t));
      TRANSPORT_LAYER_ADDRESS_IPv4_TO_BIT_STRING(xnap_handover_req->ue_context.tnl_ip_source,
                                                 &pdu_session_resources->uL_NG_U_TNLatUPF.choice.gtpTunnel->tnl_address);
      INT32_TO_OCTET_STRING(xnap_handover_req->ue_context.pdusession_tobe_setup_list.pdu[i].up_ngu_tnl_teid_upf,
                            &pdu_session_resources->uL_NG_U_TNLatUPF.choice.gtpTunnel->gtp_teid);
      /* PDU session type */
      pdu_session_resources->pduSessionType = xnap_handover_req->ue_context.pdusession_tobe_setup_list.pdu[i].pdu_session_type;
      /* QOS flows to be setup */
      {
        for (int j = 0; j < xnap_handover_req->ue_context.pdusession_tobe_setup_list.pdu[i].qos_list.num_qos; j++) {
          qos_flows = (XNAP_QoSFlowsToBeSetup_Item_t *)calloc(1, sizeof(XNAP_QoSFlowsToBeSetup_Item_t));
          /* QFI */
          qos_flows->qfi = xnap_handover_req->ue_context.pdusession_tobe_setup_list.pdu[i].qos_list.qos[j].qfi;
          /* QOS flow level QOS parameters */
          /* non-dynamic */
          qos_flows->qosFlowLevelQoSParameters.qos_characteristics.present = XNAP_QoSCharacteristics_PR_non_dynamic;
          qos_flows->qosFlowLevelQoSParameters.qos_characteristics.choice.non_dynamic =
              (XNAP_NonDynamic5QIDescriptor_t *)calloc(1, sizeof(XNAP_NonDynamic5QIDescriptor_t));
          qos_flows->qosFlowLevelQoSParameters.qos_characteristics.choice.non_dynamic->fiveQI =
              xnap_handover_req->ue_context.pdusession_tobe_setup_list.pdu[i].qos_list.qos[j].qos_params.non_dynamic.fiveqi;
          /* dynamic */
          qos_flows->qosFlowLevelQoSParameters.qos_characteristics.present = XNAP_QoSCharacteristics_PR_dynamic;
          qos_flows->qosFlowLevelQoSParameters.qos_characteristics.choice.dynamic =
              (XNAP_Dynamic5QIDescriptor_t *)calloc(1, sizeof(XNAP_Dynamic5QIDescriptor_t));
          qos_flows->qosFlowLevelQoSParameters.qos_characteristics.choice.dynamic->fiveQI =
              &xnap_handover_req->ue_context.pdusession_tobe_setup_list.pdu[i].qos_list.qos[j].qos_params.dynamic.fiveqi;
          qos_flows->qosFlowLevelQoSParameters.qos_characteristics.choice.dynamic->priorityLevelQoS =
              xnap_handover_req->ue_context.pdusession_tobe_setup_list.pdu[i].qos_list.qos[j].qos_params.dynamic.qos_priority_level;
          qos_flows->qosFlowLevelQoSParameters.qos_characteristics.choice.dynamic->packetDelayBudget =
              xnap_handover_req->ue_context.pdusession_tobe_setup_list.pdu[i]
                  .qos_list.qos[j]
                  .qos_params.dynamic.packet_delay_budget;
          qos_flows->qosFlowLevelQoSParameters.qos_characteristics.choice.dynamic->packetErrorRate.pER_Scalar =
              xnap_handover_req->ue_context.pdusession_tobe_setup_list.pdu[i]
                  .qos_list.qos[j]
                  .qos_params.dynamic.packet_error_rate.per_scalar;
          qos_flows->qosFlowLevelQoSParameters.qos_characteristics.choice.dynamic->packetErrorRate.pER_Exponent =
              xnap_handover_req->ue_context.pdusession_tobe_setup_list.pdu[i]
                  .qos_list.qos[j]
                  .qos_params.dynamic.packet_error_rate.per_exponent;
          asn1cSeqAdd(&pdu_session_resources->qosFlowsToBeSetup_List.list, qos_flows);
        }
      }
      asn1cSeqAdd(&ie->value.choice.UEContextInfoHORequest.pduSessionResourcesToBeSetup_List.list, pdu_session_resources);
    }
  }
  asn1cSeqAdd(&xnhandoverreq->protocolIEs.list, ie);

  /* mandatory */
  /* UE History Information */
  ie = (XNAP_HandoverRequest_IEs_t *)calloc(1, sizeof(XNAP_HandoverRequest_IEs_t));
  ie->id = XNAP_ProtocolIE_ID_id_UEHistoryInformation;
  ie->criticality = XNAP_Criticality_ignore;
  ie->value.present = XNAP_HandoverRequest_IEs__value_PR_UEHistoryInformation;
  {
    lastVisitedCell_Item = (XNAP_LastVisitedCell_Item_t *)calloc(1, sizeof(XNAP_LastVisitedCell_Item_t));
    lastVisitedCell_Item->present = XNAP_LastVisitedCell_Item_PR_nG_RAN_Cell;
    INT32_TO_OCTET_STRING(xnap_handover_req->uehistory_info.last_visited_cgi.cgi, &lastVisitedCell_Item->choice.nG_RAN_Cell);
    asn1cSeqAdd(&ie->value.choice.UEHistoryInformation.list, lastVisitedCell_Item);
  }
  asn1cSeqAdd(&xnhandoverreq->protocolIEs.list, ie);

  if (xnap_gNB_encode_pdu(&pdu, &buffer, &len) < 0) {
    LOG_E(XNAP, "Failed to encode XN handover request\n");
    return -1;
  }
  xnap_gNB_itti_send_sctp_data_req(assoc_id, buffer, len, 0);
  return ret;
}

int xnap_gNB_generate_xn_handover_request_ack(sctp_assoc_t assoc_id,
                                              xnap_handover_req_ack_t *xnap_handover_req_ack,
                                              instance_t instance)
{
  XNAP_XnAP_PDU_t pdu;
  XNAP_HandoverRequestAcknowledge_t *xnhandoverreqAck;
  XNAP_HandoverRequestAcknowledge_IEs_t *ie;
  XNAP_PDUSessionResourcesAdmitted_Item_t *pdu_session_resources;
  XNAP_QoSFlowsAdmitted_Item_t *qos_flows;
  uint8_t *buffer;
  uint32_t len;
  int ret = 0;

  /* Message Type */
  memset(&pdu, 0, sizeof(pdu));
  pdu.present = XNAP_XnAP_PDU_PR_successfulOutcome;
  pdu.choice.successfulOutcome = (XNAP_SuccessfulOutcome_t *)calloc(1, sizeof(XNAP_SuccessfulOutcome_t));
  pdu.choice.successfulOutcome->procedureCode = XNAP_ProcedureCode_id_handoverPreparation;
  pdu.choice.successfulOutcome->criticality = XNAP_Criticality_reject;
  pdu.choice.successfulOutcome->value.present = XNAP_SuccessfulOutcome__value_PR_HandoverRequestAcknowledge;
  xnhandoverreqAck = &pdu.choice.successfulOutcome->value.choice.HandoverRequestAcknowledge;

  /* mandatory */
  /* Source NG-RAN node UE XnAP ID */
  ie = (XNAP_HandoverRequestAcknowledge_IEs_t *)calloc(1, sizeof(XNAP_HandoverRequestAcknowledge_IEs_t));
  ie->id = XNAP_ProtocolIE_ID_id_sourceNG_RANnodeUEXnAPID;
  ie->criticality = XNAP_Criticality_ignore;
  ie->value.present = XNAP_HandoverRequest_IEs__value_PR_NG_RANnodeUEXnAPID;
  ie->value.choice.NG_RANnodeUEXnAPID = xnap_handover_req_ack->s_ng_node_ue_xnap_id; // id_source;
  asn1cSeqAdd(&xnhandoverreqAck->protocolIEs.list, ie);

  /* mandatory */
  /* Target NG-RAN node UE XnAP ID */
  ie = (XNAP_HandoverRequestAcknowledge_IEs_t *)calloc(1, sizeof(XNAP_HandoverRequestAcknowledge_IEs_t));
  ie->id = XNAP_ProtocolIE_ID_id_targetNG_RANnodeUEXnAPID;
  ie->criticality = XNAP_Criticality_ignore;
  ie->value.present = XNAP_HandoverRequestAcknowledge_IEs__value_PR_NG_RANnodeUEXnAPID_1;
  ie->value.choice.NG_RANnodeUEXnAPID_1 = xnap_handover_req_ack->t_ng_node_ue_xnap_id; // id_target;
  asn1cSeqAdd(&xnhandoverreqAck->protocolIEs.list, ie);

  /* mandatory */
  /* PDU Session Resources Admitted List */
  ie = (XNAP_HandoverRequestAcknowledge_IEs_t *)calloc(1, sizeof(XNAP_HandoverRequestAcknowledge_IEs_t));
  ie->id = XNAP_ProtocolIE_ID_id_PDUSessionResourcesAdmitted_List;
  ie->criticality = XNAP_Criticality_ignore;
  ie->value.present = XNAP_HandoverRequestAcknowledge_IEs__value_PR_PDUSessionResourcesAdmitted_List;
  {
    for (int i = 0; i < xnap_handover_req_ack->ue_context.pdusession_admitted_list.num_pdu; i++) {
      pdu_session_resources = (XNAP_PDUSessionResourcesAdmitted_Item_t *)calloc(1, sizeof(XNAP_PDUSessionResourcesAdmitted_Item_t));
      /* PDU Session id */
      pdu_session_resources->pduSessionId = xnap_handover_req_ack->ue_context.pdusession_admitted_list.pdu[i].pdusession_id;
      /* QOS flows to be setup */
      for (int j = 0; j < xnap_handover_req_ack->ue_context.pdusession_admitted_list.pdu[i].qos_list.num_qos; j++) {
        qos_flows = (XNAP_QoSFlowsAdmitted_Item_t *)calloc(1, sizeof(XNAP_QoSFlowsAdmitted_Item_t));
        /* QFI */
        qos_flows->qfi = xnap_handover_req_ack->ue_context.pdusession_admitted_list.pdu[i].qos_list.qos[j].qfi;
        asn1cSeqAdd(&pdu_session_resources->pduSessionResourceAdmittedInfo.qosFlowsAdmitted_List.list, qos_flows);
      }
      asn1cSeqAdd(&ie->value.choice.PDUSessionResourcesAdmitted_List.list, pdu_session_resources);
    }
  }
  asn1cSeqAdd(&xnhandoverreqAck->protocolIEs.list, ie);

  /* mandatory */
  /* Target NG-RAN node To Source NG-RAN node Transparent Container */
  ie = (XNAP_HandoverRequestAcknowledge_IEs_t *)calloc(1, sizeof(XNAP_HandoverRequestAcknowledge_IEs_t));
  ie->id = XNAP_ProtocolIE_ID_id_Target2SourceNG_RANnodeTranspContainer;
  ie->criticality = XNAP_Criticality_ignore;
  ie->value.present = XNAP_HandoverRequestAcknowledge_IEs__value_PR_OCTET_STRING;
  OCTET_STRING_fromBuf(&ie->value.choice.OCTET_STRING,
                       (char *)xnap_handover_req_ack->rrc_buffer,
                       xnap_handover_req_ack->rrc_buffer_size);
  asn1cSeqAdd(&xnhandoverreqAck->protocolIEs.list, ie);

  if (xnap_gNB_encode_pdu(&pdu, &buffer, &len) < 0) {
    LOG_E(XNAP, "Failed to encode XN handover request ack\n");
    return -1;
  }
  xnap_gNB_itti_send_sctp_data_req(assoc_id, buffer, len, 0);
  return ret;
}

int xnap_gNB_generate_xn_ue_context_release(xnap_gNB_instance_t *instance_p, xnap_ue_context_release_t *xnap_ue_context_release)
{
  XNAP_XnAP_PDU_t pdu;
  XNAP_UEContextRelease_t *in;
  XNAP_UEContextRelease_IEs_t *ie;
  uint8_t *buffer;
  uint32_t len;
  DevAssert(instance_p != NULL);

  /* Message Type */
  memset(&pdu, 0, sizeof(pdu));
  pdu.present = XNAP_XnAP_PDU_PR_initiatingMessage;
  pdu.choice.initiatingMessage = (XNAP_InitiatingMessage_t *)calloc(1, sizeof(XNAP_InitiatingMessage_t));
  pdu.choice.initiatingMessage->procedureCode = XNAP_ProcedureCode_id_uEContextRelease;
  pdu.choice.initiatingMessage->criticality = XNAP_Criticality_reject;
  pdu.choice.initiatingMessage->value.present = XNAP_InitiatingMessage__value_PR_UEContextRelease;
  in = &pdu.choice.initiatingMessage->value.choice.UEContextRelease;

  /* mandatory */
  /* Source NG-RAN node UE XnAP ID */
  ie = (XNAP_UEContextRelease_IEs_t *)calloc(1, sizeof(XNAP_UEContextRelease_IEs_t));
  ie->id = XNAP_ProtocolIE_ID_id_sourceNG_RANnodeUEXnAPID;
  ie->criticality = XNAP_Criticality_reject;
  ie->value.present = XNAP_UEContextRelease_IEs__value_PR_NG_RANnodeUEXnAPID;
  ie->value.choice.NG_RANnodeUEXnAPID = xnap_ue_context_release->s_ng_node_ue_xnap_id;
  asn1cSeqAdd(&in->protocolIEs.list, ie);

  /* mandatory */
  /* Target NG-RAN node UE XnAP ID */
  ie = (XNAP_UEContextRelease_IEs_t *)calloc(1, sizeof(XNAP_UEContextRelease_IEs_t));
  ie->id = XNAP_ProtocolIE_ID_id_targetNG_RANnodeUEXnAPID;
  ie->criticality = XNAP_Criticality_reject;
  ie->value.present = XNAP_UEContextRelease_IEs__value_PR_NG_RANnodeUEXnAPID_1;
  ie->value.choice.NG_RANnodeUEXnAPID_1 = xnap_ue_context_release->t_ng_node_ue_xnap_id;
  asn1cSeqAdd(&in->protocolIEs.list, ie);

  if (xnap_gNB_encode_pdu(&pdu, &buffer, &len) < 0) {
    LOG_E(XNAP, "Failed to encode UE context release\n");
    return -1;
  }
  LOG_I(XNAP, "Sending UE CONTEXT RELEASE from target to source\n");
  xnap_gNB_itti_send_sctp_data_req(xnap_ue_context_release->assoc_id, buffer, len, 0);
  return 0;
}
