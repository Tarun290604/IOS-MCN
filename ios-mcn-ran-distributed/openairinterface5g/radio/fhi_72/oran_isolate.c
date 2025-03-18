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

#include <stdio.h>
#include <string.h>
#include "common_lib.h"
#include "radio/ETHERNET/ethernet_lib.h"
#include "oran_isolate.h"
#include "oran-init.h"
#include "xran_fh_o_du.h"
#include "xran_sync_api.h"

#include "common/utils/LOG/log.h"
#include "common/utils/LOG/vcd_signal_dumper.h"
#include "openair1/PHY/defs_gNB.h"
#include "common/utils/threadPool/thread-pool.h"
#include "oaioran.h"
#include "oran-config.h"

// include the following file for VERSIONX, version of xran lib, to print it during
// startup. Only relevant for printing, if it ever makes problem, remove this
// line and the use of VERSIONX further below. It is relative to phy/fhi_lib/lib/api
#include "../../app/src/common.h"

/*Header files for mplane*/
#include "init-mplane.h"
#include "connect-mplane.h"
#include "get-mplane.h"
#include "subscribe-mplane.h"
#include "config-mplane.h"
#include "disconnect-mplane.h"
#include "xml/get-xml.h"

#define OAI_MPLANE // should be enabled for M plane support

extern uint32_t mplane_enable_global;
#ifdef OAI_MPLANE
int is_ru_connect = 0;
int is_ru_configured = 0;
pthread_cond_t mplane_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mplane_mutex = PTHREAD_MUTEX_INITIALIZER;

void *mplane_thread(void *arg){
  bool running = true;
  oai_oru_data_t *oru = (oai_oru_data_t *)arg;
  // TODO: to import list of IP addresses from gnb.conf and allocate the correct length
  printf("Started the mplane thread\n");
  ru_session_t *ru_session = init_mplane(0, NULL);
  nc_client_init();
  int num_ru=1;
  bool is_connect=false; // The RU connects to the DU using the call-home feature.

  ru_config_t *ru_config = calloc(num_ru, sizeof(ru_config_t));
  assert(ru_config != NULL);

  for (size_t i = 0; i < num_ru; i++) {
    printf("executing the command with is_connect %d num_ru %d \n", is_connect, num_ru);
    (is_connect) ? cmd_connect(&ru_session[i]) : cmd_listen(&ru_session[i]);
  }
  
  for (size_t i = 0; i < num_ru; i++) {
    const char *filename = cmd_get(&ru_session[i]);
    bool synced = get_ptp_sync_status(filename);
    get_uplane_conf_data(filename, oru);
    oru->delay_management = get_ru_delay_profile(filename);
    if(synced == false){
      cmd_subscribe(&ru_session[i]);
    }
    printf("received config from RU %s \n",filename);
  }
  
  for (size_t i = 0; i < num_ru; i++) {
    cmd_edit_config(&ru_session[i], oru); //Segmentation fault here is fixed and edit config works fine. 
    cmd_validate(&ru_session[i]);
    cmd_commit(&ru_session[i]); 
  }
  printf("Activating the carrier of the RU num_ru %d\n", num_ru);
  for (size_t i = 0; i < num_ru; i++) {
    strcpy(oru->tx_array_carrier.ru_carrier, "ACTIVE");
    strcpy(oru->rx_array_carrier.ru_carrier, "ACTIVE");
    cmd_edit_config(&ru_session[i], oru); 
    cmd_validate(&ru_session[i]);
    cmd_commit(&ru_session[i]); 
  }

  pthread_mutex_lock(&mplane_mutex); // Lock the mutex
  is_ru_connect=1; // This should be made 1 once the client is connected to RU, should debug
  pthread_cond_broadcast(&mplane_cond); // Notify the thread
  pthread_mutex_unlock(&mplane_mutex); // Unlock the mutex

  while(1){

  }
  printf("Exiting the mplane thread is_connect %d num_ru %d command %d\n", is_connect, num_ru);
  for (size_t i = 0; i < num_ru; i++) {
    cmd_disconnect(&ru_session[i]);
  }
  is_ru_connect=0; // Once the client is disconnected from the RU this becomes 0. 
  nc_client_destroy();
}
#endif

