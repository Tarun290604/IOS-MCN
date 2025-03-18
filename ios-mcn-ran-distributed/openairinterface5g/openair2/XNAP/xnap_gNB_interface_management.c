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

/*! \file xnap_gNB_interface_management.c
 * \brief xnap handling interface procedures for gNB
 * \author Sreeshma Shiv <sreeshmau@iisc.ac.in>
 * \date Dec 2023
 * \version 1.0
 */

#include <stdint.h>
#include "intertask_interface.h"
#include "xnap_common.h"
#include "xnap_gNB_defs.h"
#include "xnap_gNB_interface_management.h"
#include "xnap_gNB_handler.h"
#include "assertions.h"
#include "conversions.h"
#include "XNAP_GlobalgNB-ID.h"
#include "XNAP_ServedCells-NR-Item.h"
#include "XNAP_NRFrequencyBandItem.h"
#include "XNAP_GlobalNG-RANNode-ID.h"
#include "XNAP_NRModeInfoFDD.h"
#include "XNAP_NRModeInfoTDD.h"
#include "XNAP_SupportedSULBandList.h"
#include "XNAP_TAISupport-Item.h"
#include "XNAP_BroadcastPLMNinTAISupport-Item.h"
#include "xnap_gNB_management_procedures.h"
#include "XNAP_PDUSessionResourcesToBeSetup-Item.h"
#include "XNAP_QoSFlowsToBeSetup-Item.h"
#include "XNAP_GTPtunnelTransportLayerInformation.h"
#include "XNAP_NonDynamic5QIDescriptor.h"
#include "XNAP_Dynamic5QIDescriptor.h"
#include "XNAP_LastVisitedCell-Item.h"
#include "XNAP_QoSFlowsAdmitted-Item.h"
#include "XNAP_PDUSessionResourcesAdmitted-Item.h"

