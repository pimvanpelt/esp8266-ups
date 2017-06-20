#include "config.h"

static ESP8266WebServer *_webserver;

static void webserver_handleNotFound(){
  Serial << "webserver_handleNotFound: Serving 404 for " << _webserver->uri() << "\r\n";

  String message = "File Not Found\n\n";
  message += "URI: ";
  message += _webserver->uri();
  message += "\nMethod: ";
  message += (_webserver->method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += _webserver->args();
  message += "\n";
  for (uint8_t i=0; i<_webserver->args(); i++){
    message += " " + _webserver->argName(i) + ": " + _webserver->arg(i) + "\n";
  }

  _webserver->send(404, "text/plain", message);
}

static void webserver_handler_metrics()
{
  String ret;
  ret = "### helper_metrics() output\n"; 
  ret += helper_metrics();
  ret += "### sensor_metrics() output\n";
  ret += sensor_metrics();

  _webserver->send(200, "text/plain", ret);
}

static void webserver_handler_upgrade()
{
  String ans;

  ans = "Upgrading firmware from " UPDATE_FIRMWARE_URL " and rebooting.\r\n";
  _webserver->send(200, "text/plain", ans);

  helper_upgrade_firmware(UPDATE_FIRMWARE_URL, true);
}

static void webserver_handler_reboot()
{
  _webserver->send(200, "text/plain", "Rebooting.\r\n");
  helper_reboot();
}

void webserver_setup()
{
  Serial << "webserver_setup: Starting webserver on port " << WEBSERVER_PORT << "\r\n";
  _webserver = new ESP8266WebServer(WEBSERVER_PORT);
  _webserver->on("/metrics", HTTP_GET, webserver_handler_metrics);
  _webserver->on("/x/upgrade", HTTP_GET, webserver_handler_upgrade);
  _webserver->on("/x/reboot", HTTP_GET, webserver_handler_reboot);
  _webserver->begin();
}

void webserver_handle() 
{
  _webserver->handleClient();
}
