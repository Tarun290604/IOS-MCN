#include "get-xml.h"

#include <assert.h>
#include <string.h>

#include <libxml/parser.h>

static void log_ru_delay_profile(delay_management_t *delay)
{
  printf("\
  T2a_min_up %d\n\
  T2a_max_up %d\n\
  T2a_min_cp_dl %d\n\
  T2a_max_cp_dl %d\n\
  Tcp_adv_dl %d\n\
  Ta3_min %d\n\
  Ta3_max %d\n\
  T2a_min_cp_ul %d\n\
  T2a_max_cp_ul %d\n",
    delay->T2a_min_up,
    delay->T2a_max_up,
    delay->T2a_min_cp_dl,
    delay->T2a_max_cp_dl,
    delay->Tcp_adv_dl,
    delay->Ta3_min,
    delay->Ta3_max,
    delay->T2a_min_cp_ul,
    delay->T2a_max_cp_ul);
}

static void store_ru_delay_profile(xmlNode *node, delay_management_t *delay)
{
  for (xmlNode *cur_child = node; cur_child; cur_child = cur_child->next) {
    if(cur_child->type == XML_ELEMENT_NODE){
      int value = atoi((const char *)xmlNodeGetContent(cur_child));

      if (strcmp((const char *)cur_child->name, "t2a-min-up") == 0) {
        delay->T2a_min_up = value;
      } else if (strcmp((const char *)cur_child->name, "t2a-max-up") == 0) {
        delay->T2a_max_up = value;
      } else if (strcmp((const char *)cur_child->name, "t2a-min-cp-dl") == 0) {
        delay->T2a_min_cp_dl = value;
      } else if (strcmp((const char *)cur_child->name, "t2a-max-cp-dl") == 0) {
        delay->T2a_max_cp_dl = value;
      } else if (strcmp((const char *)cur_child->name, "tcp-adv-dl") == 0) {
        delay->Tcp_adv_dl = value;
      } else if (strcmp((const char *)cur_child->name, "ta3-min") == 0) {
        delay->Ta3_min = value;
      } else if (strcmp((const char *)cur_child->name, "ta3-max") == 0) {
        delay->Ta3_max = value;
      } else if (strcmp((const char *)cur_child->name, "t2a-min-cp-ul") == 0) {
        delay->T2a_min_cp_ul = value;
      } else if (strcmp((const char *)cur_child->name, "t2a-max-cp-ul") == 0) {
        delay->T2a_max_cp_ul = value;
      }
    }
  }
}

static void find_ru_delay_profile(xmlNode *node, delay_management_t *delay)
{
  for(xmlNode *cur_node = node; cur_node; cur_node = cur_node->next){
    if(cur_node->type == XML_ELEMENT_NODE){
      if(strcmp((const char*)cur_node->name, "ru-delay-profile") == 0){
        store_ru_delay_profile(cur_node->children, delay);
        break;
      } else {
        find_ru_delay_profile(cur_node->children, delay);
      }
    }
  }
}

delay_management_t get_ru_delay_profile(const char *filename)
{
  delay_management_t delay = {0};

  // Initialize the xml file
  xmlDoc *doc = xmlReadFile(filename, NULL, 0);
  xmlNode *root_element = xmlDocGetRootElement(doc);

  find_ru_delay_profile(root_element->children, &delay);
  log_ru_delay_profile(&delay);

  return delay;
}

static void find_ptp_status(xmlNode *node, bool *synced)
{
  for (xmlNode *cur_node = node; cur_node; cur_node = cur_node->next) {
    if(cur_node->type == XML_ELEMENT_NODE){
      if(strcmp((const char*)cur_node->name, "sync-state") == 0){
        printf("cur_node->name = %s and cur_node->value = %s\n", (const char*)cur_node->name, (const char *)xmlNodeGetContent(cur_node));
        if(strcmp((char *)xmlNodeGetContent(cur_node), "LOCKED") == 0)
          *synced = true;
        break;
      } else {
        find_ptp_status(cur_node->children, synced);
      }
    }
  }
}

bool get_ptp_sync_status(const char *filename)
{
  bool synced = false;

  // Initialize the xml file
  xmlDoc *doc = xmlReadFile(filename, NULL, 0);
  xmlNode *root_element = xmlDocGetRootElement(doc);

  find_ptp_status(root_element->children, &synced);

  return synced;
}

