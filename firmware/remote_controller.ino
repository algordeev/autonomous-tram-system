#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
/*------------------------------------------------*/
#define SCREEN_W 128
#define SCREEN_H 64
#define OLED_RESET 5
Adafruit_SSD1306 display(SCREEN_W, SCREEN_H, &Wire, OLED_RESET);

#define JOYSTICK_Y A7

// Level of joystick[3] that means forward. Flip (0/1) if the screen shows the wrong direction.
#define DIR_FWD_LEVEL 0

// Minimum time between redraws, so a bouncing button can't cause several in a row.
const unsigned long UI_MIN_PERIOD_MS = 150;

const uint64_t pipe = 0xE8E8F0F0E1LL;
RF24 radio(7, 8);
int joystick[4];

bool oledOk = false;          // false if the display failed to start (driving still works)
int shownKey = -1;            // what is currently on the screen
unsigned long lastUiMs = 0;

/*------------------------------------------------*/

void drawScreen(uint8_t mode, bool fwd)
{
  display.clearDisplay();

  if (mode == 2) {                                   // STOP button active
    display.fillRect(0, 0, SCREEN_W, SCREEN_H, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK);
    display.setTextSize(3);
    display.setCursor(28, 20);
    display.print(F("STOP"));
    display.display();
    return;
  }

  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  if (mode == 1) {                                   // autopilot
    display.setCursor(40, 0);   // y 0-15 = yellow zone of two-colour OLED
    display.print(F("AUTO"));
    display.setTextSize(1);
    display.setCursor(22, 50);
    display.print(F("lever disabled"));
    display.setTextSize(3);
    display.setCursor(19, 24);
    display.print(F("PILOT"));
  } else {                                           // manual: big direction
    display.setCursor(28, 0);   // y 0-15 = yellow zone of two-colour OLED
    display.print(F("MANUAL"));
    display.setTextSize(3);
    display.setCursor(10, 32);
    display.print(fwd ? F("FWD >>") : F("<< REV"));
  }

  display.display();
}

/* Redraw only when mode, direction or STOP changes - never while just moving
 * the lever, so radio sending is not paused during normal driving. */
void updateScreen()
{
  uint8_t mode = joystick[2] ? 2 : (joystick[1] ? 1 : 0);
  bool fwd = (joystick[3] == DIR_FWD_LEVEL);

  int key = (mode << 1) | fwd;
  if (key == shownKey) return;

  unsigned long now = millis();
  if (shownKey != -1 && now - lastUiMs < UI_MIN_PERIOD_MS) return;  // button debounce

  lastUiMs = now;
  shownKey = key;
  drawScreen(mode, fwd);
}

/*------------------------------------------------*/

void setup()
{
  oledOk = display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // I2C address 0x3C
  if (oledOk) {
    display.clearDisplay();
    display.display();
  }

  // Radio setup and packet format are unchanged from the original firmware.
  radio.begin();
  radio.openWritingPipe(pipe);
  pinMode(2, INPUT);
  pinMode(4, INPUT);
  pinMode(A7, INPUT);
  digitalWrite(2, HIGH);
  digitalWrite(4, HIGH);
  digitalWrite(6, HIGH);
}

void loop()
{
  if (digitalRead(3)) {
    joystick[0] = analogRead(JOYSTICK_Y);  // motor control
  } else {
    joystick[0] = analogRead(A6);          // motor control
  }
  joystick[1] = !digitalRead(2);           // autopilot button
  joystick[2] = !digitalRead(4);           // stop button
  joystick[3] = digitalRead(6);            // reverse button

  radio.write(joystick, sizeof(joystick)); // sent every loop, as originally

  if (oledOk) updateScreen();
}
