#include "config.h"

extern TimerObject *sensor_timer;

int wifi_setup()
{
  int i;

  Serial << "wifi_setup: Connecting to " << NETWORK_SSID << "\r\n";
  if (WiFi.isConnected()) {
    Serial << "wifi_setup: Connected, disconnecting first\r\n";
    WiFi.disconnect();
  }
  WiFi.mode(WIFI_STA);
  WiFi.begin(NETWORK_SSID, NETWORK_PSK);
  for (i = 0; i < 60 && WiFi.status() != WL_CONNECTED; i++) {
    Serial << "wifi_setup: Waiting to connect... " << i << "\r\n";
    delay(500);
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial << "wifi_setup: Failed to connect!\r\n";
    return -1;
  }
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);

  Serial << "wifi_setup: Connected to " << WiFi.SSID() << " RSSI=" << WiFi.RSSI() << "\r\n";
  Serial << "wifi_setup: MAC=" << WiFi.macAddress() << " on BSSID " << WiFi.BSSIDstr() << "\r\n";
  Serial << "wifi_setup: localIP=" << WiFi.localIP() << "/" << WiFi.subnetMask() << "\r\n";
  Serial << "wifi_setup: gatewayIP=" << WiFi.gatewayIP() << " dnsIP=" << WiFi.dnsIP() << "\r\n";
  return 0;
}

void setup()
{
  serial_setup();

  sensor_setup();

  wifi_setup();

  webserver_setup();
}

void loop() 
{
  serial_handle();
  webserver_handle();
  sensor_timer->Update();
}
