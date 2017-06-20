/*
 * Based on ADC I2C - Library for ByVac P017 www.byvac.com
 * Copyright (c) 2013 Jim Spence.  All right reserved.
 * www.byvac.com - see terms and conditions for using hardware
 * 
 * Datasheet: http://www.byvac.com/downloads/datasheets/P011%20DataSheet.pdf
 * Adapted by Pim van Pelt <pim@ipng.nl>
*/

#include "Arduino.h"
#include "p012.h"

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
