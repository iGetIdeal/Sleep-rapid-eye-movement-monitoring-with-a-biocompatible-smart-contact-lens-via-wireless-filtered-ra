// Eye model control for Arduino Nano + PCA9685
// Six-level classification: -3/-2/-1 (left/down), 1/2/3 (right/up)
// Optimized for memory: data stored in Flash (PROGMEM) not RAM

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

// ==================== CONFIGURABLE PARAMETERS - HORIZONTAL ====================
// LEFT (negative values: -1/-2/-3)
static const float ROTATION_H_LEFT_LEVEL1_DEG   = -2.0f;   // -1 档 左转
static const float ROTATION_H_LEFT_LEVEL2_DEG   = -8.0f;   // -2 档 左转
static const float ROTATION_H_LEFT_LEVEL3_DEG   = -12.0f;  // -3 档 左转

// RIGHT (positive values: 1/2/3)
static const float ROTATION_H_RIGHT_LEVEL1_DEG   = 1.0f;    // 1 档 右转
static const float ROTATION_H_RIGHT_LEVEL2_DEG   = 4.0f;    // 2 档 右转
static const float ROTATION_H_RIGHT_LEVEL3_DEG   = 7.0f;    // 3 档 右转

// Base position and conversion factor
static const int BASE_H_PULSE = 450;
static const float COUNTS_PER_DEG_H = 10.0f;

// ==================== CONFIGURABLE PARAMETERS - VERTICAL ====================
// DOWN (negative values: -1/-2/-3)
static const float ROTATION_V_DOWN_LEVEL1_DEG   = -1.0f;    // -1 档 下转
static const float ROTATION_V_DOWN_LEVEL2_DEG   = -2.0f;    // -2 档 下转
static const float ROTATION_V_DOWN_LEVEL3_DEG   = -6.0f;    // -3 档 下转

// UP (positive values: 1/2/3)
static const float ROTATION_V_UP_LEVEL1_DEG     = 1.0f;     // 1 档 上转
static const float ROTATION_V_UP_LEVEL2_DEG     = 4.0f;     // 2 档 上转
static const float ROTATION_V_UP_LEVEL3_DEG     = 8.0f;     // 3 档 上转

// Base position and conversion factor
static const int BASE_V_PULSE = 375;
static const float COUNTS_PER_DEG_V = 10.0f;

// ==================== PCA9685 CHANNELS ====================
static const uint8_t CH_EYE_LEFTRIGHT = 1;
static const uint8_t CH_EYE_UPDOWN    = 0;

// ==================== TIMING ====================
static const unsigned long INTERVAL_MS = 2000;           // Interval between gaze updates
static const unsigned long REPLAY_PAUSE_MS = 10000;     // Pause between replay cycles

// ==================== GAZE DATA STORED IN FLASH ====================
// Data contains -3/-2/-1/0/1/2/3 levels
// 0 = center, -1/-2/-3 = left/down, 1/2/3 = right/up
// 未加入实际数据（见txt），超过容量
static const int8_t gazeDataHorizontal[] PROGMEM = {
  -3,-3,-2,-2,-2,-2,-2,-2,-3,-3,-3,-3,-2,-2,-2,-3,-2,-2,-3,-3,-1,-1,-1,-1,1,1,1,1,1,-1,-1,-2,-2,-3,-3,-3,-3,-3,-3,-3,-1,1,1,1,-1,1,1,-2,-2,-2,-3,-3,-3,-3,-3,-3,-2,-2,-2,-2,-1,-1,-2,-2,-2,-2,-2,-2,-2,-2,-2,1,1,1,3,3,3,3,3,3,3,3,3,3,3,2,2,2,1,-1,2,2,3,3,3,3,3,2,2,3,3,1,-1,-1,1,1,1,2,2,3,3,3,2,2,2,2,2,2,2,2,3,3,3,2,2,2,2,2,2,-1,1,1,-1,-1,1,1,-1,1,1,-2,1,1,1,1,-1,1,1,-2,1,1,1,-1,-1,-1,2,2,-1,2,2,2,2,2,2,2,2,2,1,3,3,3,3,2,3,3,1,-1,-1,1,1,-2,1,1,-1,-1,-1,-1,1,1,2,3,3,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,1
};

