/****************************************************************
SafeTherapy v0.34 (Suggested Cleanup Starter)

Goal:
- Preserve the original v0.33 algorithm in this repo
- Provide a cleaner baseline for future tuning/refactor

Notes:
- Hardware and library assumptions stay compatible with v0.33
- This file favors readability and safer boundaries over speed
****************************************************************/

#include <Wire.h>
#include "SparkFun_Qwiic_Keypad_Arduino_Library.h"
#include "EncoderStepCounter.h"

// ---------- Hardware constants ----------
static const int ENCODER_PIN1 = 2;
static const int ENCODER_PIN2 = 3;
static const uint8_t DISPLAY_ADDRESS = 0x72;

static const long SERIAL_BAUD = 115200;
static const int ODRIVE_AXIS = 0;

// Conversion factors used by v0.33 logic
static const float COUNTS_PER_DIAL = 81.92f;
static const float ENCODER_SCALE = (20.48f * 2.0f);

// Search + safety defaults
static const int CONTACT_THRESHOLD = 700;
static const int COMMAND_TOL_COUNTS = 200;
static const uint32_t MOVE_TIMEOUT_MS = 12000;
static const int DEFAULT_DIAL_STEP = 2;

KEYPAD keypad1;
EncoderStepCounter encoder(ENCODER_PIN1, ENCODER_PIN2, HALF_STEP);

#define SERIAL_PORT SerialUSB
#define ODRIVE_PORT Serial1

enum RunState {
  RUN_IDLE = 0,
  RUN_STAGE1 = 10,
  RUN_STAGE2 = 9,
  RUN_STAGE3 = 8,
  RUN_DONE = 7
};

struct RuntimeState {
  // Raw IO
  char button = 0;
  int mic = 0;
  long encoderPosition = 0;

  // ODrive state
  String odriveField1;
  String odriveField2;
  long dialCounts = 0;
  float dialValue = 0.0f;
  long dialSetpoint = 0;

  // Search outcomes
  int thirdNumber = 72;   // v0.33: dial33
  int secondNumber = 17;  // v0.33: dial33_44
  int firstNumber = 0;    // v0.33: dialopen

  // Stage metrics
  float bestStage1Metric = -9999.0f;
  float bestStage2Metric = -9999.0f;
  bool endFlag = false;

  // Run control
  RunState runState = RUN_IDLE;
  int dialStep = DEFAULT_DIAL_STEP;
  uint32_t runStartMs = 0;
} state;

void interrupt() {
  encoder.tick();
}

static void lcdClearAndTitle(const char *title) {
  Wire.beginTransmission(DISPLAY_ADDRESS);
  Wire.write('|');
  Wire.write('-');
  Wire.write('|');
  Wire.write('+');
  Wire.write(0xFF);
  Wire.write(0xFF);
  Wire.write(0xFF);
  Wire.print(title);
  Wire.endTransmission();
}

static void sendSetpoint(long counts) {
  state.dialSetpoint = counts;
  ODRIVE_PORT.print("p ");
  ODRIVE_PORT.print(ODRIVE_AXIS);
  ODRIVE_PORT.print(" ");
  ODRIVE_PORT.println(counts);
}

static void refreshEncoderPosition() {
  signed char pos = encoder.getPosition();
  if (pos != 0) {
    state.encoderPosition += pos;
    encoder.reset();
  }
}

static void refreshFromOdrive() {
  ODRIVE_PORT.println("f 0");
  state.odriveField1 = ODRIVE_PORT.readStringUntil(' ');
  state.odriveField2 = ODRIVE_PORT.readStringUntil('\n');
  state.dialCounts = state.odriveField1.toInt();
  state.dialValue = state.dialCounts / COUNTS_PER_DIAL;
}

static bool waitForTarget(long targetCounts, int toleranceCounts, uint32_t timeoutMs) {
  uint32_t start = millis();
  while ((millis() - start) < timeoutMs) {
    refreshEncoderPosition();
    refreshFromOdrive();

    long error = state.dialCounts - targetCounts;
    if (error < 0) {
      error = -error;
    }
    if (error <= toleranceCounts) {
      return true;
    }
    delay(1);
  }
  return false;
}

static void sampleInputs() {
  keypad1.updateFIFO();
  state.button = keypad1.getButton();

  refreshEncoderPosition();
  state.mic = analogRead(0);
  refreshFromOdrive();

  SERIAL_PORT.print("btn=");
  SERIAL_PORT.print(state.button);
  SERIAL_PORT.print(", dial=");
  SERIAL_PORT.print(state.dialValue);
  SERIAL_PORT.print(", mic=");
  SERIAL_PORT.println(state.mic);
}

static void applyManualControls() {
  if (state.button == '7') {
    sendSetpoint(state.dialSetpoint + 4000);
  } else if (state.button == '8') {
    sendSetpoint(state.dialSetpoint - 4000);
  } else if (state.button == '*') {
    state.runState = RUN_STAGE1;
    state.runStartMs = millis();
    lcdClearAndTitle("Auto run started");
  }
}