int xnap_gNB_handle_xn_setup_request(instance_t instance, sctp_assoc_t assoc_id, uint32_t stream, XNAP_XnAP_PDU_t *pdu)
{
  XNAP_XnSetupRequest_t *xnSetupRequest;
  XNAP_XnSetupRequest_IEs_t *ie;

  DevAssert(pdu != NULL);
  xnSetupRequest = &pdu->choice.initiatingMessage->value.choice.XnSetupRequest;
  if (stream != 0) { /* Xn Setup: Non UE related procedure ->stream 0 */
    LOG_E(XNAP, "Received new XN setup request on stream != 0\n");
    /* Send a xn setup failure with protocol cause unspecified */
    MessageDef *message_p = itti_alloc_new_message(TASK_XNAP, 0, XNAP_SETUP_FAILURE);
    message_p->ittiMsgHeader.originInstance = assoc_id;
    xnap_setup_failure_t *fail = &XNAP_SETUP_FAILURE(message_p);
    fail->cause_type = XNAP_CAUSE_PROTOCOL;
    fail->cause_value = 6;
    itti_send_msg_to_task(TASK_XNAP, 0, message_p);
  }
  MessageDef *message_p = itti_alloc_new_message(TASK_XNAP, 0, XNAP_SETUP_REQ);
  message_p->ittiMsgHeader.originInstance = assoc_id;
  xnap_setup_req_t *req = &XNAP_SETUP_REQ(message_p);

  /* Global NG-RAN Node ID */
  XNAP_FIND_PROTOCOLIE_BY_ID(XNAP_XnSetupRequest_IEs_t, ie, xnSetupRequest, XNAP_ProtocolIE_ID_id_GlobalNG_RAN_node_ID, true);
  if (ie == NULL) {
    LOG_E(XNAP, "XNAP_ProtocolIE_ID_id_GlobalNG_RAN_node_ID is NULL pointer \n");
    return -1;
  } else {
    if (ie->value.choice.GlobalNG_RANNode_ID.choice.gNB->gnb_id.present == XNAP_GNB_ID_Choice_PR_gnb_ID) {
      uint8_t *gNB_id_buf = ie->value.choice.GlobalNG_RANNode_ID.choice.gNB->gnb_id.choice.gnb_ID.buf;
      if (ie->value.choice.GlobalNG_RANNode_ID.choice.gNB->gnb_id.choice.gnb_ID.size != 28) {
        // TODO: handle case where size != 28 -> notify ? reject ?
      }
      req->gNB_id = (gNB_id_buf[0] << 20) + (gNB_id_buf[1] << 12) + (gNB_id_buf[2] << 4) + ((gNB_id_buf[3] & 0xf0) >> 4);
    } else {
      // TODO if NSA setup
    }
  }
  LOG_D(XNAP, "Adding gNB to the list of associated gNBs: %lu\n", req->gNB_id);

  /* TAI Support list */
  XNAP_FIND_PROTOCOLIE_BY_ID(XNAP_XnSetupRequest_IEs_t, ie, xnSetupRequest, XNAP_ProtocolIE_ID_id_TAISupport_list, true);
  if (ie == NULL) {
    LOG_E(XNAP, "XNAP_ProtocolIE_ID_id_TAISupport_list is NULL pointer \n");
    return -1;
  } else {
    OCTET_STRING_TO_INT24(&ie->value.choice.TAISupport_List.list.array[0]->tac, req->tai_support);
    LOG_I(XNAP, "tac %d \n", req->tai_support);
  }
  LOG_D(XNAP, "req->gNB id: %lu \n", req->gNB_id);
  LOG_D(XNAP, "Adding gNB to the list of associated gNBs\n");

  /* List of Served Cells NR */
  XNAP_FIND_PROTOCOLIE_BY_ID(XNAP_XnSetupRequest_IEs_t, ie, xnSetupRequest, XNAP_ProtocolIE_ID_id_List_of_served_cells_NR, true);
  req->num_cells_available = ie->value.choice.ServedCells_NR.list.count;
  LOG_D(XNAP, "req->num_cells_available %d \n", req->num_cells_available);
  for (int i = 0; i < req->num_cells_available; i++) {
    XNAP_ServedCellInformation_NR_t *servedCellMember =
        &(((XNAP_ServedCells_NR_Item_t *)ie->value.choice.ServedCells_NR.list.array[i])->served_cell_info_NR);
    req->info.nr_pci = servedCellMember->nrPCI;
    LOG_D(XNAP, "req->nr_pci[%d] %d \n", i, req->info.nr_pci);
    PLMNID_TO_MCC_MNC(servedCellMember->broadcastPLMN.list.array[0],
                      req->info.plmn.mcc,
                      req->info.plmn.mnc,
                      req->info.plmn.mnc_digit_length);
    BIT_STRING_TO_NR_CELL_IDENTITY(&servedCellMember->cellID.nr_CI, req->info.nr_cellid);
    LOG_D(XNAP,
          "[SCTP %d] Received BroadcastPLMN: MCC %d, MNC %d, CELL_ID %llu\n",
          assoc_id,
          req->info.plmn.mcc,
          req->info.plmn.mnc,
          (long long unsigned int)req->info.nr_cellid);
    // FDD Cells
    if (servedCellMember->nrModeInfo.present == XNAP_NRModeInfo_PR_fdd) {
      req->info.mode = XNAP_MODE_FDD;
      xnap_fdd_info_t *FDDs = &req->info.fdd;
      XNAP_NRModeInfoFDD_t *fdd_Info = servedCellMember->nrModeInfo.choice.fdd;
      FDDs->ul_freqinfo.arfcn = fdd_Info->ulNRFrequencyInfo.nrARFCN;
      AssertFatal(fdd_Info->ulNRFrequencyInfo.frequencyBand_List.list.count == 1, "cannot handle more than one frequency band\n");
      for (int f = 0; f < fdd_Info->ulNRFrequencyInfo.frequencyBand_List.list.count; f++) {
        XNAP_NRFrequencyBandItem_t *FreqItem = fdd_Info->ulNRFrequencyInfo.frequencyBand_List.list.array[f];
        FDDs->ul_freqinfo.band = FreqItem->nr_frequency_band;
        AssertFatal(FreqItem->supported_SUL_Band_List->list.count == 0, "cannot handle SUL bands!\n");
      }
      FDDs->dl_freqinfo.arfcn = fdd_Info->dlNRFrequencyInfo.nrARFCN;
      int dlBands = fdd_Info->dlNRFrequencyInfo.frequencyBand_List.list.count;
      AssertFatal(dlBands == 0, "cannot handle more than one frequency band\n");
      for (int dlB = 0; dlB < dlBands; dlB++) {
        XNAP_NRFrequencyBandItem_t *FreqItem = fdd_Info->dlNRFrequencyInfo.frequencyBand_List.list.array[dlB];
        FDDs->dl_freqinfo.band = FreqItem->nr_frequency_band;
        int num_available_supported_SULBands = FreqItem->supported_SUL_Band_List->list.count;
        AssertFatal(num_available_supported_SULBands == 0, "cannot handle SUL bands!\n");
      }
      FDDs->ul_tbw.scs = fdd_Info->ulNRTransmissonBandwidth.nRSCS;
      FDDs->ul_tbw.nrb = fdd_Info->ulNRTransmissonBandwidth.nRNRB;
      FDDs->dl_tbw.scs = fdd_Info->dlNRTransmissonBandwidth.nRSCS;
      FDDs->dl_tbw.nrb = fdd_Info->dlNRTransmissonBandwidth.nRNRB;
    } else if (servedCellMember->nrModeInfo.present == XNAP_NRModeInfo_PR_tdd) {
      req->info.mode = XNAP_MODE_TDD;
      xnap_tdd_info_t *TDDs = &req->info.tdd;
      XNAP_NRModeInfoTDD_t *tdd_Info = servedCellMember->nrModeInfo.choice.tdd;
      TDDs->freqinfo.arfcn = tdd_Info->nrFrequencyInfo.nrARFCN;
      AssertFatal(tdd_Info->nrFrequencyInfo.frequencyBand_List.list.count == 1, "cannot handle more than one frequency band\n");
      for (int f = 0; f < tdd_Info->nrFrequencyInfo.frequencyBand_List.list.count; f++) {
        XNAP_NRFrequencyBandItem_t *FreqItem = tdd_Info->nrFrequencyInfo.frequencyBand_List.list.array[f];
        TDDs->freqinfo.band = FreqItem->nr_frequency_band;
      }
      TDDs->tbw.scs = tdd_Info->nrTransmissonBandwidth.nRSCS;
      TDDs->tbw.nrb = tdd_Info->nrTransmissonBandwidth.nRNRB;
    } else {
      AssertFatal(false, "unknown NR Mode info \n");
    }
  }

  itti_send_msg_to_task(TASK_RRC_GNB, instance, message_p);
  return 0;
}

