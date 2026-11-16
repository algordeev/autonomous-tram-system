#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <VL53L0X.h>
#include <Wire.h>

VL53L0X sensor;

const int a1 = 4;
const int a2 = 3;
const int pwmA = 5;
#define gerk A2
#define gerkGnd A3

const uint64_t pipe = 0xE8E8F0F0E1LL;
RF24 radio(7, 8);

/* ---------------- Tunables ---------------- */
const uint16_t OBSTACLE_MM        = 70;    // autopilot stops closer than this
const int      PWM_CRUISE         = 55;    // autopilot speed
const int      PWM_CREEP          = 20;    // slow approach at a tram stop
const int      PWM_MANUAL_MAX     = 170;   // full lever = this PWM
const unsigned long LINK_TIMEOUT_MS = 300; // no valid packet for this long -> motor off
const unsigned long CREEP_MS      = 2000;  // slow roll after the reed switch
const unsigned long DWELL_MS      = 5000;  // standing at the stop
const uint16_t SENSOR_TIMEOUT_MS  = 50;    // max wait for one distance reading

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

Command cmd;                       // newest valid command
bool haveCmd = false;
unsigned long lastRxMs = 0;
bool sensorOk = false;

enum StationState : uint8_t { ST_RUN, ST_CREEP, ST_DWELL };
StationState stationState = ST_RUN;
unsigned long stationStart = 0;

bool commandValid(const Command &c)
{
  return c.magic == CMD_MAGIC
      && (c.flags & FLAG_RESERVED) == 0
      && c.throttle <= 1023
      && c.crc == crc8((const uint8_t *)&c, sizeof(Command) - 1);
}

void motorStop()
{
  digitalWrite(a1, LOW);
  digitalWrite(a2, LOW);
  analogWrite(pwmA, 0);
}

// dirHigh == true -> a1 LOW, a2 HIGH (remote direction input high)
// dirHigh == false -> a1 HIGH, a2 LOW (remote direction input low, and autopilot)
void drive(int pwm, bool dirHigh)
{
  analogWrite(pwmA, pwm);
  if (dirHigh) {
    digitalWrite(a1, LOW);
    digitalWrite(a2, HIGH);
  } else {
    digitalWrite(a2, LOW);
    digitalWrite(a1, HIGH);
  }
}

// True only when the sensor delivered a valid reading AND nothing is closer
// than OBSTACLE_MM. A missing sensor or a timed-out reading counts as NOT clear.
bool pathClear()
{
  if (!sensorOk) return false;
  uint16_t d = sensor.readRangeSingleMillimeters();   // laser rangefinder
  if (sensor.timeoutOccurred()) return false;
  return d > OBSTACLE_MM;
}

void controlManual()
{
  int pwm = map(cmd.throttle, 1, 1024, 0, PWM_MANUAL_MAX);
  if (cmd.throttle == 0 || pwm <= 0) motorStop();
  else drive(pwm, (cmd.flags & FLAG_REV_HI) != 0);
}

void controlAutopilot(unsigned long now)
{
  switch (stationState) {
    case ST_DWELL:                                   // standing at the stop
      motorStop();
      if (now - stationStart >= DWELL_MS) stationState = ST_RUN;
      return;

    case ST_CREEP:                                   // slow roll after the reed switch
      if (now - stationStart >= CREEP_MS) {
        motorStop();
        stationState = ST_DWELL;
        stationStart = now;
      } else if (pathClear()) {
        drive(PWM_CREEP, false);
      } else {
        motorStop();
      }
      return;

    default:
      break;
  }

  if (pathClear()) drive(PWM_CRUISE, false);
  else motorStop();

  if (analogRead(gerk) == 0) {                       // reed switch: tram stop
    stationState = ST_CREEP;
    stationStart = now;
  }
}

void setup()
{
  Wire.begin();

  for (uint8_t i = 0; i < 3 && !sensorOk; i++) {     // sensor can need a moment after power-up
    sensorOk = sensor.init();
    if (!sensorOk) delay(50);
  }
  if (sensorOk) {
    sensor.setTimeout(SENSOR_TIMEOUT_MS);
    sensor.setSignalRateLimit(0.1);
    sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange, 18);
    sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, 14);
    sensor.setMeasurementTimingBudget(20000);
  }

  delay(1000);

  radio.begin();
  radio.openReadingPipe(1, pipe);
  radio.startListening();

  pinMode(2, OUTPUT);
  pinMode(a1, OUTPUT);
  pinMode(a2, OUTPUT);
  pinMode(pwmA, OUTPUT);
  pinMode(gerk, INPUT);
  pinMode(gerkGnd, OUTPUT);
  digitalWrite(gerkGnd, LOW);
  motorStop();
}

void loop()
{
  // 1. Drain the radio, keep the newest valid command, ignore everything else.
  for (uint8_t n = 0; n < 4 && radio.available(); n++) {
    Command c;
    radio.read(&c, sizeof(c));
    if (commandValid(c)) {
      cmd = c;
      haveCmd = true;
      lastRxMs = millis();
    }
  }

  unsigned long now = millis();

  // 2. Radio-loss watchdog: no valid packet recently -> motor off.
  if (!haveCmd || now - lastRxMs > LINK_TIMEOUT_MS) {
    stationState = ST_RUN;
    motorStop();
    return;
  }

  // 3. Act on the latest command. Nothing here blocks for more than one sensor reading.
  if (cmd.flags & FLAG_STOP) {                       // stop button always wins
    stationState = ST_RUN;
    motorStop();
  } else if (cmd.flags & FLAG_AUTO) {
    controlAutopilot(now);
  } else {
    stationState = ST_RUN;
    controlManual();
  }
}