static void store_tx_array_carriers(xmlNode *node, array_carrier_t *tx_array_carrier)
{
  for (xmlNode *cur_child = node; cur_child; cur_child = cur_child->next) {
    if(cur_child->type == XML_ELEMENT_NODE){
    //   int value = atoi((const char *)xmlNodeGetContent(cur_child));
    int value;

      if (strcmp((const char *)cur_child->name, "name") == 0) {
        strcpy(tx_array_carrier->name, (const char *)xmlNodeGetContent(cur_child));
      } else if (strcmp((const char *)cur_child->name, "absolute-frequency-center") == 0) {
        value = atoi((const char *)xmlNodeGetContent(cur_child));
        tx_array_carrier->arfcn_center = value;
      } else if (strcmp((const char *)cur_child->name, "center-of-channel-bandwidth") == 0) {
        value = atoi((const char *)xmlNodeGetContent(cur_child));
        tx_array_carrier->center_channel_bw = (uint32_t)value;
      } else if (strcmp((const char *)cur_child->name, "channel-bandwidth") == 0) {
        value = atoi((const char *)xmlNodeGetContent(cur_child));
        tx_array_carrier->channel_bw = value;
      } else if (strcmp((const char *)cur_child->name, "active") == 0) {
        strcpy(tx_array_carrier->ru_carrier, (const char *)xmlNodeGetContent(cur_child));
      } else if (strcmp((const char *)cur_child->name, "rw-duplex-scheme") == 0) {
        strcpy(tx_array_carrier->rw_duplex_scheme, (const char *)xmlNodeGetContent(cur_child));
      } else if (strcmp((const char *)cur_child->name, "rw-type") == 0) {
        strcpy(tx_array_carrier->rw_type, (const char *)xmlNodeGetContent(cur_child));
      } else if (strcmp((const char *)cur_child->name, "gain") == 0) {
        value = atoi((const char *)xmlNodeGetContent(cur_child));
        tx_array_carrier->gain = value;
      } else if (strcmp((const char *)cur_child->name, "downlink-radio-frame-offset") == 0) {
        value = atoi((const char *)xmlNodeGetContent(cur_child));
        tx_array_carrier->dl_radio_frame_offset = value;
      } else if (strcmp((const char *)cur_child->name, "downlink-sfn-offset") == 0) {
        value = atoi((const char *)xmlNodeGetContent(cur_child));
        tx_array_carrier->dl_sfn_offset = value;
      }
    }
  }
}

static void store_rx_array_carriers(xmlNode *node, array_carrier_t *rx_array_carrier)
{
  for (xmlNode *cur_child = node; cur_child; cur_child = cur_child->next) {
    if(cur_child->type == XML_ELEMENT_NODE){
    //   int value = atoi((const char *)xmlNodeGetContent(cur_child));
    int value;

      if (strcmp((const char *)cur_child->name, "name") == 0) {
        strcpy(rx_array_carrier->name, (const char *)xmlNodeGetContent(cur_child));
      } else if (strcmp((const char *)cur_child->name, "absolute-frequency-center") == 0) {
        value = atoi((const char *)xmlNodeGetContent(cur_child));
        rx_array_carrier->arfcn_center = value;
      } else if (strcmp((const char *)cur_child->name, "center-of-channel-bandwidth") == 0) {
        value = atoi((const char *)xmlNodeGetContent(cur_child));
        rx_array_carrier->center_channel_bw = (uint32_t)value;
      } else if (strcmp((const char *)cur_child->name, "channel-bandwidth") == 0) {
        value = atoi((const char *)xmlNodeGetContent(cur_child));
        rx_array_carrier->channel_bw = value;
      } else if (strcmp((const char *)cur_child->name, "active") == 0) {
        strcpy(rx_array_carrier->ru_carrier, (const char *)xmlNodeGetContent(cur_child));
      } else if (strcmp((const char *)cur_child->name, "downlink-radio-frame-offset") == 0) {
        value = atoi((const char *)xmlNodeGetContent(cur_child));
        rx_array_carrier->dl_radio_frame_offset = value;
      } else if (strcmp((const char *)cur_child->name, "downlink-sfn-offset") == 0) {
        value = atoi((const char *)xmlNodeGetContent(cur_child));
        rx_array_carrier->dl_sfn_offset = value;
      } else if (strcmp((const char *)cur_child->name, "gain-correction") == 0) {
        value = atoi((const char *)xmlNodeGetContent(cur_child));
        rx_array_carrier->gain_correction = value;
      } else if (strcmp((const char *)cur_child->name, "n-ta-offset") == 0) {
        value = atoi((const char *)xmlNodeGetContent(cur_child));
        rx_array_carrier->n_ta_offset = value;
      } 
    }
  }
}

static void find_uplane_conf_data(xmlNode *node, oai_oru_data_t *oru){
    for(xmlNode *cur_node = node; cur_node; cur_node = cur_node->next){
            if(cur_node->type == XML_ELEMENT_NODE){
            if(strcmp((const char*)cur_node->name, "tx-array-carriers") == 0){
                store_tx_array_carriers(cur_node->children, &oru->tx_array_carrier);
                break;
            } else {
                find_uplane_conf_data(cur_node->children, oru);
            }
        }
    }
    for(xmlNode *cur_node = node; cur_node; cur_node = cur_node->next){
            if(cur_node->type == XML_ELEMENT_NODE){
            if(strcmp((const char*)cur_node->name, "rx-array-carriers") == 0){
                store_rx_array_carriers(cur_node->children, &oru->rx_array_carrier);
                break;
            } else {
                find_uplane_conf_data(cur_node->children, oru);
            }
        }
    }
}

void get_uplane_conf_data(const char *filename, oai_oru_data_t *oru){

  // Initialize the xml file
  xmlDoc *doc = xmlReadFile(filename, NULL, 0);
  xmlNode *root_element = xmlDocGetRootElement(doc);

  find_uplane_conf_data(root_element->children, oru);
}
