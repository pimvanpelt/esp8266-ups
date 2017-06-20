#include "config.h"

static String inputString; // Input buffer for serial


static int __cmd_help()
{
  Serial << "__cmd_help: Not implemented yet \r\n";
  return 0;
}

static int serial_command(String command)
{
  if (command.startsWith("help")) return __cmd_help();
  Serial << "serial_command: Unknown input: \"" << command << "\", " << command.length() << " chars; type 'help' for help\r\n";
}

void serial_setup()
{
  Serial.begin(115200);
  Serial << "serial_setup: Welcome to " << PROGNAME << "\r\n";
}

void serial_handle()
{
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    if (inChar == '\n' || inChar == '\r') {
      inputString.trim();
      if (inputString.length() > 0)
        serial_command(inputString);
      inputString = "";
    } else {
      inputString += inChar;
    }
  }
}
