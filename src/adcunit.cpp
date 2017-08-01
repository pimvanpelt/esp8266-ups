#include "config.h"
#include "adcunit.h"

// ==============================================================
// I2C PRIVATE
// ==============================================================
// ==============================================================
// **************************************************************
// gets an unsigned 16 bit value from the i2c bus
// **************************************************************
uint16_t ADCIC::i2_16bit()
{
  uint16_t rv;
  Wire.beginTransmission(_i2c_address);
  Wire.requestFrom((uint8_t) _i2c_address, (uint8_t) 2); // returns 2 bytes
  rv=Wire.read()*256; // high byte
  rv+=Wire.read(); // low byte
  Wire.endTransmission();
  return rv;
}

// =============================================================================
// I2C ADC P011/2
// =============================================================================
// =============================================================================
// *****************************************************************************
// constructors
// *****************************************************************************
ADCIC::ADCIC(uint8_t i2c_addr, uint8_t reference)
{
  _i2c_address=i2c_addr;  
  Wire.begin(); // join i2c bus
  ADCIC::set_vref(reference);
}


// *****************************************************************************
// Get value from channel
// *****************************************************************************
uint16_t ADCIC::get(uint8_t channel)
{
    Wire.beginTransmission(_i2c_address);
    Wire.write(1);
    Wire.write(channel);
    Wire.endTransmission();
    return i2_16bit();
}

// *****************************************************************************
// Get version
// *****************************************************************************
uint16_t ADCIC::get_version()
{
    Wire.beginTransmission(_i2c_address);
    Wire.write(0xA0);
    Wire.endTransmission();
    return i2_16bit();
}

// *****************************************************************************
// Digital output, set either high or low, channel is a value from 0 to 3
// *****************************************************************************
void ADCIC::digital(uint8_t channel, char op)
{
    if(channel > 3) channel = 3;
    Wire.beginTransmission(_i2c_address);
    Wire.write(2);
    Wire.write(channel);
    Wire.write(op);
    Wire.endTransmission();
}

// *****************************************************************************
// Set ADCIC reference voltage
// Use ADCIC_* defines from the header file.
// *****************************************************************************
void ADCIC::set_vref(uint8_t reference)
{
    if(reference > ADCIC_VDD) reference = ADCIC_VDD;
    Wire.beginTransmission(_i2c_address);
    Wire.write(3);
    Wire.write(reference);
    Wire.endTransmission();
}

void ADCIC::eeprom_write(uint8_t loc, uint8_t val)
{
  if (loc == 0)
    return;

  // 32<=loc<=255 are user defined bytes
  Wire.beginTransmission(_i2c_address);
  Wire.write(0x91);
  Wire.write(loc);
  Wire.write(val);
  Wire.endTransmission();
}

uint8_t ADCIC::eeprom_read(uint8_t loc)
{
  uint8_t ret;

  Wire.beginTransmission(_i2c_address);
  Wire.write(0x90);
  Wire.write(loc);
  Wire.write(1);
  Wire.endTransmission();

  Wire.beginTransmission(_i2c_address);
  Wire.requestFrom((uint8_t) _i2c_address, (uint8_t) 1);
  ret=Wire.read();
  Wire.endTransmission();
  return ret;
}

ADCUnit::ADCUnit(uint8_t i2c_addr)
{
  adc = new ADCIC(i2c_addr, ADCIC_VREF);
  version = adc->get_version();
  // Versions are 0x107 (263) or 0x207 (519)
  if (version < 256 || version > 1024) {
    version = 0;
    return;
  }

  for (int i=0; i<3; i++)
    digital[i]=false;
  for (int i=0; i<8; i++)
    adc_chan[i] = new RunningAverage(20);
}

ADCUnit::~ADCUnit()
{
  for (int i=0; i<8; i++)
    if (adc_chan[i]) delete(adc_chan[i]);
  if (adc) delete(adc);
}


bool ADCUnit::sample()
{
  uint8_t i;
  uint16_t val;
  bool ret = true;

  if (!version) return false;

  for(i=0; i<8; i++) {
    val = adc->get(i+1);
    if (val > 1024)
      ret = false;
    else
      adc_chan[i]->addValue(val);
  }
  return ret;
}

uint16_t ADCUnit::getLast(uint8_t chan)
{
  if (chan<1 || chan>8 || !version)
    return 0;
  return (uint16_t) adc_chan[chan-1]->getLast();
}

float ADCUnit::getAverage(uint8_t chan)
{
  if (chan<1 || chan>8 || !version)
    return NAN;
  return adc_chan[chan-1]->getAverage();
}

bool ADCUnit::getOutput(uint8_t chan)
{
  if (chan<1 || chan>3 || !version)
    return false;
  return digital[chan-1];
}

void ADCUnit::setOutput(uint8_t chan, bool value)
{
  if (chan<1 || chan>3 || !version)
    return;
  digital[chan-1]=value;
  adc->digital(chan, value);
}

uint16_t ADCUnit::getVersion()
{
  return version;
}

/* First time setup 
 * Set ADC EEPROM values:
 *   loc=1, val=112  Device Address ([97..122])
 *   loc=2, val=6    ACK value
 *   loc=3, val=21   NACK value
 *   loc=4, val=3    Baud rate 1:2400 2:4800 3:9600 4:14400 5:19200 6:38400 7:57600 8:115200
 *   loc=5, val=1    Error Reporting 1:on
 *   loc=6, val=13   End of Line
 *   loc=7, val=1    Invert TX 1:invert
 *   loc=14, val=112 Device Address (copy)
 *   loc=16, val=8   ADC scanrate (val*128 uS)
 *   loc=17, val=2   Voltage Reference code 0:VREF 1:1.024V 2:2.048V 3:4.096V 4:VDD
 *   loc=250, val=6  Device Address (copy)
 *
 * Suggest to set these things once, and then reboot the chip
 * adc.eeprom_write(16, 39);      // 5ms (39*128 == 4992us)
 * adc.eeprom_write(17, 0);       // Use VREF pin
 *
 * Set address (pos1 and pos14, on reboot, pos 250 will be written to by the ADC)
 * adc.eeprom_write(1, 0x34);     
 * adc.eeprom_write(14, 0x34);   
 */

void ADCUnit::Initialize()
{
  adc->eeprom_write(16, 39);      // 5ms (39*128 == 4992us)
  adc->eeprom_write(17, 0);       // Use VREF pin
}

/* Note: setting I2C Address requires reboot of the chip
 */
void ADCUnit::SetAddress(uint8_t i2c_addr)
{
  adc->eeprom_write(1, i2c_addr);
  adc->eeprom_write(14, i2c_addr);
}
