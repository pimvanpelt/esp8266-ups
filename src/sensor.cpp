#include "config.h"
#include "adcunit.h"

// These are the I2C pins on ESP8266 (NodeMCU v1.0)
// D1 = SCL
// D2 = SDA

ADCUnit *adc_unit[ADCUNIT_MAX];
uint8_t num_adcunit = 0;

TimerObject *sensor_timer;
static uint32_t sensor_count = 0;
static uint64_t sensor_msec_total = 0;
static uint32_t sensor_errors = 0;

void sensor_setup()  {
  int addr, unit;
  // Detect ADCUnit devices. Poll the bus, and if an I2C address responds,
  // ask the device its version. If it responds to the version query, install
  // it, otherwise skip over the I2C device.

  unit = 0;
  num_adcunit = 0;
  Wire.begin();
  for (addr=1; addr<127; addr++) {
    int err, version;
    Wire.beginTransmission(addr);
    err = Wire.endTransmission();
    if (err == 0) {
      Serial << "sensor_setup: I2C device found at address " << addr << "\r\n";
      if (unit < ADCUNIT_MAX) {
        adc_unit[unit] = new ADCUnit(addr);
        version = adc_unit[unit]->getVersion();
        Serial << "sensor_setup: ADCUnit[" << unit << "] version " << version << "\r\n";
        if (!version) {
          Serial << "sensor_setup: Version is bogus, skipping device\r\n";
          delete(adc_unit[unit]);
          adc_unit[unit] = NULL;
        } else {
/* Only call once! This is to set the P012 to default settings, including an
   individual I2C address, so that we can have multiple on the bus.
   Method:
   1) Uncomment this and flash the firmware
   2) Start the system with one (1) P012 attached.
   3) EEPROM is written with the I2C address (0x34 is default, 0x33 her)
   4) Flash the previous firmware, which has this next line commented
   5) Power down the system (at least the P012), it will come up on the
      new address.

   TODO(pim): Make this a serial configurable, something like: 
      setaddr <adcunit> <i2c_addr>
*/
//          adc_unit[unit]->Initialize_EEPROM(0x33);     
          unit++;
          num_adcunit++;
        }
      }
    }
//    Serial << "sensor_setup: No I2C device at address " << addr << "\r\n";
  }
  if (0 == num_adcunit)
    Serial << "sensor_setup: Whoops, no ADCUnits found - check your I2C bus!\r\n";

  // Initialize timer
  sensor_timer = new TimerObject(SENSOR_PERIOD);
  sensor_timer->setOnTimer(&sensor_handle);
  sensor_timer->Start();
}

static void sensor_output() 
{
  static uint8_t i=0;
  uint8_t unit;

  for(unit=0; unit<ADCUNIT_MAX; unit++) {
    if (adc_unit[unit]) {
      adc_unit[unit]->setOutput(1, i&1);
      adc_unit[unit]->setOutput(2, i&2);
      adc_unit[unit]->setOutput(3, i&4);
    }
  }
  i++;
  if (i>7) i=0;
}

static String sensor_metrics_unit(int unit)
{
  String ret = "";
  if (unit >= ADCUNIT_MAX)
    return ret;
  if (!adc_unit[unit])
    return ret;

  // Map ADC values into sensible outputs
  // From the Eagle schematic:
  // adc_chan[0] - AN1 - MAX471 Current output port 1
  // adc_chan[1] - AN2 - MAX471 Current output port 2
  // adc_chan[2] - AN3 - MAX471 Current output port 3
  // adc_chan[3] - AN4 - Voltage on UPS output
  // adc_chan[4] - AN5 - Voltage on Battery
  // adc_chan[5] - AN6 - Voltage in Input (VCC)
  // adc_chan[6] - AN7 - ACS712 Current on Input (VCC)
  // adc_chan[7] - AN8 - ACS712 Current on Battery

  for (int i=1; i<=3; i++) {
    ret += "current{adcunit=\"";
    ret += unit;
    ret += "\",chan=\"";
    ret += i;
    ret += "\"} ";
    ret += adc_unit[unit]->getAverage(i);
    ret += "\n";
  }
  ret += "current{adcunit=\"";
  ret += unit;
  ret += "\",chan=\"Input\"} ";
  ret += adc_unit[unit]->getAverage(6);
  ret += "\n";
  ret += "current{adcunit=\"";
  ret += unit;
  ret += "\",chan=\"Battery\"} ";
  ret += adc_unit[unit]->getAverage(7);
  ret += "\n";

  ret += "voltage{adcunit=\"";
  ret += unit;
  ret += "\",chan=\"Input\"} ";
  ret += adc_unit[unit]->getAverage(5);
  ret += "\n";
  ret += "voltage{adcunit=\"";
  ret += unit;
  ret += "\",chan=\"Battery\"} ";
  ret += adc_unit[unit]->getAverage(4);
  ret += "\n";
  ret += "voltage{adcunit=\"";
  ret += unit;
  ret += "\",chan=\"Output\"} ";
  ret += adc_unit[unit]->getAverage(3);
  ret += "\n";

  for (int i=1; i<=3; i++) {
    ret += "adc_output{adcunit=\"";
    ret += unit;
    ret += "\",chan=\"";
    ret += i;
    ret += "\"} ";
    if (!adc_unit[unit]->getVersion())
      ret += "nan";
    else
      ret += adc_unit[unit]->getOutput(i);
    ret += "\n";
  }

  for (int i=1; i<=8; i++) {
    ret += "adc_input{adcunit=\"";
    ret += unit;
    ret += "\",chan=\"";
    ret += i;
    ret += "\"} ";
    if (!adc_unit[unit]->getVersion())
      ret += "nan";
    else
      ret += adc_unit[unit]->getLast(i);
    ret += "\n";
  }

  return ret;
}

String sensor_metrics()
{
  String ret;

  ret = "# TYPE num_adcunit gauge\n";
  ret += "num_adcunit "; 
  ret += num_adcunit;
  ret += "\n# TYPE sensor_period gauge\nsensor_period ";
  ret += SENSOR_PERIOD;
  ret += "\n# TYPE sensor_count counter\nsensor_count ";
  ret += sensor_count;
  ret += "\n# TYPE sensor_msec_total counter\nsensor_msec_total ";
  ret += (uint32_t) sensor_msec_total;
  ret += "\n# TYPE sensor_errors counter\nsensor_errors ";
  ret += sensor_errors;
  ret += "\n";
  ret += "# TYPE adc_output gauge\n";
  ret += "# TYPE adc_input gauge\n";
  ret += "# TYPE current gauge\n";
  ret += "# TYPE voltage gauge\n";

  for (int i=0; i<ADCUNIT_MAX; i++) {
    if (adc_unit[i])
      ret += sensor_metrics_unit(i);
  }

  return ret;
}

static void sensor_sample()
{
  int i;

  for(i=0; i<ADCUNIT_MAX; i++) {
    if (adc_unit[i]) {
      if (!adc_unit[i]->sample())
        sensor_errors++;
    }
  }
}

void sensor_handle() 
{
  unsigned long _start = millis();

  sensor_sample();
  sensor_output();

  sensor_msec_total += millis() - _start;
  sensor_count++;
}