int xnap_gNB_handle_xn_setup_response(instance_t instance, sctp_assoc_t assoc_id, uint32_t stream, XNAP_XnAP_PDU_t *pdu)
{
  AssertFatal(pdu->present == XNAP_XnAP_PDU_PR_successfulOutcome, "pdu->present != XNAP_XnAP_PDU_PR_successfulOutcome,\n");
  AssertFatal(pdu->choice.successfulOutcome->procedureCode == XNAP_ProcedureCode_id_xnSetup,
              "pdu->choice.successfulOutcome->procedureCode != XNAP_ProcedureCode_id_xnSetup\n");
  AssertFatal(pdu->choice.successfulOutcome->criticality == XNAP_Criticality_reject,
              "pdu->choice.successfulOutcome->criticality != XNAP_Criticality_reject\n");
  AssertFatal(pdu->choice.successfulOutcome->value.present == XNAP_SuccessfulOutcome__value_PR_XnSetupResponse,
              "pdu->choice.successfulOutcome->value.present != XNAP_SuccessfulOutcome__value_PR_XnSetupResponse\n");

  XNAP_XnSetupResponse_t *xnSetupResponse = &pdu->choice.successfulOutcome->value.choice.XnSetupResponse;
  XNAP_XnSetupResponse_IEs_t *ie;
  uint32_t gNB_id = 0;
  MessageDef *msg = itti_alloc_new_message(TASK_XNAP, 0, XNAP_SETUP_RESP);
  msg->ittiMsgHeader.originInstance = assoc_id;
  xnap_setup_resp_t *resp = &XNAP_SETUP_RESP(msg);
  xnap_gNB_instance_t *instance_p = xnap_gNB_get_instance(instance);
  xnap_gNB_data_t *xnap_gnb_data_p = xnap_get_gNB(instance, assoc_id);
  for (int i = 0; i < xnSetupResponse->protocolIEs.list.count; i++) {
    ie = xnSetupResponse->protocolIEs.list.array[i];
    switch (ie->id) {
      case XNAP_ProtocolIE_ID_id_GlobalNG_RAN_node_ID: /* Global NG-RAN Node ID */
        AssertFatal(ie->criticality == XNAP_Criticality_reject, "ie->criticality != XNAP_Criticality_reject\n");
        AssertFatal(ie->value.present == XNAP_XnSetupResponse_IEs__value_PR_GlobalNG_RANNode_ID,
                    "ie->value.present != XNAP_XnSetupResponse_IEs__value_PR_GlobalNG_RANNode_ID\n");
        uint8_t *gNB_id_buf = ie->value.choice.GlobalNG_RANNode_ID.choice.gNB->gnb_id.choice.gnb_ID.buf;
        gNB_id = (gNB_id_buf[0] << 20) + (gNB_id_buf[1] << 12) + (gNB_id_buf[2] << 4) + ((gNB_id_buf[3] & 0xf0) >> 4);
        LOG_D(XNAP, "Connected gNB id: %07x\n", gNB_id);
        LOG_D(XNAP, "Adding gNB to the list of associated gNBs\n");
        xnap_gnb_data_p->state = XNAP_GNB_STATE_CONNECTED;
        xnap_gnb_data_p->gNB_id = gNB_id;
        break;
      case XNAP_ProtocolIE_ID_id_TAISupport_list: /* TAI Support List */
        AssertFatal(ie->criticality == XNAP_Criticality_reject, "ie->criticality != XNAP_Criticality_reject\n");
        AssertFatal(ie->value.present == XNAP_XnSetupResponse_IEs__value_PR_TAISupport_List,
                    "ie->value.present != XNAP_XnSetupResponse_IEs__value_PR_TAISupport_List\n");
        PLMNID_TO_MCC_MNC(&ie->value.choice.TAISupport_List.list.array[0]->broadcastPLMNs.list.array[0]->plmn_id,
                          resp->info.plmn.mcc,
                          resp->info.plmn.mnc,
                          resp->info.plmn.mnc_digit_length);
        break;
    }
  }

  /* List of Served Cells NR */
  XNAP_FIND_PROTOCOLIE_BY_ID(XNAP_XnSetupResponse_IEs_t, ie, xnSetupResponse, XNAP_ProtocolIE_ID_id_List_of_served_cells_NR, true);
  if (ie == NULL) {
    LOG_E(XNAP, "%s %d: ie is a NULL pointer \n", __FILE__, __LINE__);
    return -1;
  } else {
    resp->nb_xn = ie->value.choice.ServedCells_NR.list.count;
    LOG_D(XNAP, "resp->nb_xn %d \n", resp->nb_xn);
    for (int i = 0; i < resp->nb_xn; i++) {
      XNAP_ServedCellInformation_NR_t *servedCellMember =
          &(((XNAP_ServedCells_NR_Item_t *)ie->value.choice.ServedCells_NR.list.array[i])->served_cell_info_NR);
      resp->info.nr_pci = servedCellMember->nrPCI;
      LOG_D(XNAP, "resp->nr_pci[%d] %d \n", i, resp->info.nr_pci);
      PLMNID_TO_MCC_MNC(servedCellMember->broadcastPLMN.list.array[0],
                        resp->info.plmn.mcc,
                        resp->info.plmn.mnc,
                        resp->info.plmn.mnc_digit_length);
      BIT_STRING_TO_NR_CELL_IDENTITY(&servedCellMember->cellID.nr_CI, resp->info.nr_cellid);
      LOG_D(XNAP,
            "[SCTP %d] Received BroadcastPLMN: MCC %d, MNC %d, CELL_ID %llu\n",
            assoc_id,
            resp->info.plmn.mcc,
            resp->info.plmn.mnc,
            (long long unsigned int)resp->info.nr_cellid);
    }
  }
  instance_p->xn_target_gnb_associated_nb++;

  itti_send_msg_to_task(TASK_RRC_GNB, instance_p->instance, msg);
  return 0;
}

