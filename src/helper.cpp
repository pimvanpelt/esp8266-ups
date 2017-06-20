#include "config.h"
#include <ESP8266httpUpdate.h>

enum _upgrade_type {
  UPGRADE_FIRMWARE,
  UPGRADE_SPIFFS
}; 


static int upgrade(String url, bool wantreboot, _upgrade_type what) 
{
  t_httpUpdate_return ret;
  Serial << "upgrade: Fetching from " << url << "\r\n";

  ESPhttpUpdate.rebootOnUpdate(false);
  switch(what) {
    case UPGRADE_FIRMWARE:
      ret = ESPhttpUpdate.update(url);
    break;
    case UPGRADE_SPIFFS:
      ret = ESPhttpUpdate.updateSpiffs(url);
    break;
  }

  switch(ret) {
    case HTTP_UPDATE_FAILED:
      Serial << "upgrade: HTTP_UPDATE_FAILED Error (%d): %s\r\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str();
      break;
    case HTTP_UPDATE_NO_UPDATES:
      Serial << "upgrade: HTTP_UPDATE_NO_UPDATES\r\n";
      break;
    case HTTP_UPDATE_OK:
      Serial << "upgrade: HTTP_UPDATE_OK\r\n";
      break;
    default:
       Serial << "upgrade: Unknown return value " << ret << "\r\n";
  }

  if (wantreboot) 
    helper_reboot();

  return ret;
}

int helper_upgrade_firmware(String url, bool wantreboot=true)
{
  return upgrade(url, wantreboot, UPGRADE_FIRMWARE);
}

int helper_upgrade_spiffs(String url, bool wantreboot=false)
{
  return upgrade(url, wantreboot, UPGRADE_SPIFFS);
}

static void reboot()
{
  Serial << "reboot: Rebooting\r\n";
  ESP.restart();
}

void helper_reboot()
{
  reboot();
}

String helper_metrics()
{
  String ret;

  ret = "# TYPE uptime counter\nuptime ";
  ret += millis()/1000;
  ret += "\n# TYPE esp_freeheap gauge\nesp_freeheap ";
  ret += ESP.getFreeHeap();
  ret += "\n# TYPE esp_wifi_rssi gauge\nesp_wifi_rssi ";
  ret += WiFi.RSSI();
  ret += "\n# TYPE esp_sketch_size gauge\nesp_sketch_size ";
  ret += ESP.getSketchSize();
  ret += "\n# TYPE esp_sketch_md5 string\nesp_sketch_md5 ";
  ret += ESP.getSketchMD5();
  ret += "\n# TYPE esp_free_sketch_space gauge\nesp_free_sketch_space ";
  ret += ESP.getFreeSketchSpace();
  ret += "\n";

  return ret;
}
