#include "lcd_menu.h"
/**
* @file lcd_menu.ino
* @brief The deta entry and LCD display program for the first Arduino NANO.
*
* @author J Wilkinson
* @date 25/7/2016
*
*/

static Floatbyte_t gwireSize;
static Floatbyte_t gturnsTotal;
static Floatbyte_t gspoolLength;

static long glastRotor = 0; // The laet rotor position
StackFloatBytes_t gstackup;
static boolean gnewJobSetup;

// initialize the lcd library with the numbers of the interface pins
LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

// interrupt service routine vars
boolean A_set = false;
boolean B_set = false;

RotaryEncoderAcelleration rotor;
Button pushButton;

void UpdateRotor() { rotor.update(); }

mainMenuMode_t gmenuMode = setupJobMode;

void loop()
{

	delay(100);

}

void setup() {
  // setup LCD
  lcd.begin(LCD_COLS, LCD_ROWS);

  // setup rotary encoder
  rotor.initialize(rotorPinA, rotorPinB);

  // setup push button on encode
  pushButton.initialize(buttonPin);

  // Setup Wire
  Wire.begin();

  // setup interupt for encoder
  attachInterrupt(0, UpdateRotor, CHANGE);

  // Print Splash screen to the LCD.
  lcd.setCursor(8, 1);
  lcd.print("Coil");
  lcd.setCursor(7, 2);
  lcd.print("Winder");

  delay(2000);
  lcd.clear();

  // Data Stream Test returns number of failed recieve events
  int tryCount = 0;

  // Try ro do the I2C test 3 times.
  while (int fail = DataStreamTest() && tryCount < 3) {
    lcd.clear();
    lcd.print("I2C test failed on  ");
    lcd.setCursor(0, 1);
    lcd.print(fail);
    lcd.print(" events");
    lcd.setCursor(0, 3);
    lcd.print("Retry number ");
    lcd.print(tryCount + 1);
    delay(2000);
    tryCount++;
  };

  // If we have failed 3 times stop and flash LED.
  if (tryCount >= 3) {
    while (1) {
      digitalWrite(LED, HIGH); // turn the LED on (HIGH is the voltage level)
      delay(1000);             // wait for a second
      digitalWrite(LED, LOW);  // turn the LED off by making the voltage LOW
      delay(1000);             // wait for a second
    }
  }

  gnewJobSetup = false;
  lcdMainMenu();
}

void newJob() {
  menuResult wireResult;
  menuResult turnsResult;
  menuResult spoolResult;

  wireResult = showMenu(2.00, 0.01, 0.5, 2, "mm", "Set wire size");

  if (!wireResult.status) {
    return;
  } else {
    turnsResult =
        showMenu(20000.0, 1.0, 500.0, 1, "turns", "Set number of turns");
    if (!turnsResult.status) {
      return;
    } else {
      spoolResult = showMenu(500.0, 1.0, 10.0, 1, "mm", "Set spool length");
      if (!spoolResult.status) {
        return;
      } else {
        // If we get here all data has been OK'd

        gwireSize.value = wireResult.value;
        gturnsTotal.value = turnsResult.value;
        gspoolLength.value = spoolResult.value;
      }
    }
  }

  gstackup =
      calculateStackup(gwireSize.value, gspoolLength.value, gturnsTotal.value);

  return;
}

StackFloatBytes_t calculateStackup(double wireSize, double bobbinLength,
                                   double turns) {

  StackFloatBytes_t newStack;

  int turnsPerLayer;

  double fractional, wholeLayers;

  turnsPerLayer = (int)(bobbinLength / wireSize);

  double layers = turns / (double)turnsPerLayer;

  fractional = modf(layers, &wholeLayers);

  newStack.numberWholeLayers = (uint8_t)wholeLayers;
  newStack.turnsWholeLayer.value = turnsPerLayer;
  newStack.turnsLastLayer.value = fractional * (double)turnsPerLayer;

  return newStack;
}