int xnap_gNB_handle_xn_setup_failure(instance_t instance, sctp_assoc_t assoc_id, uint32_t stream, XNAP_XnAP_PDU_t *pdu)
{
  DevAssert(pdu != NULL);
  XNAP_XnSetupFailure_t *xnSetupFailure;
  XNAP_XnSetupFailure_IEs_t *ie;
  xnap_gNB_data_t *xnap_gNB_data;

  xnSetupFailure = &pdu->choice.unsuccessfulOutcome->value.choice.XnSetupFailure;
  /*
   * We received a new valid XN Setup Failure on a stream != 0.
   * * * * This should not happen -> reject gNB xn setup failure.
   */
  if (stream != 0) {
    LOG_W(XNAP, "[SCTP %d] Received xn setup failure on stream != 0 (%d)\n", assoc_id, stream);
  }
  if ((xnap_gNB_data = xnap_get_gNB(instance, assoc_id)) == NULL) {
    LOG_E(XNAP,
          "[SCTP %d] Received XN setup failure for non existing "
          "gNB context\n",
          assoc_id);
    return -1;
  }
  if ((xnap_gNB_data->state == XNAP_GNB_STATE_CONNECTED) || (xnap_gNB_data->state == XNAP_GNB_STATE_READY)) {
    LOG_E(XNAP, "Received Unexpexted XN Setup Failure Message\n");
    return -1;
  }

  /* Cause */
  XNAP_FIND_PROTOCOLIE_BY_ID(XNAP_XnSetupFailure_IEs_t, ie, xnSetupFailure, XNAP_ProtocolIE_ID_id_Cause, true);
  if (ie == NULL) {
    LOG_E(XNAP, "%s %d: ie is a NULL pointer \n", __FILE__, __LINE__);
    return -1;
  }
  if ((ie->value.choice.Cause.present == XNAP_Cause_PR_misc)
      && (ie->value.choice.Cause.choice.misc == XNAP_CauseMisc_unspecified)) {
    LOG_E(XNAP, "Received XN setup failure for gNB ... gNB is not ready\n");
    exit(1);
  } else {
    LOG_E(XNAP, "Received xn setup failure for gNB... please check your parameters\n");
    exit(1);
  }
  xnap_gNB_data->state = XNAP_GNB_STATE_WAITING;
  xnap_handle_xn_setup_message(instance, assoc_id, 0);
  return 0;
}