static const int8_t gazeDataVertical[] PROGMEM = {
  -1,1,2,2,2,2,2,2,-1,3,3,3,3,3,3,-2,2,2,-1,-1,3,3,3,3,3,3,3,3,2,3,3,2,2,-2,-2,-2,1,1,-2,-2,-2,-1,-1,-3,-1,-1,-3,-1,-1,-2,-3,-3,-3,-3,-3,-1,-1,-1,-1,-3,-3,-3,-3,-3,-3,-3,-3,-3,-3,1,1,1,2,2,1,1,1,-1,-1,-1,-3,-1,-1,-1,2,2,2,-1,1,1,1,1,1,2,3,3,2,1,1,-2,-3,-1,-1,-1,-1,-1,-1,-1,-1,1,1,2,2,2,1,2,2,2,2,1,1,1,-1,1,1,1,1,-2,1,1,1,1,1,1,2,2,1,-2,1,1,1,1,-2,2,2,-2,2,2,1,-2,-2,-2,1,1,-1,2,2,2,2,2,-1,-1,-2,-2,-2,1,2,2,1,-1,3,3,-2,-1,-1,3,3,-1,3,3,3,3,3,3,3,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,3,3,2,3,3,3
};

static const size_t GAZE_H_SIZE = sizeof(gazeDataHorizontal) / sizeof(gazeDataHorizontal[0]);
static const size_t GAZE_V_SIZE = sizeof(gazeDataVertical) / sizeof(gazeDataVertical[0]);
static const size_t MIN_GAZE_SIZE = (GAZE_H_SIZE < GAZE_V_SIZE) ? GAZE_H_SIZE : GAZE_V_SIZE;

// ==================== STATE VARIABLES ====================
size_t dataIndex = 0;
unsigned long lastUpdateTime = 0;

// 暂停/结束控制：播放结束后暂停10秒，然后彻底停止不再循环
unsigned long replayCycleStart = 0;
bool inReplayPause = false;
bool done = false;

// ==================== HELPER FUNCTIONS ====================

// Get rotation angle for horizontal movement based on six-level value
// -3/-2/-1 for left, 1/2/3 for right, 0 for center
static float getRotationAngleH(int8_t gazeLevel) {
  if (gazeLevel < 0) {
    // LEFT movement
    switch (gazeLevel) {
      case -1: return ROTATION_H_LEFT_LEVEL1_DEG;
      case -2: return ROTATION_H_LEFT_LEVEL2_DEG;
      case -3: return ROTATION_H_LEFT_LEVEL3_DEG;
      default: return 0.0f;
    }
  } else if (gazeLevel > 0) {
    // RIGHT movement
    switch (gazeLevel) {
      case 1: return ROTATION_H_RIGHT_LEVEL1_DEG;
      case 2: return ROTATION_H_RIGHT_LEVEL2_DEG;
      case 3: return ROTATION_H_RIGHT_LEVEL3_DEG;
      default: return 0.0f;
    }
  } else {
    // CENTER
    return 0.0f;
  }
}

// Get rotation angle for vertical movement based on six-level value
// -3/-2/-1 for down, 1/2/3 for up, 0 for center
static float getRotationAngleV(int8_t gazeLevel) {
  if (gazeLevel < 0) {
    // DOWN movement
    switch (gazeLevel) {
      case -1: return ROTATION_V_DOWN_LEVEL1_DEG;
      case -2: return ROTATION_V_DOWN_LEVEL2_DEG;
      case -3: return ROTATION_V_DOWN_LEVEL3_DEG;
      default: return 0.0f;
    }
  } else if (gazeLevel > 0) {
    // UP movement
    switch (gazeLevel) {
      case 1: return ROTATION_V_UP_LEVEL1_DEG;
      case 2: return ROTATION_V_UP_LEVEL2_DEG;
      case 3: return ROTATION_V_UP_LEVEL3_DEG;
      default: return 0.0f;
    }
  } else {
    // CENTER
    return 0.0f;
  }
}

// Convert rotation angle to PWM pulse value for horizontal
static int angleToPulseH(float angleDeg) {
  int pulse = BASE_H_PULSE + (int)(angleDeg * COUNTS_PER_DEG_H + 0.5f);
  if (pulse < 0) return 0;
  if (pulse > 4095) return 4095;
  return pulse;
}

// Convert rotation angle to PWM pulse value for vertical
static int angleToPulseV(float angleDeg) {
  int pulse = BASE_V_PULSE + (int)(angleDeg * COUNTS_PER_DEG_V + 0.5f);
  if (pulse < 0) return 0;
  if (pulse > 4095) return 4095;
  return pulse;
}

