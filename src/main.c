#include <assert.h>
#include <expidus-launcher-build.h>
#include <libvenfig.h>
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
  NtError* error = NULL;

  const char* loc = expidus_vendor_config_get_location(EXPIDUS_VENDOR_CONFIG_SYSTEM);

  ExpidusVendorConfig* sysconfig = expidus_vendor_config_new(loc, bt, &error);
  if (error != NULL) {
    print_err("failed to read vendor config");
    nt_type_instance_unref((NtTypeInstance*)error);
    nt_type_instance_unref((NtTypeInstance*)bt);
    cleanup();
    return EXIT_FAILURE;
  }

  assert(sysconfig != NULL);

  NtValue value = expidus_vendor_config_get(sysconfig, NT_TYPE_ARGUMENT_KEY(VendorConfig, datafs));
  assert(value.type == NT_VALUE_TYPE_BOOL);

  if (value.data.boolean) {
    loc = expidus_vendor_config_get_location(EXPIDUS_VENDOR_CONFIG_DATA);

    ExpidusVendorConfig* datafs_config = expidus_vendor_config_new(loc, bt, &error);
    if (error != NULL) {
      print_err("failed to read vendor config from data file system");
      nt_type_instance_unref((NtTypeInstance*)error);
      nt_type_instance_unref((NtTypeInstance*)bt);
      cleanup();
      return EXIT_FAILURE;
    }

    assert(datafs_config != NULL);

    expidus_vendor_config_merge(sysconfig, datafs_config);
    nt_type_instance_unref((NtTypeInstance*)datafs_config);
  }

  value = expidus_vendor_config_get(sysconfig, NT_TYPE_ARGUMENT_KEY(Launcher, id));
  assert(value.type == NT_VALUE_TYPE_STRING);

  nt_type_instance_unref((NtTypeInstance*)bt);
  cleanup();
  return EXIT_SUCCESS;
}
