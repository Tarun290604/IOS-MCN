#include "init-mplane.h"

#include <assert.h>
#include <stdio.h>
#include <stdbool.h>

static void lnc2_print_clb(NC_VERB_LEVEL level, const char *msg)
{
  printf("netconf2 verb level = %d\n", level);

  switch (level) {
    case NC_VERB_ERROR:
      fprintf(stderr, "nc ERROR: %s\n", msg);
      assert(false && "Cannot continue, netconf2 version does not match\n");
      break;
    case NC_VERB_WARNING:
      fprintf(stderr, "nc WARNING: %s\n", msg);
      break;
    case NC_VERB_VERBOSE:
      fprintf(stderr, "nc VERBOSE: %s\n", msg);
      break;
    case NC_VERB_DEBUG:
    case NC_VERB_DEBUG_LOWLVL:
      fprintf(stderr, "nc DEBUG: %s\n", msg);
      break;
  }
}

static void ly_print_clb(LY_LOG_LEVEL level, const char *msg, const char *path)
{
  printf("yang verb level = %d\n", level);
  // printf("ru address = %s\n", ru_ip_add);

  switch (level) {
    case LY_LLERR:
      if (path) {
          fprintf(stderr, "ly ERROR: %s (%s)\n", msg, path);
      } else {
          fprintf(stderr, "ly ERROR: %s\n", msg);
      }
      assert(false && "Cannot continue, yang version does not match\n");
      break;
    case LY_LLWRN:
      if (path) {
          fprintf(stderr, "ly WARNING: %s (%s)\n", msg, path);
      } else {
          fprintf(stderr, "ly WARNING: %s\n", msg);
      }
      break;
    case LY_LLVRB:
      if (path) {
          fprintf(stderr, "ly VERBOSE: %s (%s)\n", msg, path);
      } else {
          fprintf(stderr, "ly VERBOSE: %s\n", msg);
      }
      break;
    case LY_LLDBG:
      if (path) {
          fprintf(stderr, "ly DEBUG: %s (%s)\n", msg, path);
      } else {
          fprintf(stderr, "ly DEBUG: %s\n", msg);
      }
      break;
    default:
      /* silent, just to cover enum, shouldn't be here in real world */
      return;
  }
}

ru_session_t * init_mplane(const int num_ru, char *ru_ip_add)
{
  ru_session_t *ru_session = calloc(num_ru, sizeof(ru_session_t));
  assert(ru_session != NULL);

  for (int i = 0; i < num_ru; i++){
    ru_session[i].session = NULL;
    ru_session[i].ru_ip_add = malloc(strlen(ru_ip_add) + 1);
    memcpy(ru_session[i].ru_ip_add, ru_ip_add, strlen(ru_ip_add) + 1);
  }

  // logs for netconf2 and yang libraries
  // nc_set_print_clb(lnc2_print_clb); 
  ly_set_log_clb(ly_print_clb, 1);
  // NOTE: nc_set_print_clb_session() function introduced in v2

  return ru_session;
}
