// esp32-s3-ws4-caravan-mini v1.0
/* Two extras that use the TF card:
 *  - measurements as CSV (/data.csv), one line per interval, for graphing later
 *  - settings backup and restore (/settings.json), so a flash erase isn't fatal
 * Both are started from Settings -> SD card. */
#include "app.h"
#include <ArduinoJson.h>

#define CSV_NAME "/data.csv"
#define CSV_OLD_NAME "/data-old.csv"
#define CSV_HEADER "time,uptime_s,soc,batt_v,batt_a,batt_w,solar_w,yield_kwh,temp1,temp2,temp3,outside\n"

static char csv_msg[64] = N_("Not started");

const char *csv_status() {
  return csv_msg;
}

/* One line of measurements; called from loop() via csv_service() */
static void csv_write_line() {
  if (!sd_log_ok()) {
    strlcpy(csv_msg, TR("No card mounted"), sizeof(csv_msg));
    return;
  }
  uint32_t now = millis();

  /* battery and solar, same sources as the Home tab */
  static VicCfg cfg[MAX_VIC];
  static VicData dat[MAX_VIC];
  xSemaphoreTake(vic_mtx, portMAX_DELAY);
  memcpy(cfg, vic_cfg, sizeof(cfg));
  memcpy(dat, vic_data, sizeof(dat));
  xSemaphoreGive(vic_mtx);

  float soc = NAN, bv = NAN, ba = NAN, pv = NAN, yield = NAN;
  bool pv_any = false;
  for (int i = 0; i < MAX_VIC; i++) {
    if (!cfg[i].used || !vic_fresh(dat[i], now) || !dat[i].key_ok) continue;
    if (cfg[i].type == VIC_SOLAR) {
      if (!isnan(dat[i].pv_w)) {
        pv = (pv_any ? pv : 0) + dat[i].pv_w;
        pv_any = true;
      }
      if (!isnan(dat[i].yield_kwh)) yield = (isnan(yield) ? 0 : yield) + dat[i].yield_kwh;
    } else if (cfg[i].type == VIC_BATTMON && isnan(soc)) {
      soc = dat[i].soc;
      bv = dat[i].batt_v;
      ba = dat[i].batt_i;
    }
  }

  /* temperatures */
  float t[MAX_RUUVI];
  for (int i = 0; i < MAX_RUUVI; i++) t[i] = NAN;
  if (feat_ruuvi) {
    static RuuviTag copy[MAX_TAGS];
    xSemaphoreTake(ruuvi_mtx, portMAX_DELAY);
    memcpy(copy, tags, sizeof(copy));
    xSemaphoreGive(ruuvi_mtx);
    for (int i = 0; i < MAX_RUUVI; i++) {
      if (!ruuvi_cfg[i].used) continue;
      for (int k = 0; k < MAX_TAGS; k++)
        if (copy[k].used && !strcmp(copy[k].addr, ruuvi_cfg[i].mac) && copy[k].has_temp && now - copy[k].last_seen < sensor_stale_ms(600000UL))
          t[i] = copy[k].temp;
    }
  }

  Weather w;
  LOCK();
  w = g.weather;
  UNLOCK();

  /* local time, or empty if the clock isn't synced yet */
  char ts[24] = "";
  time_t tnow = time(nullptr);
  if (tnow > 1700000000) {
    time_t lt = tnow + g.utc_offset;
    struct tm tm;
    gmtime_r(&lt, &tm);
    strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", &tm);
  }

  /* a data.csv written with other columns (by the full caravan firmware) is
     moved aside once, so the columns of this file always match its header */
  static bool header_checked = false;
  if (!header_checked && sd_fs().exists(CSV_NAME)) {
    header_checked = true;
    File old = sd_fs().open(CSV_NAME);
    String first = old ? old.readStringUntil('\n') : String();
    if (old) old.close();
    first += "\n";
    if (first != CSV_HEADER) {
      sd_fs().remove(CSV_OLD_NAME);
      sd_fs().rename(CSV_NAME, CSV_OLD_NAME);
      logf("data.csv had other columns: moved to %s", CSV_OLD_NAME);
    }
  }
  bool need_header = !sd_fs().exists(CSV_NAME);
  File f = sd_fs().open(CSV_NAME, FILE_APPEND);
  if (!f) {
    strlcpy(csv_msg, TR("Cannot write data.csv"), sizeof(csv_msg));
    return;
  }
  if (need_header) f.print(CSV_HEADER);

  char line[256];
  int n = snprintf(line, sizeof(line), "%s,%lu,", ts, (unsigned long)(now / 1000));
  auto num = [&](float v, int dec) {
    if (isnan(v)) n += snprintf(line + n, sizeof(line) - n, ",");
    else n += snprintf(line + n, sizeof(line) - n, "%.*f,", dec, v);
  };
  num(soc, 1);
  num(bv, 2);
  num(ba, 2);
  num((isnan(bv) || isnan(ba)) ? NAN : bv * ba, 0);
  num(pv, 0);
  num(yield, 2);
  num(t[0], 1);
  num(t[1], 1);
  num(t[2], 1);
  num(w.valid ? w.temp : NAN, 1);
  if (n > 0 && line[n - 1] == ',') line[n - 1] = 0;  // drop the trailing comma
  f.println(line);
  f.close();

  snprintf(csv_msg, sizeof(csv_msg), TR("Last entry %s"), ts[0] ? ts : TR("(no time yet)"));
}