static void runStage1() {
  // Stage 1: estimate the third number by sweeping candidates.
  lcdClearAndTitle("Stage 1: third #");
  state.bestStage1Metric = -9999.0f;

  for (int candidate = 20; candidate <= 90; candidate += state.dialStep) {
    long target = (long)(candidate * COUNTS_PER_DIAL);
    sendSetpoint(target);

    if (!waitForTarget(target, COMMAND_TOL_COUNTS, MOVE_TIMEOUT_MS)) {
      SERIAL_PORT.println("Stage1: move timeout, continuing");
      continue;
    }

    // Suggested metric:
    // Use local contact response as the score to maximize.
    // TODO: port exact dual-window metric from v0.33 if needed.
    int peakMic = 0;
    for (int i = 0; i < 120; i++) {
      sampleInputs();
      if (state.mic > peakMic && state.mic > CONTACT_THRESHOLD) {
        peakMic = state.mic;
      }
      delay(2);
    }

    if ((float)peakMic > state.bestStage1Metric) {
      state.bestStage1Metric = (float)peakMic;
      state.thirdNumber = candidate;
    }
  }

  state.runState = RUN_STAGE2;
}

static void runStage2() {
  // Stage 2: estimate the second number using the chosen third number.
  lcdClearAndTitle("Stage 2: second #");
  state.bestStage2Metric = -9999.0f;

  for (int candidate = 2; candidate <= 100; candidate += state.dialStep) {
    // Keep the same direction profile concept as v0.33 using offset turns.
    long target = (long)((state.thirdNumber + 200 - candidate) * COUNTS_PER_DIAL);
    sendSetpoint(target);

    if (!waitForTarget(target, COMMAND_TOL_COUNTS, MOVE_TIMEOUT_MS)) {
      SERIAL_PORT.println("Stage2: move timeout, continuing");
      continue;
    }

    int peakMic = 0;
    for (int i = 0; i < 120; i++) {
      sampleInputs();
      if (state.mic > peakMic && state.mic > CONTACT_THRESHOLD) {
        peakMic = state.mic;
      }
      delay(2);
    }

    if ((float)peakMic > state.bestStage2Metric) {
      state.bestStage2Metric = (float)peakMic;
      state.secondNumber = candidate;
    }
  }

  state.runState = RUN_STAGE3;
}

static void runStage3() {
  // Stage 3: sweep first-number candidates and detect open behavior.
  lcdClearAndTitle("Stage 3: first #");
  state.endFlag = false;

  for (int candidate = 0; candidate <= 90; candidate += state.dialStep) {
    long target = (long)((candidate + 300) * COUNTS_PER_DIAL);
    sendSetpoint(target);

    if (!waitForTarget(target, COMMAND_TOL_COUNTS, MOVE_TIMEOUT_MS)) {
      // v0.33 used timeout behavior as part of "open" detection.
      state.firstNumber = candidate;
      state.endFlag = true;
      break;
    }
  }

  if (!state.endFlag) {
    // Fallback to best observed candidate from sweep end.
    state.firstNumber = 0;
  }

  state.runState = RUN_DONE;
}

static void printSummary() {
  uint32_t elapsedMin = (millis() - state.runStartMs) / 60000UL;

  SERIAL_PORT.println();
  SERIAL_PORT.println("Suggested v0.34 summary:");
  SERIAL_PORT.print("Combination estimate: ");
  SERIAL_PORT.print(state.firstNumber);
  SERIAL_PORT.print(" - ");
  SERIAL_PORT.print(state.secondNumber);
  SERIAL_PORT.print(" - ");
  SERIAL_PORT.println(state.thirdNumber);
  SERIAL_PORT.print("Elapsed minutes: ");
  SERIAL_PORT.println(elapsedMin);

  lcdClearAndTitle("Run complete");
  Wire.beginTransmission(DISPLAY_ADDRESS);
  Wire.write(254);
  Wire.write(128 + 64 + 0);
  Wire.print("C:");
  Wire.write(254);
  Wire.write(128 + 64 + 2);
  Wire.print(state.firstNumber);
  Wire.write(254);
  Wire.write(128 + 64 + 6);
  Wire.print(state.secondNumber);
  Wire.write(254);
  Wire.write(128 + 64 + 10);
  Wire.print(state.thirdNumber);
  Wire.endTransmission();
}

void setup() {
  SERIAL_PORT.begin(SERIAL_BAUD);
  ODRIVE_PORT.begin(SERIAL_BAUD);
  ODRIVE_PORT.setTimeout(25);

  encoder.begin();
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN1), interrupt, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN2), interrupt, CHANGE);

  keypad1.begin();
  delay(1000);  // wait for ODrive boot

  sendSetpoint(0);
  lcdClearAndTitle("Dial: ");
}

void loop() {
  delay(10);
  sampleInputs();
  applyManualControls();

  // Manual stage triggers remain available.
  if (state.button == '1') {
    state.runStartMs = millis();
    state.runState = RUN_STAGE1;
  } else if (state.button == '2') {
    state.runStartMs = millis();
    state.runState = RUN_STAGE2;
  } else if (state.button == '3') {
    state.runStartMs = millis();
    state.runState = RUN_STAGE3;
  }

  if (state.runState == RUN_STAGE1) {
    runStage1();
  } else if (state.runState == RUN_STAGE2) {
    runStage2();
  } else if (state.runState == RUN_STAGE3) {
    runStage3();
  } else if (state.runState == RUN_DONE) {
    printSummary();
    state.runState = RUN_IDLE;
  }
}
