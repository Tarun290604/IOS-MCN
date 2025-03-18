#include "subscribe-mplane.h"
#include "rpc-send-recv.h"

#include <assert.h>
#include <stdio.h>
#include <stdbool.h>

static bool synced = false;

#define V2
#ifdef V1
static void notif_clb_v1(struct nc_session *session, const struct nc_notif *notif)
{
  FILE *output = nc_session_get_data(session);
  LYD_FORMAT output_format = LYD_XML;
  uint32_t output_flag = 0;

  fprintf(output, "notification (%s)\n", notif->datetime);
  lyd_print_file(output, notif->tree, output_format, LYP_WITHSIBLINGS | output_flag);
  fprintf(output, "\n");
  fflush(output);

  // add here fo synced value
}
#elif defined V2
static void notif_clb_v2(struct nc_session *session, const struct lyd_node *envp, const struct lyd_node *op, void *user_data)
{
  FILE *output = user_data;
  LYD_FORMAT output_format = LYD_XML;
  uint32_t output_flag = 0;

  fprintf(output, "notification (%s)\n", ((struct lyd_node_opaq *)lyd_child(envp))->value);
  lyd_print_file(output, op, output_format, LYD_PRINT_WITHSIBLINGS | output_flag);
  fprintf(output, "\n");
  fflush(output);

  // printf("current = %s\n", op->schema->name);
  // printf("child = %s\n", ((struct lyd_node_inner *)op)->child->schema->name);
  // printf("child value = %s\n", lyd_get_value(((struct lyd_node_inner *)op)->child));
  if(strcmp(lyd_get_value(((struct lyd_node_inner *)op)->child), "LOCKED") == 0){
    synced = true;
  } else {
    synced = false;
  }
}
#endif

void cmd_subscribe(ru_session_t *ru_session)
{
  int timeout = CLI_RPC_REPLY_TIMEOUT;
  struct nc_rpc *rpc;
  NC_WD_MODE wd = NC_WD_UNKNOWN;
  NC_PARAMTYPE param = NC_PARAMTYPE_CONST;
  char *stream = NULL; // "NETCONF" for all notifications
  char *filter = "/o-ran-sync:synchronization-state-change"; // NULL for all notifications
  char *start_time = NULL, *stop_time = NULL;
  FILE *output = stdout;

  /* create requests */
  rpc = nc_rpc_subscribe(stream, filter, start_time, stop_time, param);
  assert(rpc != NULL && "RPC creation failed.\n");

  /* create notification thread so that notifications can immediately be received */
#ifdef V1
  if (!nc_session_ntf_thread_running(ru_session->session)) {
    nc_session_set_data(ru_session->session, output);
    int ret = nc_recv_notif_dispatch(ru_session->session, notif_clb_v1);
    assert(ret == 0 && "Failed to create notification thread.");
  }
#elif defined V2
  int ret = nc_recv_notif_dispatch_data(ru_session->session, notif_clb_v2, output, NULL);
  assert(ret == 0 && "Failed to create notification thread.");
#endif

  rpc_send_recv(ru_session, rpc, output, wd, timeout);
  printf("Successfully subscribed to PTP sync status\n");

  while(!synced){
    // wait
  }

  free(start_time);
  free(stop_time);
  nc_rpc_free(rpc);
}
