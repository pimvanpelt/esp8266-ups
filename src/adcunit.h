#ifndef _ADCUNIT_H
#define _ADCUNIT_H
/*
 * Based on ADC I2C - Library for ByVac P017 www.byvac.com
 * Copyright (c) 2013 Jim Spence.  All right reserved.
 * www.byvac.com - see terms and conditions for using hardware
 * 
 * Datasheet: http://www.byvac.com/downloads/datasheets/P011%20DataSheet.pdf
 * Adapted by Pim van Pelt <pim@ipng.nl>
*/

#include "RunningAverage.h"
#include "Arduino.h"
#include "Wire.h"

#define ADCIC_VREF 0
#define ADCIC_1V   1
#define ADCIC_2V   2
#define ADCIC_4V   3
#define ADCIC_VDD  4

// Map ADC values into sensible outputs (see the Eagle schematic)
#define ADCUNIT_CURRENT_CHAN1     1    // AN1 - MAX471 Current Output Port 1
#define ADCUNIT_CURRENT_CHAN2     2    // AN2 - MAX471 Current Output Port 2
#define ADCUNIT_CURRENT_CHAN3     3    // AN3 - MAX471 Current Output Port 3
#define ADCUNIT_VOLTAGE_OUTPUT    4    // AN4 - Voltage on UPS output
#define ADCUNIT_VOLTAGE_BATTERY   5    // AN5 - Voltage on Battery
#define ADCUNIT_VOLTAGE_INPUT     6    // AN6 - Voltage in Input (VCC)
#define ADCUNIT_CURRENT_INPUT     7    // AN7 - ACS712 Current on Input (VCC)
#define ADCUNIT_CURRENT_BATTERY   8    // AN8 - ACS712 Current on Battery

class ADCIC : public TwoWire
{
    public:
        /* Initialize the ADC -- there's really nothing to do, so all this does
         *  is store the i2c_addr. You can alternatively initialize the voltage
         *  reference here (see 'set_vref' method below for details)
         *  Safe defaults are: i2c_addr = 0x34; reference = ADCIC_VDD
         */
        ADCIC(uint8_t i2c_addr, uint8_t reference);

        /* Return reading from channel. Channel can be between 1 and 8
         * for ADC readings, or 9 for an internal thermometer. Returned
         * value is between 0..1023 (ie a 10-bit reading) 
         */
        uint16_t get(uint8_t channel);

        /* Returned a 16 bit integer with the I2C ADC version number
         * Most chips in the field return 0x0107 (ie version 1.7) 
         */
        uint16_t get_version();

        /* Sets digital output channel to HIGH or LOW
         * channel must be between 1 and 4
         */
        void digital(uint8_t channel, char op);

        /* Sets the Voltage Reference for the ADC to one of:
         *  ADCIC_VREF (0): Use the VREF pin
         *  ADCIC_1V   (1): Use 1.024 Volts
         *  ADCIC_2V   (2): Use 2.048 Volts
         *  ADCIC_4V   (3): Use 4.096 Volts
         *  ADCIC_VDD  (4): Use VDD pin
         *  Any other value of reference will mean "ADCIC_VDD"
         */
        void set_vref(uint8_t reference);

        /* Sets scan time in increments of 128uS
         * Valid values are [1,254] which is 128uS .. 32.5mS
         * Factory default is '8' which is 1024uS or 1ms
         */
        void set_scanrate(uint8_t n);

        /* Write EEPROM value at location */
        void eeprom_write(uint8_t loc, uint8_t val);

        /* Read EEPROM value at location */
        uint8_t eeprom_read(uint8_t loc);

    private:
        uint8_t _i2c_address;
        uint16_t i2_16bit();
};

class ADCUnit {
  public:
    ADCUnit(uint8_t i2c_addr);
    ~ADCUnit();

    // Read one sample for each ADC channel
    // Return true if successful, false if error.
    bool sample();

    uint16_t getLast(uint8_t chan);
    float getAverage(uint8_t chan);
    
    bool getOutput(uint8_t chan);
    void setOutput(uint8_t chan, bool value);

    uint16_t getVersion();

    // Write ADCUnit defaults to the P012
    void Initialize();

    // Set the I2C address in case we want
    // multiple chips on the bus. Note that a reset of the P012 is
    // required after changing the I2C address.
    void SetAddress(uint8_t i2c_addr = 0x34);

  private:
    uint16_t version;
    ADCIC *adc;
    RunningAverage *adc_chan[8];
    bool digital[3];
};

#endif // _ADCUNIT_H