typedef struct {
  eth_state_t e;
  rru_config_msg_type_t last_msg;
  int capabilities_sent;
  void *oran_priv;
} oran_eth_state_t;

notifiedFIFO_t oran_sync_fifo;

int trx_oran_start(openair0_device *device)
{
  printf("ORAN: %s\n", __FUNCTION__);

  oran_eth_state_t *s = device->priv;

  // Start ORAN
  if (xran_start(s->oran_priv) != 0) {
    printf("%s:%d:%s: Start ORAN failed ... Exit\n", __FILE__, __LINE__, __FUNCTION__);
    exit(1);
  } else {
    printf("Start ORAN. Done\n");
  }
  return 0;
}

void trx_oran_end(openair0_device *device)
{
  printf("ORAN: %s\n", __FUNCTION__);
  oran_eth_state_t *s = device->priv;
  xran_close(s->oran_priv);
}

int trx_oran_stop(openair0_device *device)
{
  printf("ORAN: %s\n", __FUNCTION__);
  oran_eth_state_t *s = device->priv;
  xran_stop(s->oran_priv);
  return (0);
}

int trx_oran_set_freq(openair0_device *device, openair0_config_t *openair0_cfg)
{
  printf("ORAN: %s\n", __FUNCTION__);
  return (0);
}

int trx_oran_set_gains(openair0_device *device, openair0_config_t *openair0_cfg)
{
  printf("ORAN: %s\n", __FUNCTION__);
  return (0);
}

int trx_oran_get_stats(openair0_device *device)
{
  printf("ORAN: %s\n", __FUNCTION__);
  return (0);
}

int trx_oran_reset_stats(openair0_device *device)
{
  printf("ORAN: %s\n", __FUNCTION__);
  return (0);
}

int ethernet_tune(openair0_device *device, unsigned int option, int value)
{
  printf("ORAN: %s\n", __FUNCTION__);
  return 0;
}

int trx_oran_write_raw(openair0_device *device, openair0_timestamp timestamp, void **buff, int nsamps, int cc, int flags)
{
  printf("ORAN: %s\n", __FUNCTION__);
  return 0;
}

int trx_oran_read_raw(openair0_device *device, openair0_timestamp *timestamp, void **buff, int nsamps, int cc)
{
  printf("ORAN: %s\n", __FUNCTION__);
  return 0;
}

char *msg_type(int t)
{
  static char *s[12] = {
      "RAU_tick",
      "RRU_capabilities",
      "RRU_config",
      "RRU_config_ok",
      "RRU_start",
      "RRU_stop",
      "RRU_sync_ok",
      "RRU_frame_resynch",
      "RRU_MSG_max_num",
      "RRU_check_sync",
      "RRU_config_update",
      "RRU_config_update_ok",
  };

  if (t < 0 || t > 11)
    return "UNKNOWN";
  return s[t];
}

int trx_oran_ctlsend(openair0_device *device, void *msg, ssize_t msg_len)
{
  RRU_CONFIG_msg_t *rru_config_msg = msg;
  oran_eth_state_t *s = device->priv;

  printf("ORAN: %s\n", __FUNCTION__);

  printf("    rru_config_msg->type %d [%s]\n", rru_config_msg->type, msg_type(rru_config_msg->type));

  s->last_msg = rru_config_msg->type;

  return msg_len;
}

int trx_oran_ctlrecv(openair0_device *device, void *msg, ssize_t msg_len)
{
  RRU_CONFIG_msg_t *rru_config_msg = msg;
  oran_eth_state_t *s = device->priv;

  printf("ORAN: %s\n", __FUNCTION__);

  if (s->last_msg == RAU_tick && s->capabilities_sent == 0) {
    printf("ORAN ctrlrcv RRU_tick received and send capabilities hard coded\n");
    RRU_capabilities_t *cap;
    rru_config_msg->type = RRU_capabilities;
    rru_config_msg->len = sizeof(RRU_CONFIG_msg_t) - MAX_RRU_CONFIG_SIZE + sizeof(RRU_capabilities_t);
    // Fill RRU capabilities (see openair1/PHY/defs_RU.h)
    // For now they are hard coded - try to retreive the params from openari device

    cap = (RRU_capabilities_t *)&rru_config_msg->msg[0];
    cap->FH_fmt = OAI_IF4p5_only;
    cap->num_bands = 1;
    cap->band_list[0] = 78;
    // cap->num_concurrent_bands             = 1; component carriers
    cap->nb_rx[0] = 1; // device->openair0_cfg->rx_num_channels;
    cap->nb_tx[0] = 1; // device->openair0_cfg->tx_num_channels;
    cap->max_pdschReferenceSignalPower[0] = -27;
    cap->max_rxgain[0] = 90;
    cap->N_RB_DL[0] = 106;
    cap->N_RB_UL[0] = 106;

    s->capabilities_sent = 1;

    return rru_config_msg->len;
  }
  if (s->last_msg == RRU_config) {
    printf("Oran RRU_config\n");
    rru_config_msg->type = RRU_config_ok;
  }
  return 0;
}

