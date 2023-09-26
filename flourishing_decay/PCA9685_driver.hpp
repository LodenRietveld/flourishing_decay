#ifndef _PCA9685_H
#define _PCA9685_H

#include "./I2CDevice.hpp"
#include <Arduino.h>
#include <Softwire.h>

// REGISTER ADDRESSES

enum PCA_CMDS : uint8_t {
    MODE1 = 0x00,       /**< Mode Register 1 */
    MODE2 = 0x01,       /**< Mode Register 2 */
    SUBADR1 = 0x02,     /**< I2C-bus subaddress 1 */
    SUBADR2 = 0x03,     /**< I2C-bus subaddress 2 */
    SUBADR3 = 0x04,     /**< I2C-bus subaddress 3 */
    ALLCALLADR = 0x05,  /**< LED All Call I2C-bus address */
    LED0_ON_L = 0x06,   /**< LED0 on tick, low byte*/
    LED0_ON_H = 0x07,   /**< LED0 on tick, high byte*/
    LED0_OFF_L = 0x08,  /**< LED0 off tick, low byte */
    LED0_OFF_H = 0x09,  /**< LED0 off tick, high byte */
    // etc all 16:  LED15_OFF_H 0x45
    ALLLED_ON_L = 0xFA,   /**< load all the LEDn_ON registers, low */
    ALLLED_ON_H = 0xFB,   /**< load all the LEDn_ON registers, high */
    ALLLED_OFF_L = 0xFC,  /**< load all the LEDn_OFF registers, low */
    ALLLED_OFF_H = 0xFD,  /**< load all the LEDn_OFF registers,high */
    PRESCALE = 0xFE,      /**< Prescaler for PWM output frequency */
    TESTMODE = 0xFF,      /**< defines the test mode to be entered */
};


// MODE1 bits
enum PCA_MODE1 : uint8_t {
    ALLCAL = 0x01,   /**< respond to LED All Call I2C-bus address */
    SUB3 = 0x02,     /**< respond to I2C-bus subaddress 3 */
    SUB2 = 0x04,     /**< respond to I2C-bus subaddress 2 */
    SUB1 = 0x08,     /**< respond to I2C-bus subaddress 1 */
    SLEEP = 0x10,    /**< Low power mode. Oscillator off */
    AI = 0x20,       /**< Auto-Increment enabled */
    EXTCLK = 0x40,   /**< Use EXTCLK pin clock */
    RESTART = 0x80,  /**< Restart enabled */
};

enum PCA_MODE2 : uint8_t {
   OUTNE_0 = 0x01,  /**< Active LOW output enable input */
   OUTNE_1 = 0x02,  /**< Active LOW output enable input - high impedience */
   OUTDRV = 0x04,  /**< totem pole structure vs open-drain */
   OCH = 0x08,     /**< Outputs change on ACK vs STOP */
   INVRT = 0x10,   /**< Output logic state inverted */
};

#define PCA9685_I2C_ADDRESS 0x40      /**< Default PCA9685 I2C Slave Address */
#define FREQUENCY_OSCILLATOR 25000000 /**< Int. osc. frequency in datasheet */

#define PCA9685_PRESCALE_MIN 3   /**< minimum prescale value */
#define PCA9685_PRESCALE_MAX 255 /**< maximum prescale value */

#define PCA9685_MAX 4095
#define PCA9685_MIN 0

/*!
 *  @brief  Class that stores state and functions for interacting with PCA9685
 * PWM chip
 */
class PCA9685 {
public:
  PCA9685(const uint8_t addr, SoftWire &i2c);
  bool begin(uint8_t prescale = 0);
  void reset();
  void sleep();
  void wakeup();
  void setExtClk(uint8_t prescale);
  void setPWMFreq(float freq);
  void setOutputMode(bool totempole);
  uint16_t getPWM(uint8_t num, bool off = false);
  uint8_t setPWM(uint8_t num, uint16_t on, uint16_t off);
  void setPin(uint8_t num, uint16_t val, bool invert = false);
  uint8_t readPrescale(void);
  void writeMicroseconds(uint8_t num, uint16_t Microseconds);

  void setOscillatorFrequency(uint32_t freq);
  uint32_t getOscillatorFrequency(void);

  void change_err(uint8_t err);
  void show_err();
private:
  uint8_t _i2caddr;
  I2CDevice i2c_dev; ///< Pointer to I2C bus interface
  uint8_t err = 0, _err = 0;

  uint32_t _oscillator_freq;
  uint8_t read8(uint8_t addr);
  void write8(uint8_t addr, uint8_t d);
};

#endif