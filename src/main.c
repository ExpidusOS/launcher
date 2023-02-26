#include <expidus/vendor-config.h>
#include <assert.h>
#include <expidus-launcher-build.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef HAS_PLYMOUTH
#include <ply-boot-client.h>

static ply_boot_client_t* plymouth_client = NULL;
static bool is_plymouth_connected = false;
static int plymouth_reconn_tries = 3;

static void plymouth_client_disconnect(void* data, ply_boot_client_t* client) {
  fprintf(stderr, "[WARN]: lost connection with plymouth\n");

  is_plymouth_connected = false;

  if (plymouth_reconn_tries > 0) {
    fprintf(stdout, "[MSG]: retrying %d more times to connect to plymouth\n", plymouth_reconn_tries--);
    is_plymouth_connected = ply_boot_client_connect(plymouth_client, plymouth_client_disconnect, NULL);
  }
}
#endif

static void print_err(const char* str) {
#ifdef HAS_PLYMOUTH
  if (plymouth_client != NULL && is_plymouth_connected) {
    ply_boot_client_tell_daemon_to_display_message(plymouth_client, str, NULL, NULL, NULL);
    return;
  }
#endif
  fprintf(stderr, "[ERR]: %s\n", str);
}

static void print_msg(const char* str) {
#ifdef HAS_PLYMOUTH
  if (plymouth_client != NULL && is_plymouth_connected) {
    ply_boot_client_tell_daemon_to_display_message(plymouth_client, str, NULL, NULL, NULL);
    return;
  }
#endif
  fprintf(stdout, "[MSG]: %s\n", str);
}

static void cleanup() {
#ifdef HAS_PLYMOUTH
  if (plymouth_client != NULL) {
    if (is_plymouth_connected) ply_boot_client_disconnect(plymouth_client);
    is_plymouth_connected = false;
    ply_boot_client_free(plymouth_client);
  }
#endif
}

int main(int argc, char** argv) {
#ifdef HAS_PLYMOUTH
  plymouth_client = ply_boot_client_new();
  if (plymouth_client == NULL) {
    fprintf(stderr, "[ERR]: failed to connect to plymouth\n");
  } else {
    is_plymouth_connected = ply_boot_client_connect(plymouth_client, plymouth_client_disconnect, NULL);
  }

  if (is_plymouth_connected) {
    fprintf(stdout, "[MSG]: connected to plymouth\n");
  } else {
    fprintf(stderr, "[WARN]: not connected to plymouth\n");
  }
#endif

  NtBacktrace* bt = nt_backtrace_new();
  nt_backtrace_push(bt, main);

  NtError* error = NULL;

  NtTypeArgument* config = expidus_vendor_config_load(bt, &error);
  if (config == NULL) {
    NtString* str = nt_error_to_string(error);
    assert(str != NULL);

    const char* s = nt_string_get_value(str, NULL);

    fprintf(stderr, "[ERR]: failed to get the vendor config: %s", s);
    free((char*)s);

    nt_type_instance_unref((NtTypeInstance*)str);
    nt_type_instance_unref((NtTypeInstance*)bt);
    cleanup();
    return EXIT_FAILURE;
  }

  nt_type_instance_unref((NtTypeInstance*)bt);
  cleanup();
  return EXIT_SUCCESS;
}