void oran_fh_if4p5_south_in(RU_t *ru, int *frame, int *slot)
{
  ru_info_t ru_info;
  ru_info.nb_rx = ru->nb_rx;
  ru_info.rxdataF = ru->common.rxdataF;
  ru_info.prach_buf = ru->prach_rxsigF[0]; // index: [prach_oca][ant_id]

  RU_proc_t *proc = &ru->proc;
  extern uint16_t sl_ahead;
  int f, sl;
  LOG_D(PHY, "Read rxdataF %p,%p\n", ru_info.rxdataF[0], ru_info.rxdataF[1]);
  start_meas(&ru->rx_fhaul);
  int ret = xran_fh_rx_read_slot(&ru_info, &f, &sl);
  stop_meas(&ru->rx_fhaul);
  LOG_D(PHY, "Read %d.%d rxdataF %p,%p\n", f, sl, ru_info.rxdataF[0], ru_info.rxdataF[1]);
  if (ret != 0) {
    printf("ORAN: %d.%d ORAN_fh_if4p5_south_in ERROR in RX function \n", f, sl);
  }

  int slots_per_frame = 10 << (ru->openair0_cfg.nr_scs_for_raster);
  proc->tti_rx = sl;
  proc->frame_rx = f;
  proc->tti_tx = (sl + sl_ahead) % slots_per_frame;
  proc->frame_tx = (sl > (slots_per_frame - 1 - sl_ahead)) ? (f + 1) & 1023 : f;

  if (proc->first_rx == 0) {
    if (proc->tti_rx != *slot) {
      LOG_E(PHY,
            "Received Time doesn't correspond to the time we think it is (slot mismatch, received %d.%d, expected %d.%d)\n",
            proc->frame_rx,
            proc->tti_rx,
            *frame,
            *slot);
      *slot = proc->tti_rx;
    }

    if (proc->frame_rx != *frame) {
      LOG_E(PHY,
            "Received Time doesn't correspond to the time we think it is (frame mismatch, %d.%d , expected %d.%d)\n",
            proc->frame_rx,
            proc->tti_rx,
            *frame,
            *slot);
      *frame = proc->frame_rx;
    }
  } else {
    proc->first_rx = 0;
    LOG_I(PHY, "before adjusting, OAI: frame=%d slot=%d, XRAN: frame=%d slot=%d\n", *frame, *slot, proc->frame_rx, proc->tti_rx);
    *frame = proc->frame_rx;
    *slot = proc->tti_rx;
    LOG_I(PHY, "After adjusting, OAI: frame=%d slot=%d, XRAN: frame=%d slot=%d\n", *frame, *slot, proc->frame_rx, proc->tti_rx);
  }
}

void oran_fh_if4p5_south_out(RU_t *ru, int frame, int slot, uint64_t timestamp)
{
  start_meas(&ru->tx_fhaul);
  ru_info_t ru_info;
  ru_info.nb_tx = ru->nb_tx;
  ru_info.txdataF_BF = ru->common.txdataF_BF;
  // printf("south_out:\tframe=%d\tslot=%d\ttimestamp=%ld\n",frame,slot,timestamp);

  int ret = xran_fh_tx_send_slot(&ru_info, frame, slot, timestamp);
  if (ret != 0) {
    printf("ORAN: ORAN_fh_if4p5_south_out ERROR in TX function \n");
  }
  stop_meas(&ru->tx_fhaul);
}

