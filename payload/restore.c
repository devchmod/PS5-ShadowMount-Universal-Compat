#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <unistd.h>

#define SHADOWMOUNT_DIR "/data/shadowmount"
#define CONFIG_PATH SHADOWMOUNT_DIR "/config.ini"
#define BACKUP_PATH SHADOWMOUNT_DIR "/config.ini.pre-universal"
#define TEMP_PATH SHADOWMOUNT_DIR "/config.ini.restore.tmp"
#define LOG_PATH SHADOWMOUNT_DIR "/universal-compat.log"

static void log_line(const char *message) {
  FILE *f = fopen(LOG_PATH, "a");
  if (f == NULL)
    return;
  fputs(message, f);
  fputc('\n', f);
  fclose(f);
}

static int copy_file(const char *src, const char *dst) {
  FILE *in = fopen(src, "rb");
  if (in == NULL)
    return -1;

  FILE *out = fopen(dst, "wb");
  if (out == NULL) {
    fclose(in);
    return -1;
  }

  char buf[16384];
  size_t n;
  int rc = 0;

  while ((n = fread(buf, 1, sizeof(buf), in)) != 0) {
    if (fwrite(buf, 1, n, out) != n) {
      rc = -1;
      break;
    }
  }

  if (ferror(in))
    rc = -1;
  if (fflush(out) != 0)
    rc = -1;

  fclose(out);
  fclose(in);
  return rc;
}

int main(void) {
  if (access(BACKUP_PATH, F_OK) != 0) {
    log_line("error: no config.ini.pre-universal backup found");
    return 1;
  }

  if (copy_file(BACKUP_PATH, TEMP_PATH) != 0) {
    log_line("error: failed to copy backup to temporary restore file");
    return 1;
  }

  if (rename(TEMP_PATH, CONFIG_PATH) != 0) {
    unlink(TEMP_PATH);
    log_line("error: failed to restore config.ini");
    return 1;
  }

  log_line("ok: original ShadowMountPlus config restored; restart/reload ShadowMountPlus");
  return 0;
}