// Set eye positions (no delay between calls for simultaneous movement)
static void setEyePositionH(int pulse) {
  pwm.setPWM(CH_EYE_LEFTRIGHT, 0, pulse);
}

static void setEyePositionV(int pulse) {
  pwm.setPWM(CH_EYE_UPDOWN, 0, pulse);
}

// Helper to format direction string
static const char* getDirectionH(int8_t level) {
  if (level < 0) return "L";
  if (level > 0) return "R";
  return "-";
}

static const char* getDirectionV(int8_t level) {
  if (level < 0) return "D";
  if (level > 0) return "U";
  return "-";
}

// ==================== ARDUINO LIFECYCLE ====================

void setup() {
  Serial.begin(9600);
  delay(100);

  Serial.println("\n========== EYE MODEL 2D GAZE REPLAY (SIX-LEVEL) ==========");
  Serial.println("Starting eye model control...");
  Serial.print("Horizontal data points: ");
  Serial.println(GAZE_H_SIZE);
  Serial.print("Vertical data points: ");
  Serial.println(GAZE_V_SIZE);
  Serial.print("Active replay length: ");
  Serial.println(MIN_GAZE_SIZE);
  Serial.println("\nSix-level mapping: -3/-2/-1 (L/D) | 0 (center) | 1/2/3 (R/U)");

  pwm.begin();
  pwm.setPWMFreq(60);

  // Initialize to center position (0 degrees)
  setEyePositionH(BASE_H_PULSE);
  setEyePositionV(BASE_V_PULSE);
  delay(500);

  Serial.println("Eye initialized at center position.");
  Serial.println("Starting replay...\n");

  lastUpdateTime = millis();
  replayCycleStart = millis();
}

void loop() {
  if (done) {
    // 已播放完成且暂停结束：不再执行任何重播逻辑
    delay(1000);
    return;
  }

  unsigned long currentTime = millis();

  // 暂停阶段（播放结束后暂停10秒）
  if (inReplayPause) {
    if (currentTime - replayCycleStart >= REPLAY_PAUSE_MS) {
      inReplayPause = false;
      done = true; // 关键：暂停结束后标记完成，不再重播
      Serial.println("\n--- REPLAY STOPPED (NO LOOP) ---");
      // 保持在中心位置（你也可以改成别的姿态）
      setEyePositionH(BASE_H_PULSE);
      setEyePositionV(BASE_V_PULSE);
    }
    delay(10);
    return;
  }

  // 正常播放：每隔 INTERVAL_MS 更新一次
  if (currentTime - lastUpdateTime >= INTERVAL_MS) {
    lastUpdateTime = currentTime;

    if (dataIndex < MIN_GAZE_SIZE) {
      // Read horizontal and vertical levels (-3 to 3)
      int8_t gazeH = pgm_read_byte(&gazeDataHorizontal[dataIndex]);
      int8_t gazeV = pgm_read_byte(&gazeDataVertical[dataIndex]);

      // Get rotation angles
      float rotationAngleH = getRotationAngleH(gazeH);
      float rotationAngleV = getRotationAngleV(gazeV);

      // Convert to PWM pulses
      int pulseH = angleToPulseH(rotationAngleH);
      int pulseV = angleToPulseV(rotationAngleV);

      // Drive servos (horizontal first, then vertical immediately)
      setEyePositionH(pulseH);
      setEyePositionV(pulseV);

      // Debug output
      Serial.print("Index: ");
      Serial.print(dataIndex);
      Serial.print(" | H: ");
      Serial.print(getDirectionH(gazeH));
      Serial.print(abs(gazeH));
      Serial.print(" (");
      Serial.print(rotationAngleH, 1);
      Serial.print("°, PWM:");
      Serial.print(pulseH);
      Serial.print(") | V: ");
      Serial.print(getDirectionV(gazeV));
      Serial.print(abs(gazeV));
      Serial.print(" (");
      Serial.print(rotationAngleV, 1);
      Serial.print("°, PWM:");
      Serial.print(pulseV);
      Serial.println(")");

      dataIndex++;
    } else {
      // End of replay, enter one-time pause
      inReplayPause = true;
      replayCycleStart = currentTime;

      Serial.println("\n--- REPLAY COMPLETE, PAUSING 10 SECONDS ---");
      setEyePositionH(BASE_H_PULSE);
      setEyePositionV(BASE_V_PULSE);
    }
  }
}