void *get_internal_parameter(char *name)
{
  printf("ORAN: %s\n", __FUNCTION__);

  if (!strcmp(name, "fh_if4p5_south_in"))
    return (void *)oran_fh_if4p5_south_in;
  if (!strcmp(name, "fh_if4p5_south_out"))
    return (void *)oran_fh_if4p5_south_out;

  return NULL;
}

__attribute__((__visibility__("default"))) int transport_init(openair0_device *device,
                                                              openair0_config_t *openair0_cfg,
                                                              eth_params_t *eth_params)
{
  oran_eth_state_t *eth;

  device->Mod_id = 0;
  device->transp_type = ETHERNET_TP;
  device->trx_start_func = trx_oran_start;
  device->trx_get_stats_func = trx_oran_get_stats;
  device->trx_reset_stats_func = trx_oran_reset_stats;
  device->trx_end_func = trx_oran_end;
  device->trx_stop_func = trx_oran_stop;
  device->trx_set_freq_func = trx_oran_set_freq;
  device->trx_set_gains_func = trx_oran_set_gains;

  device->trx_write_func = trx_oran_write_raw;
  device->trx_read_func = trx_oran_read_raw;

  device->trx_ctlsend_func = trx_oran_ctlsend;
  device->trx_ctlrecv_func = trx_oran_ctlrecv;

  device->get_internal_parameter = get_internal_parameter;

  eth = (oran_eth_state_t *)calloc(1, sizeof(oran_eth_state_t));
  if (eth == NULL) {
    AssertFatal(0 == 1, "out of memory\n");
  }

  eth->e.flags = ETH_RAW_IF4p5_MODE;
  eth->e.compression = NO_COMPRESS;
  eth->e.if_name = eth_params->local_if_name;
  eth->oran_priv = NULL; // define_oran_pointer();
  device->priv = eth;
  device->openair0_cfg = &openair0_cfg[0];

  eth->last_msg = (rru_config_msg_type_t)-1;

  LOG_I(HW, "Initializing O-RAN 7.2 FH interface through xran library (compiled against headers of %s)\n", VERSIONX);

  initNotifiedFIFO(&oran_sync_fifo);

  struct xran_fh_init fh_init = {0};
  struct xran_fh_config fh_config[XRAN_PORTS_NUM] = {0};
if(!mplane_enable_global){
  bool success = get_xran_config(openair0_cfg, &fh_init, fh_config);
  AssertFatal(success, "cannot get configuration for xran\n");
}
else {
#ifdef OAI_MPLANE
  /* TODO: M-plane integration */
  pthread_t mplane;
  oai_oru_data_t *oru = (oai_oru_data_t *)&openair0_cfg->split7.oru;
  threadCreate(&mplane, mplane_thread, (void *)oru, "mplane_thread", -1,OAI_PRIORITY_RT_LOW);

  pthread_mutex_lock(&mplane_mutex);
  while(!is_ru_connect){
    printf("[%s] waiting for conditional broadcast\n", __func__);
    pthread_cond_wait(&mplane_cond, &mplane_mutex);
    printf("[%s] broadcast received and is_ru_connect %d\n", __func__, is_ru_connect);
  }
  // while(!is_ru_configured){
  //   printf("[%s] waiting for conditional broadcast in is_ru_configured\n", __func__);
  //   struct timespec ts;
  //   clock_gettime(CLOCK_REALTIME, &ts);
  //   ts.tv_sec += 2;  // Wait for 2 seconds
  //   pthread_cond_timedwait(&mplane_cond, &mplane_mutex, &ts);
  //   printf("[%s] broadcast received and is_ru_configured %d\n", __func__, is_ru_configured);
  // }
  pthread_mutex_unlock(&mplane_mutex);
  // sleep(30); // Added this for debugging the issue caused by waiting for more than 20s here. 
  bool success = get_xran_config(openair0_cfg, &fh_init, fh_config);
  AssertFatal(success, "cannot get configuration for xran\n");

  print_fh_init(&fh_init);
#else
  AssertFatal(1==0,"Rebuild code defining OAI_MPLANE macro or run without --mplane\n");
#endif
  }
  eth->oran_priv = oai_oran_initialize(&fh_init, fh_config);
  AssertFatal(eth->oran_priv != NULL, "can not initialize fronthaul");
  // create message queues for ORAN sync
  return 0;
}
