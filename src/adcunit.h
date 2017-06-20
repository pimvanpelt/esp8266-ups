#ifndef _ADCUNIT_H
#define _ADCUNIT_H

#include "p012.h"
#include "RunningAverage.h"

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
    // Including the I2C address in case we want
    // multiple chips on the bus.
    void Initialize_EEPROM(uint8_t i2c_addr = 0x34);

  private:
    uint16_t version;
    ADCIC *adc;
    RunningAverage *adc_chan[8];
    bool digital[3];
};

#endif // _ADCUNIT_H
