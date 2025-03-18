#include "get-mplane.h"
#include "rpc-send-recv.h"

#include <assert.h>
#include <stdio.h>

char *cmd_get(ru_session_t *ru_session)
{
  char *filename = malloc(strlen(ru_session->ru_ip_add) + 10);
  sprintf(filename, "%s%s", ru_session->ru_ip_add, "_conf.xml");

  int timeout = CLI_RPC_REPLY_TIMEOUT;
  struct nc_rpc *rpc;
  NC_WD_MODE wd = NC_WD_UNKNOWN;  // try with explicit!
  NC_PARAMTYPE param = NC_PARAMTYPE_CONST;

  FILE *output = fopen(filename, "w");  // "delay.xml"
  char *filter = NULL;   // "/o-ran-delay-management:delay-management";

  /* create request */
  rpc = nc_rpc_get(filter, wd, param);
  assert(rpc != NULL && "RPC creation failed.\n");

  rpc_send_recv(ru_session, rpc, output, wd, timeout);
  printf("Successfully retreived RU delay profile\n");

  fclose(output);
  nc_rpc_free(rpc);

  return filename;
}
