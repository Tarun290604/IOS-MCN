#include "config-mplane.h"
#include "rpc-send-recv.h"

#include <assert.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>

#include <libxml/parser.h>
#include <libxml/tree.h>

void add_element(xmlNodePtr parent, const char *name, const char *content) {
    xmlNewChild(parent, NULL, BAD_CAST name, BAD_CAST content);
}

char *generate_uplane_conf_xml(oai_oru_data_t *oru) {
    // Create a new XML document
    xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");
    xmlNodePtr root_node = xmlNewNode(NULL, BAD_CAST "user-plane-configuration");
    xmlNewProp(root_node, BAD_CAST "xmlns", BAD_CAST "urn:o-ran:uplane-conf:1.0");
    xmlDocSetRootElement(doc, root_node);

    // Create tx-array-carriers node
    xmlNodePtr tx_node = xmlNewChild(root_node, NULL, BAD_CAST "tx-array-carriers", NULL);
    add_element(tx_node, "name", oru->tx_array_carrier.name);

    char buffer[50];
    sprintf(buffer, "%d", oru->tx_array_carrier.arfcn_center);
    add_element(tx_node, "absolute-frequency-center", buffer);

    sprintf(buffer, "%ld", oru->tx_array_carrier.center_channel_bw);
    add_element(tx_node, "center-of-channel-bandwidth", buffer);

    sprintf(buffer, "%d", oru->tx_array_carrier.channel_bw);
    add_element(tx_node, "channel-bandwidth", buffer);

    add_element(tx_node, "active", oru->tx_array_carrier.ru_carrier);
    add_element(tx_node, "rw-duplex-scheme", oru->tx_array_carrier.rw_duplex_scheme);
    add_element(tx_node, "rw-type", oru->tx_array_carrier.rw_type);

    sprintf(buffer, "%.1f", oru->tx_array_carrier.gain);
    add_element(tx_node, "gain", buffer);

    sprintf(buffer, "%d", oru->tx_array_carrier.dl_radio_frame_offset);
    add_element(tx_node, "downlink-radio-frame-offset", buffer);

    sprintf(buffer, "%d", oru->tx_array_carrier.dl_sfn_offset);
    add_element(tx_node, "downlink-sfn-offset", buffer);

    // Create rx-array-carriers node
    xmlNodePtr rx_node = xmlNewChild(root_node, NULL, BAD_CAST "rx-array-carriers", NULL);
     add_element(rx_node, "name", oru->rx_array_carrier.name);

    sprintf(buffer, "%d", oru->rx_array_carrier.arfcn_center);
    add_element(rx_node, "absolute-frequency-center", buffer);

    sprintf(buffer, "%ld", oru->rx_array_carrier.center_channel_bw);
    add_element(rx_node, "center-of-channel-bandwidth", buffer);

    sprintf(buffer, "%d", oru->rx_array_carrier.channel_bw);
    add_element(rx_node, "channel-bandwidth", buffer);

    add_element(rx_node, "active", oru->rx_array_carrier.ru_carrier);

    sprintf(buffer, "%d", oru->rx_array_carrier.dl_radio_frame_offset);
    add_element(rx_node, "downlink-radio-frame-offset", buffer);

    sprintf(buffer, "%d", oru->rx_array_carrier.dl_sfn_offset);
    add_element(rx_node, "downlink-sfn-offset", buffer);

    sprintf(buffer, "%.1f", oru->rx_array_carrier.gain_correction);
    add_element(rx_node, "gain-correction", buffer);

    sprintf(buffer, "%d", oru->rx_array_carrier.n_ta_offset);
    add_element(rx_node, "n-ta-offset", buffer);

    // Save the XML document to a string
    xmlChar *xml_buffer = NULL;
    int buffer_size = 0;
    xmlDocDumpFormatMemoryEnc(doc, &xml_buffer, &buffer_size, "UTF-8", 1);

    // Convert xmlChar* to a regular C string
    char *xml_string = strdup((const char *)xml_buffer);

    // Free the document and the temporary XML buffer
    xmlFree(xml_buffer);
    xmlFreeDoc(doc);
    xmlCleanupParser();

    return xml_string;
}

/*! This function sends the oru configuration in edit-config RPC to the connected RU*/
void cmd_edit_config(ru_session_t *ru_session, oai_oru_data_t *oru)
{
  int c, config_fd, ret = EXIT_FAILURE, content_param = 0, timeout = CLI_RPC_REPLY_TIMEOUT;
  struct stat config_stat;
  char *content = NULL, *config_m = NULL, *cont_start;
  NC_DATASTORE target = NC_DATASTORE_CANDIDATE;  // also, can be RUNNING, but by M-plane spec, we should modify CANDIDATE, then verify if it's ok and then COMMIT
  struct nc_rpc *rpc;
  NC_RPC_EDIT_DFLTOP op = NC_RPC_EDIT_DFLTOP_MERGE;  // defop merge, save the existing values + modify the ones requested + add new ones if requested
  NC_RPC_EDIT_TESTOPT test = NC_RPC_EDIT_TESTOPT_UNKNOWN;
  NC_RPC_EDIT_ERROPT err = NC_RPC_EDIT_ERROPT_UNKNOWN;

  /* Generate xml string from the configuration in oru */
  content = generate_uplane_conf_xml(oru);
  printf("[%s] Content generated\n %s\n", __func__, content);


  printf("[%s] calling nc_rpc_edit for edit config", __func__);
  rpc = nc_rpc_edit(target, op, test, err, content, NC_PARAMTYPE_CONST);
  assert(rpc != NULL && "RPC creation failed.\n");

  rpc_send_recv(ru_session, rpc, stdout, 0, timeout);
  printf("Successfully edited RU config\n");

  nc_rpc_free(rpc);
  free(content);
}

void cmd_validate(ru_session_t *ru_session)
{
  struct nc_rpc *rpc;
  int timeout = CLI_RPC_REPLY_TIMEOUT;
  char *src_start = NULL;
  NC_DATASTORE source = NC_DATASTORE_CANDIDATE;

  /* create requests */
  rpc = nc_rpc_validate(source, src_start, NC_PARAMTYPE_CONST);
  assert(rpc != NULL && "RPC val creation failed.");

  rpc_send_recv(ru_session, rpc, stdout, 0, timeout);
  printf("RU config successfully validated. Ready for the changes to be commited\n");

  nc_rpc_free(rpc);
}

void cmd_commit(ru_session_t *ru_session)
{
  struct nc_rpc *rpc;
  int confirmed = 0, timeout = CLI_RPC_REPLY_TIMEOUT;
  int32_t confirm_timeout = 0;
  char *persist = NULL, *persist_id = NULL;

  /* creat request */
  rpc = nc_rpc_commit(confirmed, confirm_timeout, persist, persist_id, NC_PARAMTYPE_CONST);
  assert(rpc != NULL && "RPC creation failed.");

  rpc_send_recv(ru_session, rpc, stdout, 0, timeout);
  printf("RU config is successfully committed\n");

  nc_rpc_free(rpc);
}
