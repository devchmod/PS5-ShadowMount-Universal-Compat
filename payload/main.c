#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define SHADOWMOUNT_DIR "/data/shadowmount"
#define CONFIG_PATH SHADOWMOUNT_DIR "/config.ini"
#define BACKUP_PATH SHADOWMOUNT_DIR "/config.ini.pre-universal"
#define TEMP_PATH SHADOWMOUNT_DIR "/config.ini.universal.tmp"
#define LOG_PATH SHADOWMOUNT_DIR "/universal-compat.log"

struct setting {
  const char *key;
  const char *value;
};

static const struct setting k_settings[] = {
    {"debug", "1"},
    {"mount_read_only", "1"},
    {"scan_depth", "1"},
    {"stability_wait_seconds", "10"},
    {"exfat_backend", "lvd"},
    {"ufs_backend", "lvd"},
    {"backport_fakelib", "1"},
    {"update_emulators", "1"},
    {"emulators_path", "/data/shadowmount/emus"},
    {"auto_update_ampr", "0"},
    {"global_fakelib", "1"},
    {"global_fakelib_path", "/data/shadowmount/fakelib"},
    {"global_fakelib_priority", "game"},
    {"kstuff_game_auto_toggle", "1"},
    {"kstuff_crash_detection", "1"},
    {"kstuff_pause_delay_image_seconds", "25"},
    {"kstuff_pause_delay_direct_seconds", "15"},
    {"lvd_exfat_sector_size", "512"},
    {"lvd_ufs_sector_size", "4096"},
    {"lvd_pfs_sector_size", "4096"},
};

static void log_line(const char *message) {
  FILE *f = fopen(LOG_PATH, "a");
  if (f == NULL)
    return;
  fputs(message, f);
  fputc('\n', f);
  fclose(f);
}

static int ensure_dir(const char *path) {
  if (mkdir(path, 0777) == 0 || errno == EEXIST)
    return 0;
  return -1;
}

static int copy_file(const char *src, const char *dst, bool overwrite) {
  if (!overwrite && access(dst, F_OK) == 0)
    return 0;

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

static const char *skip_space(const char *s) {
  while (*s == ' ' || *s == '\t')
    ++s;
  return s;
}

static bool line_sets_key(const char *line, const char *key) {
  const char *p = skip_space(line);

  if (*p == '#' || *p == ';' || *p == '\0' || *p == '\r' || *p == '\n')
    return false;

  size_t n = strlen(key);
  if (strncmp(p, key, n) != 0)
    return false;

  p += n;
  p = skip_space(p);
  return *p == '=';
}

static bool is_managed_line(const char *line) {
  const size_t count = sizeof(k_settings) / sizeof(k_settings[0]);

  for (size_t i = 0; i < count; ++i) {
    if (line_sets_key(line, k_settings[i].key))
      return true;
  }

  return false;
}

static int write_profile(void) {
  FILE *out = fopen(TEMP_PATH, "wb");
  if (out == NULL)
    return -1;

  FILE *in = fopen(CONFIG_PATH, "rb");
  if (in != NULL) {
    char line[2048];

    while (fgets(line, sizeof(line), in) != NULL) {
      if (!is_managed_line(line))
        fputs(line, out);
    }

    fclose(in);
  }

  fputs("\n# --- PS5-ShadowMount-Universal-Compat managed block ---\n", out);

  const size_t count = sizeof(k_settings) / sizeof(k_settings[0]);
  for (size_t i = 0; i < count; ++i)
    fprintf(out, "%s=%s\n", k_settings[i].key, k_settings[i].value);

  fputs("# Preserves per-title kstuff_no_pause/kstuff_delay, scanpath and exclusions.\n", out);
  fputs("# Do not run standalone BackPork together with backport_fakelib=1.\n", out);
  fputs("# --- end managed block ---\n", out);

  if (fflush(out) != 0) {
    fclose(out);
    unlink(TEMP_PATH);
    return -1;
  }

  if (fclose(out) != 0) {
    unlink(TEMP_PATH);
    return -1;
  }

  if (rename(TEMP_PATH, CONFIG_PATH) != 0) {
    unlink(TEMP_PATH);
    return -1;
  }

  return 0;
}

int main(void) {
  (void)ensure_dir(SHADOWMOUNT_DIR);
  (void)ensure_dir(SHADOWMOUNT_DIR "/emus");
  (void)ensure_dir(SHADOWMOUNT_DIR "/fakelib");
  (void)ensure_dir(SHADOWMOUNT_DIR "/cache");

  if (access(CONFIG_PATH, F_OK) == 0) {
    if (copy_file(CONFIG_PATH, BACKUP_PATH, false) != 0)
      log_line("warning: could not create config backup");
  }

  if (write_profile() != 0) {
    log_line("error: failed to apply universal compatibility profile");
    return 1;
  }

  log_line("ok: universal compatibility profile applied; restart/reload ShadowMountPlus");
  return 0;
}