void lcdReview(StackFloatBytes_t stack) {
  lcd.clear();
  lcd.print("T:");
  lcd.print(gturnsTotal.value, 1);
  lcd.print(" W:");
  lcd.print(gwireSize.value, 2);

  lcd.setCursor(0, 1);
  lcd.print("Spool:");
  lcd.print(gspoolLength.value, 1);

  lcd.setCursor(0, 2);
  lcd.print("Stk:");
  lcd.print(stack.numberWholeLayers, DEC);
  lcd.print("@");
  lcd.print(stack.turnsWholeLayer.value, 1);
  lcd.print(" 1@");
  lcd.print(stack.turnsLastLayer.value, 1);
  lcd.setCursor(0, 3);
  lcd.print("    >OK");

  do {
    pushButton.update();
  } while (!pushButton.isPressed());

  return;
}

void printMainMenu() {
  rotor.setMinMax(0, startJobMode);
  rotor.setPosition(0);

  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("Setup new job");
  lcd.setCursor(1, 1);
  lcd.print("Review job");
  lcd.setCursor(1, 2);
  lcd.print("Manual Motor Control");
  lcd.setCursor(1, 3);
  lcd.print("Start");

  // Print out the cursor
  lcdPrintCursor();

  rotor.setMinMax(0, 3);
  rotor.setPosition(0);

  return;
}

void lcdMainMenu() {
  printMainMenu();

  while (1) {
    pushButton.update();
    if (pushButton.isPressed()) {
      if (gmenuMode == setupJobMode) {
        // Setup a new job
        newJob();
        printMainMenu();
      } else if (gmenuMode == reviewJobMode) {
        // Review the current job
        lcdReview(gstackup);
        printMainMenu();
      } else if (gmenuMode == startJobMode) {
        // Start the job
        startJob(gwireSize, gturnsTotal, gspoolLength, gstackup);
        printMainMenu();
      } else if (gmenuMode == manualMode) {
        manualMenu();
        printMainMenu();
      }
    }

    long pos = rotor.getPosition();

    if (glastRotor != pos) {
      switch (pos) {
      case 0: {
        gmenuMode = setupJobMode;
        break;
      };
      case 1: {
        gmenuMode = reviewJobMode;
        break;
      };
      case 2: {
        gmenuMode = manualMode;
        break;
      };
      case 3: {
        gmenuMode = startJobMode;
        break;
      };
      }
      lcdPrintCursor();
    }
    glastRotor = pos;
  };
}

// Decide which cursor position to clear based on the current menu
void lcdPrintCursor() {

  // Clear all >
  for (int i = 0; i < 4; i++) {
    lcd.setCursor(0, i);
    lcd.print(" ");
  }

  // Display the > at the correct line
  switch (gmenuMode) {
  case setupJobMode: {
    lcd.setCursor(0, 0);
    break;
  }
  case reviewJobMode: {
    lcd.setCursor(0, 1);
    break;
  }
  case manualMode: {
    lcd.setCursor(0, 2);
    break;
  }
  case startJobMode: {
    lcd.setCursor(0, 3);
    break;
  }
  }
  lcd.print(">");
}

int DataStreamTest() {
  // Command Test

  Wire.beginTransmission(8);
  Wire.write(0x00);

  // Send Data
  Wire.write(0xAA);
  Wire.write(0x55);
  Wire.write(0xFF);

  Wire.endTransmission();
  delay(100);
  // Listen for data from other side
  Wire.requestFrom(8, 3);

  int i = 0;
  int completed = 0;
  while (Wire.available()) {
    byte c = Wire.read();
    if (c == 0xAA && i == 0) {
      completed++;
    } else if (c == 0x55 && i == 1) {
      completed++;
    } else if (c == 0xFF && i == 2) {
      completed++;
    }
    i++;
  }
  return 3 - completed;
}
