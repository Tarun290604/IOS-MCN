#include "disconnect-mplane.h"

#include <assert.h>
#include <stdio.h>

void cmd_disconnect(ru_session_t *ru_session)
{
  assert(ru_session->session != NULL);  // probably can be taken out

  nc_session_free(ru_session->session, NULL);
  ru_session->session = NULL;

  printf("Successfully disconnected from RU with IP address %s\n", ru_session->ru_ip_add);
}