int xnap_gNB_handle_handover_request(instance_t instance, sctp_assoc_t assoc_id, uint32_t stream, XNAP_XnAP_PDU_t *pdu)
{
  XNAP_HandoverRequest_t *xnHandoverRequest;
  XNAP_HandoverRequest_IEs_t *ie;
  xnap_gNB_instance_t *instance_p;
  xnap_id_manager *id_manager;
  int xn_id;
  XNAP_PDUSessionResourcesToBeSetup_Item_t *pdu_session_resources;
  XNAP_QoSFlowsToBeSetup_Item_t *qos_flows;
  XNAP_LastVisitedCell_Item_t *lastVisitedCell_Item;
  instance_p = xnap_gNB_get_instance(instance);
  updateXninst(0, NULL, NULL, assoc_id);

  DevAssert(pdu != NULL);
  xnHandoverRequest = &pdu->choice.initiatingMessage->value.choice.HandoverRequest;
  if (stream != 0) {
    LOG_E(XNAP, "Received new XN handover request on stream != 0\n");
    // sending handover failed
    MessageDef *message_p = itti_alloc_new_message(TASK_XNAP, 0, XNAP_HANDOVER_REQ_FAILURE);
    message_p->ittiMsgHeader.originInstance = assoc_id;
    xnap_handover_req_failure_t *fail = &XNAP_HANDOVER_REQ_FAILURE(message_p);
    fail->cause_type = XNAP_CAUSE_PROTOCOL;
    fail->cause_value = 6;
    itti_send_msg_to_task(TASK_XNAP, 0, message_p);
  }

  MessageDef *message_p = itti_alloc_new_message(TASK_XNAP, 0, XNAP_HANDOVER_REQ);
  message_p->ittiMsgHeader.originInstance = assoc_id;
  xnap_handover_req_t *req = &XNAP_HANDOVER_REQ(message_p);

  /* Source NG-RAN node UE XnAP ID reference */
  XNAP_FIND_PROTOCOLIE_BY_ID(XNAP_HandoverRequest_IEs_t,
                             ie,
                             xnHandoverRequest,
                             XNAP_ProtocolIE_ID_id_sourceNG_RANnodeUEXnAPID,
                             true);
  if (ie == NULL) {
    LOG_E(XNAP, "XNAP_ProtocolIE_ID_id_sourceNG_RANnodeUEXnAPID, is NULL pointer \n");
    return -1;
  } else {
    req->s_ng_node_ue_xnap_id = ie->value.choice.NG_RANnodeUEXnAPID;
  }
  id_manager = &instance_p->id_manager;
  xnap_id_manager_init(id_manager);
  xn_id = xnap_allocate_new_id(id_manager);
  if (xn_id == -1) {
    LOG_E(XNAP, "could not allocate a new XNAP UE ID\n");
    exit(1);
  }
  req->t_ng_node_ue_xnap_id = xn_id;

  /* Target Cell Global ID */
  XNAP_FIND_PROTOCOLIE_BY_ID(XNAP_HandoverRequest_IEs_t, ie, xnHandoverRequest, XNAP_ProtocolIE_ID_id_targetCellGlobalID, true);
  if (ie == NULL) {
    LOG_E(XNAP, "XNAP_ProtocolIE_ID_id_TargetCellCGI, is NULL pointer \n");
    return -1;
  } else {
    PLMNID_TO_MCC_MNC(&ie->value.choice.Target_CGI.choice.nr->plmn_id,
                      req->plmn_id.mcc,
                      req->plmn_id.mnc,
                      req->plmn_id.mnc_digit_length);
    BIT_STRING_TO_NR_CELL_IDENTITY(&ie->value.choice.Target_CGI.choice.nr->nr_CI, req->target_cgi.cgi);
    LOG_D(XNAP, "XNAP_ProtocolIE_ID_id_TargetCellCGI, not a null pointer \n");
  }

  /* GUAMI */
  XNAP_FIND_PROTOCOLIE_BY_ID(XNAP_HandoverRequest_IEs_t, ie, xnHandoverRequest, XNAP_ProtocolIE_ID_id_GUAMI, true);
  if (ie == NULL) {
    LOG_E(XNAP, "XNAP_ProtocolIE_ID_id_GUAMI is NULL pointer \n");
    return -1;
  } else {
    LOG_D(XNAP, "XNAP_ProtocolIE_ID_id_GUAMI not a null pointer \n");
    PLMNID_TO_MCC_MNC(&ie->value.choice.GUAMI.plmn_ID, req->plmn_id.mcc, req->plmn_id.mnc, req->plmn_id.mnc_digit_length);
    req->guami.amf_region_id = BIT_STRING_to_uint8(&ie->value.choice.GUAMI.amf_region_id);
    req->guami.amf_set_id = BIT_STRING_to_uint16(&ie->value.choice.GUAMI.amf_set_id);
    req->guami.amf_pointer = BIT_STRING_to_uint8(&ie->value.choice.GUAMI.amf_pointer);
  }

  /* UE context Information */
  XNAP_FIND_PROTOCOLIE_BY_ID(XNAP_HandoverRequest_IEs_t, ie, xnHandoverRequest, XNAP_ProtocolIE_ID_id_UEContextInfoHORequest, true);
  if (ie == NULL) {
    LOG_E(XNAP, "XNAP_ProtocolIE_ID_id_UEContextInfoHORequest, is NULL pointer \n");
    return -1;
  }
  {
    /* NG-C UE associated Signalling reference - AMF UE NGAP ID */
    asn_INTEGER2uint64(&ie->value.choice.UEContextInfoHORequest.ng_c_UE_reference, &req->ue_context.ngc_ue_sig_ref);

    /* Signalling TNL association address at source NG-C side - CP Transport Layer Information */
    BIT_STRING_TO_TRANSPORT_LAYER_ADDRESS_IPv4(&ie->value.choice.UEContextInfoHORequest.cp_TNL_info_source.choice.endpointIPAddress,
                                               req->ue_context.tnl_ip_source);

    /* AS iSecurity */
    if ((ie->value.choice.UEContextInfoHORequest.securityInformation.key_NG_RAN_Star.buf)
        && (ie->value.choice.UEContextInfoHORequest.securityInformation.key_NG_RAN_Star.size == 32)) {
      memcpy(&req->ue_context.as_security_key_ranstar,
             ie->value.choice.UEContextInfoHORequest.securityInformation.key_NG_RAN_Star.buf,
             32);
    } else {
      LOG_E(XNAP, "Size of key star does not match the expected value\n");
    }
    if (req->ue_context.as_security_ncc >= 0) {
      req->ue_context.as_security_ncc = ie->value.choice.UEContextInfoHORequest.securityInformation.ncc;
    } else {
      req->ue_context.as_security_ncc = 1;
    }

    /* UESecurityCapabilities */
    req->ue_context.security_capabilities.nRencryption_algorithms =
        BIT_STRING_to_uint16(&ie->value.choice.UEContextInfoHORequest.ueSecurityCapabilities.nr_EncyptionAlgorithms);
    req->ue_context.security_capabilities.nRintegrity_algorithms =
        BIT_STRING_to_uint16(&ie->value.choice.UEContextInfoHORequest.ueSecurityCapabilities.nr_IntegrityProtectionAlgorithms);
    req->ue_context.security_capabilities.eUTRAencryption_algorithms =
        BIT_STRING_to_uint16(&ie->value.choice.UEContextInfoHORequest.ueSecurityCapabilities.e_utra_EncyptionAlgorithms);
    req->ue_context.security_capabilities.eUTRAintegrity_algorithms =
        BIT_STRING_to_uint16(&ie->value.choice.UEContextInfoHORequest.ueSecurityCapabilities.e_utra_IntegrityProtectionAlgorithms);

    /* RRC Context */
    OCTET_STRING_t *c = &ie->value.choice.UEContextInfoHORequest.rrc_Context;
    if (c->size > 1024) {
      LOG_I(XNAP, "c is not NULL");
    }
    memcpy(req->ue_context.rrc_buffer, c->buf, c->size);
    req->ue_context.rrc_buffer_size = c->size;

    /* PDU session resources to be setup list */
    if (ie->value.choice.UEContextInfoHORequest.pduSessionResourcesToBeSetup_List.list.count > 0) {
      req->ue_context.pdusession_tobe_setup_list.num_pdu =
          ie->value.choice.UEContextInfoHORequest.pduSessionResourcesToBeSetup_List.list.count;
      for (int i = 0; i < ie->value.choice.UEContextInfoHORequest.pduSessionResourcesToBeSetup_List.list.count; i++) {
        pdu_session_resources = ie->value.choice.UEContextInfoHORequest.pduSessionResourcesToBeSetup_List.list.array[i];
        /* PDU Session id */
        req->ue_context.pdusession_tobe_setup_list.pdu[i].pdusession_id = pdu_session_resources->pduSessionId;
        /* SSNSAI */
        OCTET_STRING_TO_INT8(&pdu_session_resources->s_NSSAI.sst, req->ue_context.pdusession_tobe_setup_list.pdu[i].snssai.sst);
        /* UP TNL Information */
        BIT_STRING_TO_TRANSPORT_LAYER_ADDRESS_IPv4(&pdu_session_resources->uL_NG_U_TNLatUPF.choice.gtpTunnel->tnl_address,
                                                   req->ue_context.tnl_ip_source);
        OCTET_STRING_TO_INT32(&pdu_session_resources->uL_NG_U_TNLatUPF.choice.gtpTunnel->gtp_teid,
                              req->ue_context.pdusession_tobe_setup_list.pdu[i].up_ngu_tnl_teid_upf);
        /* PDU session type */
        req->ue_context.pdusession_tobe_setup_list.pdu[i].pdu_session_type = pdu_session_resources->pduSessionType;
        /* QOS flows to be setup */
        req->ue_context.pdusession_tobe_setup_list.pdu[i].qos_list.num_qos =
            pdu_session_resources->qosFlowsToBeSetup_List.list.count;
        for (int j = 0; j < pdu_session_resources->qosFlowsToBeSetup_List.list.count; j++) {
          qos_flows = ie->value.choice.UEContextInfoHORequest.pduSessionResourcesToBeSetup_List.list.array[i]
                          ->qosFlowsToBeSetup_List.list.array[j];
          /* QFI */
          req->ue_context.pdusession_tobe_setup_list.pdu[i].qos_list.qos[j].qfi = qos_flows->qfi;
          /* dynamic */
          req->ue_context.pdusession_tobe_setup_list.pdu[i].qos_list.qos[j].qos_params.dynamic.fiveqi =
              *qos_flows->qosFlowLevelQoSParameters.qos_characteristics.choice.dynamic->fiveQI;
          req->ue_context.pdusession_tobe_setup_list.pdu[i].qos_list.qos[j].qos_params.dynamic.qos_priority_level =
              qos_flows->qosFlowLevelQoSParameters.qos_characteristics.choice.dynamic->priorityLevelQoS;
        }
      }
    }
  }

  /* UE History Information */
  XNAP_FIND_PROTOCOLIE_BY_ID(XNAP_HandoverRequest_IEs_t, ie, xnHandoverRequest, XNAP_ProtocolIE_ID_id_UEHistoryInformation, true);
  if (ie == NULL) {
    LOG_E(XNAP, "XNAP_ProtocolIE_ID_id_UEHistoryInformation is NULL pointer \n");
    return -1;
  } else {
    if (ie->value.choice.UEHistoryInformation.list.count > 0) {
      for (int i = 0; i < ie->value.choice.UEHistoryInformation.list.count; i++) {
        lastVisitedCell_Item = ie->value.choice.UEHistoryInformation.list.array[i];
        OCTET_STRING_TO_INT32(&lastVisitedCell_Item->choice.nG_RAN_Cell, req->uehistory_info.last_visited_cgi.cgi);
      }
    } else {
      LOG_D(XNAP, "XNAP_ProtocolIE_ID_id_UEHistoryInformation not a null pointer \n");
    }
  }

  itti_send_msg_to_task(TASK_RRC_GNB, instance_p->instance, message_p);
  return 0;
}

