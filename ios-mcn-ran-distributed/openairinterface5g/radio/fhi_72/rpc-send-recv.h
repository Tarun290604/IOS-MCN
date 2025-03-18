#ifndef RPC_SEND_RECEIVE_MPLANE_H
#define RPC_SEND_RECEIVE_MPLANE_H

#include "ru-session-api.h"

#define CLI_RPC_REPLY_TIMEOUT 5   // time to wait for server reply

void rpc_send_recv(ru_session_t *ru_session, struct nc_rpc *rpc, FILE *output, NC_WD_MODE wd_mode, int timeout_s);

#endif