/* Called from loop(): writes a line every csv_interval_min minutes */
void csv_service() {
  static uint32_t next = 0;
  if (!feat_csv) return;
  if (next && (int32_t)(millis() - next) < 0) return;
  next = millis() + (uint32_t)csv_interval_min * 60000UL;
  csv_write_line();
}

/* ---------- settings backup and restore ---------- */
static String to_hex(const uint8_t *d, size_t n) {
  String s;
  char b[3];
  for (size_t i = 0; i < n; i++) {
    snprintf(b, sizeof(b), "%02x", d[i]);
    s += b;
  }
  return s;
}

static bool from_hex(const char *s, uint8_t *out, size_t n) {
  if (strlen(s) != n * 2) return false;
  auto v = [](char c) -> int {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
  };
  for (size_t i = 0; i < n; i++) {
    int h = v(s[i * 2]), l = v(s[i * 2 + 1]);
    if (h < 0 || l < 0) return false;
    out[i] = (h << 4) | l;
  }
  return true;
}

/* Writes every setting to /settings.json, including Victron encryption keys */
bool settings_backup() {
  if (!sd_log_ok()) return false;
  JsonDocument doc;
  LOCK();
  doc["ssid"] = g.ssid;
  doc["pass"] = g.pass;
  doc["city"] = g.city;
  doc["place"] = g.place;
  doc["lat"] = g.lat;
  doc["lon"] = g.lon;
  doc["hasloc"] = g.has_loc;
  doc["utcoff"] = g.utc_offset;
  doc["webpass"] = g.web_pass;
  doc["webname"] = g.web_name;
  doc["ruuvis"] = to_hex((const uint8_t *)ruuvi_cfg, sizeof(ruuvi_cfg));
  UNLOCK();

  xSemaphoreTake(vic_mtx, portMAX_DELAY);
  doc["victron"] = to_hex((const uint8_t *)vic_cfg, sizeof(vic_cfg));
  xSemaphoreGive(vic_mtx);

  doc["scan_s"] = scan_interval_s;
  doc["bl"] = bl_normal;
  doc["blsaver"] = bl_saver;
  doc["feat"] = (prefs.getUChar("feat", 0x01) & 0x1E) | (feat_ruuvi ? 0x01 : 0) | (feat_sdlog ? 0x20 : 0) | (feat_csv ? 0x40 : 0);
  doc["csvmin"] = csv_interval_min;
  doc["lang"] = ui_lang;

  File f = sd_fs().open("/settings.json", FILE_WRITE);
  if (!f) return false;
  bool ok = serializeJsonPretty(doc, f) > 0;
  f.close();
  logf("Settings backed up to /settings.json");
  return ok;
}

/* Reads /settings.json back into flash. The board restarts afterwards. */
bool settings_restore() {
  if (!sd_log_ok()) return false;
  File f = sd_fs().open("/settings.json");
  if (!f) return false;
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, f);
  f.close();
  if (err) {
    log_fault("Restore failed: %s", err.c_str());
    return false;
  }

  prefs.putString("ssid", (const char *)(doc["ssid"] | ""));
  prefs.putString("pass", (const char *)(doc["pass"] | ""));
  prefs.putString("city", (const char *)(doc["city"] | ""));
  prefs.putString("place", (const char *)(doc["place"] | ""));
  prefs.putFloat("lat", doc["lat"] | 0.0f);
  prefs.putFloat("lon", doc["lon"] | 0.0f);
  prefs.putBool("hasloc", doc["hasloc"] | false);
  prefs.putInt("utcoff", doc["utcoff"] | 0);
  prefs.putString("webpass", (const char *)(doc["webpass"] | ""));
  prefs.putString("webname", (const char *)(doc["webname"] | DEFAULT_WEB_NAME));
  prefs.putUShort("scan_s", constrain((int)(doc["scan_s"] | SCAN_MIN_S), SCAN_MIN_S, SCAN_MAX_S));
  prefs.putUChar("bl", doc["bl"] | 100);
  prefs.putUChar("blsaver", doc["blsaver"] | 10);
  prefs.putUChar("feat", doc["feat"] | 0x01);
  prefs.putUChar("csvmin", doc["csvmin"] | 5);
  prefs.putString("lang", (const char *)(doc["lang"] | "en"));

  static RuuviCfg rc[MAX_RUUVI];
  if (from_hex(doc["ruuvis"] | "", (uint8_t *)rc, sizeof(rc))) prefs.putBytes("ruuvis", rc, sizeof(rc));
  static VicCfg vc[MAX_VIC];
  if (from_hex(doc["victron"] | "", (uint8_t *)vc, sizeof(vc))) prefs.putBytes("victron", vc, sizeof(vc));

  logf("Settings restored from /settings.json - restarting");
  return true;
}