int xnap_gNB_handle_handover_request_ack(instance_t instance, sctp_assoc_t assoc_id, uint32_t stream, XNAP_XnAP_PDU_t *pdu)
{
  XNAP_HandoverRequestAcknowledge_t *xnHandoverRequestAck;
  XNAP_HandoverRequestAcknowledge_IEs_t *ie;
  xnap_gNB_instance_t *instance_p;
  instance_p = xnap_gNB_get_instance(0);
  XNAP_PDUSessionResourcesAdmitted_Item_t *pdu_session_resources;
  XNAP_QoSFlowsAdmitted_Item_t *qos_flows;

  DevAssert(pdu != NULL);
  xnHandoverRequestAck = &pdu->choice.successfulOutcome->value.choice.HandoverRequestAcknowledge;
  MessageDef *message_p = itti_alloc_new_message(TASK_XNAP, 0, XNAP_HANDOVER_REQ_ACK);
  message_p->ittiMsgHeader.originInstance = assoc_id;
  xnap_handover_req_ack_t *ack = &XNAP_HANDOVER_REQ_ACK(message_p);

  /* Source NG-RAN node UE XnAP ID */
  XNAP_FIND_PROTOCOLIE_BY_ID(XNAP_HandoverRequestAcknowledge_IEs_t,
                             ie,
                             xnHandoverRequestAck,
                             XNAP_ProtocolIE_ID_id_sourceNG_RANnodeUEXnAPID,
                             true);
  if (ie == NULL) {
    LOG_E(XNAP, "XNAP_ProtocolIE_ID_id_sourceNG_RANnodeUEXnAPID, is NULL pointer \n");
    itti_free(ITTI_MSG_ORIGIN_ID(message_p), message_p);
    return -1;
  }
  ack->s_ng_node_ue_xnap_id = ie->value.choice.NG_RANnodeUEXnAPID;

  /* Target NG-RAN node UE XnAP ID */
  XNAP_FIND_PROTOCOLIE_BY_ID(XNAP_HandoverRequestAcknowledge_IEs_t,
                             ie,
                             xnHandoverRequestAck,
 			     XNAP_ProtocolIE_ID_id_targetNG_RANnodeUEXnAPID,
                             true);
  if (ie == NULL) {
    LOG_E(XNAP, "XNAP_ProtocolIE_ID_id_sourceNG_RANnodeUEXnAPID, is NULL pointer \n");
    itti_free(ITTI_MSG_ORIGIN_ID(message_p), message_p);
    return -1;
  }
  ack->t_ng_node_ue_xnap_id = ie->value.choice.NG_RANnodeUEXnAPID_1;

  /* PDU Session Resources Admitted List */
  XNAP_FIND_PROTOCOLIE_BY_ID(XNAP_HandoverRequestAcknowledge_IEs_t,
                             ie,
                             xnHandoverRequestAck,
                             XNAP_ProtocolIE_ID_id_PDUSessionResourcesAdmitted_List,
                             true);
  if (ie == NULL) {
    LOG_E(XNAP, "XNAP_ProtocolIE_ID_id_sourceNG_RANnodeUEXnAPID, is NULL pointer \n");
    itti_free(ITTI_MSG_ORIGIN_ID(message_p), message_p);
    return -1;
  } else {
    if (ie->value.choice.PDUSessionResourcesAdmitted_List.list.count > 0) {
      ack->ue_context.pdusession_admitted_list.num_pdu = ie->value.choice.PDUSessionResourcesAdmitted_List.list.count;
      for (int i = 0; i < ie->value.choice.PDUSessionResourcesAdmitted_List.list.count; i++) {
        pdu_session_resources = ie->value.choice.PDUSessionResourcesAdmitted_List.list.array[i];
        /* PDU Session id */
        ack->ue_context.pdusession_admitted_list.pdu[i].pdusession_id = pdu_session_resources->pduSessionId;
        /* QOS flows to be setup */
        ack->ue_context.pdusession_admitted_list.pdu[i].qos_list.num_qos =
            pdu_session_resources->pduSessionResourceAdmittedInfo.qosFlowsAdmitted_List.list.count;
        for (int j = 0; j < pdu_session_resources->pduSessionResourceAdmittedInfo.qosFlowsAdmitted_List.list.count; j++) {
          qos_flows = ie->value.choice.PDUSessionResourcesAdmitted_List.list.array[i]
                          ->pduSessionResourceAdmittedInfo.qosFlowsAdmitted_List.list.array[j];
          /* QFI */
          ack->ue_context.pdusession_admitted_list.pdu[i].qos_list.qos[j].qfi = qos_flows->qfi;
        }
      }
    }
  }

  /* Target NG-RAN node To Source NG-RAN node Transparent Container */
  XNAP_FIND_PROTOCOLIE_BY_ID(XNAP_HandoverRequestAcknowledge_IEs_t,
                             ie,
                             xnHandoverRequestAck,
                             XNAP_ProtocolIE_ID_id_Target2SourceNG_RANnodeTranspContainer,
                             true);
  if (ie == NULL) {
    LOG_E(XNAP, "iXNAP_ProtocolIE_ID_id_Target2SourceNG_RANnodeTranspContainer is NULL pointer \n");
    itti_free(ITTI_MSG_ORIGIN_ID(message_p), message_p);
    return -1;
  }
  OCTET_STRING_t *c = &ie->value.choice.OCTET_STRING;
  if (c->size > 1024) {
    LOG_I(XNAP, "c is not NULL");
  }
  memcpy(ack->rrc_buffer, c->buf, c->size);
  ack->rrc_buffer_size = c->size;

  itti_send_msg_to_task(TASK_RRC_GNB, instance_p->instance, message_p);
  return 0;
}

