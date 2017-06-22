#include "config.h"
#include "adcunit.h"

// These are the I2C pins on ESP8266 (NodeMCU v1.0)
// D1 = SCL
// D2 = SDA

ADCUnit *adc_unit[ADCUNIT_MAX];
uint8_t num_adcunit = 0;

TimerObject *_sensor_timer;
static uint32_t sensor_count = 0;
static uint64_t sensor_msec_total = 0;
static uint32_t sensor_errors = 0;

static void sensor_timer();

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
  _sensor_timer = new TimerObject(SENSOR_PERIOD);
  _sensor_timer->setOnTimer(&sensor_timer);
  _sensor_timer->Start();
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
  ret += adc_unit[unit]->getAverage(ADCUNIT_VOLTAGE_INPUT);
  ret += "\n";
  ret += "current{adcunit=\"";
  ret += unit;
  ret += "\",chan=\"Battery\"} ";
  ret += adc_unit[unit]->getAverage(ADCUNIT_CURRENT_INPUT);
  ret += "\n";

  ret += "voltage{adcunit=\"";
  ret += unit;
  ret += "\",chan=\"Input\"} ";
  ret += adc_unit[unit]->getAverage(ADCUNIT_VOLTAGE_INPUT);
  ret += "\n";
  ret += "voltage{adcunit=\"";
  ret += unit;
  ret += "\",chan=\"Battery\"} ";
  ret += adc_unit[unit]->getAverage(ADCUNIT_VOLTAGE_BATTERY);
  ret += "\n";
  ret += "voltage{adcunit=\"";
  ret += unit;
  ret += "\",chan=\"Output\"} ";
  ret += adc_unit[unit]->getAverage(ADCUNIT_VOLTAGE_OUTPUT);
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

static void sensor_timer() 
{
  unsigned long _start = millis();

  sensor_sample();
  sensor_output();

  sensor_msec_total += millis() - _start;
  sensor_count++;
}

void sensor_handle() 
{
  _sensor_timer->Update();
}

