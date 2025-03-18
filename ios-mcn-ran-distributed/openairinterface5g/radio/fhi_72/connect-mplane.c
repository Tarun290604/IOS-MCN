#include "connect-mplane.h"

#include <assert.h>
#include <stdio.h>

#define CLI_CH_TIMEOUT 30         // time to wait for call-home

#define HOME_DIR "/home/test"

static int my_auth_hostkey_check(const char *hostname, ssh_session session, void *priv)
{
  (void)hostname;
  (void)session;
  (void)priv;

  return 0;
}

/* be aware that this function might need to be expanded to unix and TLS use cases */
void cmd_listen(ru_session_t * ru_session)
{
  int port = NC_PORT_CH_SSH;
  char *host = "0.0.0.0";       // better IPv4
  int timeout = CLI_CH_TIMEOUT;
  char *user = "root";

  /* create the session */
  nc_client_ssh_ch_set_username(user);
  nc_client_ssh_ch_add_bind_listen(host, port);

  nc_client_ssh_ch_set_auth_pref(NC_SSH_AUTH_PASSWORD, -1);
  nc_client_ssh_ch_set_auth_pref(NC_SSH_AUTH_PUBLICKEY, 1);  // ssh-key identification
  nc_client_ssh_ch_set_auth_pref(NC_SSH_AUTH_INTERACTIVE, -1);

  char pub_key[64], priv_key[64];
  sprintf(pub_key, "%s%s", HOME_DIR, "/.ssh/id_rsa.pub");
  sprintf(priv_key, "%s%s", HOME_DIR, "/.ssh/id_rsa");
  printf("pub_key = %s, prv_key = %s\n", pub_key, priv_key);
  int keypair_ret = nc_client_ssh_ch_add_keypair(pub_key, priv_key);
  assert(keypair_ret == 0 && "Unable to authenticate RU\n");
  nc_client_ssh_ch_set_auth_hostkey_check_clb(my_auth_hostkey_check, "DATA");  // host-key identification

  printf("Waiting %ds for an SSH Call Home connection on port %u...\n", timeout, port);

  int ret = nc_accept_callhome(timeout * 1000, NULL, &ru_session->session); // check the right session; maybe session[0] is already ongoing, and need to create session[1]
  assert(ret == 1 && "SSH Call Home failed.");

  nc_client_ssh_ch_del_bind(host, port);

  const char *ru_ip_add = nc_session_get_host(ru_session->session);
  ru_session->ru_ip_add = malloc(strlen(ru_ip_add) + 1);
  memcpy(ru_session->ru_ip_add, ru_ip_add, strlen(ru_ip_add) + 1);
  printf("Successfuly connected to RU with IP address %s\n", ru_session->ru_ip_add);
}

void cmd_connect(ru_session_t *ru_session)
{
  int port = NC_PORT_SSH;
  char *user = "root";

  nc_client_ssh_set_username(user);

  nc_client_ssh_set_auth_pref(NC_SSH_AUTH_PASSWORD, -1);
  nc_client_ssh_set_auth_pref(NC_SSH_AUTH_PUBLICKEY, 1);  // ssh-key identification
  nc_client_ssh_set_auth_pref(NC_SSH_AUTH_INTERACTIVE, -1);

  char pub_key[64], priv_key[64];
  sprintf(pub_key, "%s%s", HOME_DIR, "/.ssh/id_rsa.pub");
  sprintf(priv_key, "%s%s", HOME_DIR, "/.ssh/id_rsa");
  printf("pub_key = %s, prv_key = %s\n", pub_key, priv_key);
  int keypair_ret = nc_client_ssh_add_keypair(pub_key, priv_key);
  assert(keypair_ret == 0 && "Unable to authenticate RU\n");
  nc_client_ssh_set_auth_hostkey_check_clb(my_auth_hostkey_check, "DATA");  // host-key identification


  /* create the session */
  ru_session->session = nc_connect_ssh(ru_session->ru_ip_add, port, NULL);
  assert(ru_session->session != NULL && "Connecting to the RU as user root failed.\n");   // ru_ip_add, port, user);   // use AssertFatal()

  printf("Successfuly connected to RU with IP address %s\n", ru_session->ru_ip_add);
}