int xnap_gNB_handle_ue_context_release(instance_t instance, sctp_assoc_t assoc_id, uint32_t stream, XNAP_XnAP_PDU_t *pdu)
{
  XNAP_UEContextRelease_t *uerelease;
  XNAP_UEContextRelease_IEs_t *ie;
  xnap_gNB_instance_t *instance_p;
  xnap_gNB_data_t *xnap_gNB_data;
  MessageDef *msg;
  int ue_id;
  int id_source;
  int id_target;

  DevAssert(pdu != NULL);
  uerelease = &pdu->choice.initiatingMessage->value.choice.UEContextRelease;
  if (stream != 0) {
    LOG_E(XNAP, "Received new xn ue context release on stream == 0\n");
    /* TODO: send a xn failure response */
    return 0;
  }
  xnap_gNB_data = xnap_get_gNB(instance, assoc_id);
  DevAssert(xnap_gNB_data != NULL);
  instance_p = xnap_gNB_get_instance(instance);
  DevAssert(instance_p != NULL);
  msg = itti_alloc_new_message(TASK_XNAP, 0, XNAP_UE_CONTEXT_RELEASE);

  /* Source NG-RAN node UE XnAP ID */
  XNAP_FIND_PROTOCOLIE_BY_ID(XNAP_UEContextRelease_IEs_t, ie, uerelease, XNAP_ProtocolIE_ID_id_sourceNG_RANnodeUEXnAPID, true);
  if (ie == NULL) {
    LOG_E(XNAP, "%s %d: ie is a NULL pointer \n", __FILE__, __LINE__);
    itti_free(ITTI_MSG_ORIGIN_ID(msg), msg);
    return -1;
  }
  id_source = ie->value.choice.NG_RANnodeUEXnAPID;

  /* Target NG-RAN node UE XnAP ID */
  XNAP_FIND_PROTOCOLIE_BY_ID(XNAP_UEContextRelease_IEs_t, ie, uerelease, XNAP_ProtocolIE_ID_id_targetNG_RANnodeUEXnAPID, true);
  if (ie == NULL) {
    LOG_E(XNAP, "%s %d: ie is a NULL pointer \n", __FILE__, __LINE__);
    itti_free(ITTI_MSG_ORIGIN_ID(msg), msg);
    return -1;
  }
  id_target = ie->value.choice.NG_RANnodeUEXnAPID_1;
  ue_id = id_source;
  if (ue_id != xnap_find_id_from_id_source(&instance_p->id_manager, id_source)) {
    LOG_E(XNAP, "incorrect/unknown XNAP IDs for UE (old ID %d new ID %d), ignoring UE context release\n", id_source, id_target);
    itti_free(ITTI_MSG_ORIGIN_ID(msg), msg);
    return 0;
  }
  if (id_target != xnap_id_get_id_target(&instance_p->id_manager, ue_id)) {
    LOG_E(XNAP,
          "UE context release: bad id_target for UE %x (id_source %d) expected %d got %d, ignoring message\n",
          xnap_id_get_ueid(&instance_p->id_manager, ue_id),
          id_source,
          xnap_id_get_id_target(&instance_p->id_manager, ue_id),
          id_target);
    itti_free(ITTI_MSG_ORIGIN_ID(msg), msg);
    return 0;
  }
  XNAP_UE_CONTEXT_RELEASE(msg).rnti = xnap_id_get_ueid(&instance_p->id_manager, ue_id);

  itti_send_msg_to_task(TASK_RRC_GNB, instance_p->instance, msg);
  xnap_release_id(&instance_p->id_manager, ue_id);
  return 0;
}
