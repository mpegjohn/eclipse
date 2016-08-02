#ifndef JobMenu_h
#define JobMenu_h

#include <Arduino.h>
#include <Button.h>
#include <LiquidCrystal.h>
#include <RotaryEncoderAcelleration.h>
#include <TicksPerSecond.h>
#include <Wire.h>

#define LCD_COLS 20
#define LCD_ROWS 4

  // Send job parameters to the other UNO
  // [0x1] -- Mode 1 comand job paremeters
  // [4 bytes] -- wire size
  // [4 bytes] -- Total turns
  // [4 bytes ] -- spool length
  // [4 bytes ] -- Turns per layer
  // [1 byte ] -- Number of whole layers
  // [4 bytes ] -- Turns last layer
#define I2C_TX_LENGTH 22

//[1 byte layer]
//[4 bytes turns]
//[4 bytes layer turns]
//[4 bytes speed]
//[1 byte] direction 1 = L to R, 0 = R to L
//[1 byte] running 1 = running, 0 - stopped
#define I2C_RX_LENGTH 15

typedef enum menuSelection { cancelSelected, okSelected } menuSelection_t;

typedef union floatbytes {
  uint8_t bytes[4];
  float value;
} Floatbyte_t;

typedef struct stackupFloats {
  uint8_t numberWholeLayers;
  Floatbyte_t turnsWholeLayer;
  Floatbyte_t turnsLastLayer;
} StackFloatBytes_t;

extern LiquidCrystal lcd;
extern Button pushButton;
extern RotaryEncoderAcelleration rotor;

void startJob(Floatbyte_t wireSize, Floatbyte_t turnsTotal,
              Floatbyte_t spoolLength, StackFloatBytes_t stackUp);

int confirm();
uint8_t *doubleToData(uint8_t *dataArray, uint8_t *pparameterData);

uint8_t *get_float_from_array(uint8_t *out_array, uint8_t *current_index);

void updateDisplay(double total_turns);
#endif
