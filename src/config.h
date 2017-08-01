#ifndef __CONFIG_H
#define __CONFIG_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <TimerObject.h>

// Allow us to use Serial as a printable device, ie:
// Serial << "Hoi Pim\r\n";
template<class T> inline Print &operator <<(Print &obj, T arg) {
  obj.print(arg);
  return obj;
}

#define PROGNAME          "ESP8266-UPS"
#define PROGID            "$Id$"
#define NETWORK_SSID      "dapches-iot"
#define NETWORK_PSK       "marielle"
#define SENSOR_PERIOD     100                    // in ms
#define ADCUNIT_MAX       4                      
#define WEBSERVER_PORT    80
#define UPDATE_FIRMWARE_URL    "http://ghoul.ipng.nl/fw/ESP8266-UPS/firmware.bin"
#define UPDATE_SPIFFS_URL      "http://ghoul.ipng.nl/fw/ESP8266-UPS/spiffs.bin"

struct {
  uint16_t c1, c2, c3;
} _calibration_t;

// Forward declarations
void sensor_setup();
void sensor_handle();
String sensor_metrics();

void serial_setup();
void serial_handle();

void webserver_setup();
void webserver_handle();

int helper_upgrade_firmware(String, bool);
int helper_upgrade_spiffs(String, bool);
void helper_reboot();
String helper_metrics();

#endif // __CONFIG_H
