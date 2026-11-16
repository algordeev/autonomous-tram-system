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

/* ---- Command packet (remote -> tram). Keep identical to remote_controller.ino ---- */
const uint8_t CMD_MAGIC    = 0xA7;
const uint8_t FLAG_AUTO    = 0x01;   // autopilot button
const uint8_t FLAG_STOP    = 0x02;   // stop button
const uint8_t FLAG_REV_HI  = 0x04;   // level of the direction input (D6)
const uint8_t FLAG_RESERVED = 0xF8;  // must be zero

struct __attribute__((packed)) Command {
  uint8_t  magic;
  uint8_t  flags;
  uint16_t throttle;   // joystick reading, 0-1023
  uint8_t  crc;        // CRC-8 (poly 0x07) over the four bytes above
};

static_assert(sizeof(Command) == 5, "Command must be exactly 5 bytes");

uint8_t crc8(const uint8_t *data, uint8_t len)
{
  uint8_t crc = 0;
  while (len--) {
    crc ^= *data++;
    for (uint8_t i = 0; i < 8; i++)
      crc = (crc & 0x80) ? (uint8_t)((crc << 1) ^ 0x07) : (uint8_t)(crc << 1);
  }
  return crc;
}
/* ---- end of shared packet definition ---- */

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

  Command pkt;                             // sent every loop, as originally
  pkt.magic    = CMD_MAGIC;
  pkt.flags    = (joystick[1] ? FLAG_AUTO : 0) | (joystick[2] ? FLAG_STOP : 0)
               | (joystick[3] ? FLAG_REV_HI : 0);
  pkt.throttle = joystick[0];
  pkt.crc      = crc8((const uint8_t *)&pkt, sizeof(pkt) - 1);
  radio.write(&pkt, sizeof(pkt));

  if (oledOk) updateScreen();
}
