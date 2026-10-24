// Eye model control for Arduino Nano + PCA9685
// Prerecorded gaze data replay with two-dimensional (horizontal + vertical) control
// Three-class classification for each dimension
// Optimized for memory: data stored in Flash (PROGMEM) not RAM

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

// ==================== CONFIGURABLE PARAMETERS - HORIZONTAL ====================
// LEFT (negative values)
static const float ROTATION_H_LEFT_SMALL_DEG   = -2.0f;   // Small left movement
static const float ROTATION_H_LEFT_MEDIUM_DEG  = -8.0f;   // Medium left movement
static const float ROTATION_H_LEFT_LARGE_DEG   = -12.0f;   // Large left movement

// RIGHT (positive values)
static const float ROTATION_H_RIGHT_SMALL_DEG   = 1.0f;    // Small right movement
static const float ROTATION_H_RIGHT_MEDIUM_DEG  = 4.0f;    // Medium right movement
static const float ROTATION_H_RIGHT_LARGE_DEG   = 7.0f;    // Large right movement

// Classification thresholds for horizontal (independent from vertical)
static const float THRESHOLD_H_SMALL_MED  = 2.06f;    // Boundary between small and medium
static const float THRESHOLD_H_MED_LARGE  = 4.70f;    // Boundary between medium and large

// Base position and conversion factor
static const int BASE_H_PULSE = 450;
static const float COUNTS_PER_DEG_H = 10;

// ==================== CONFIGURABLE PARAMETERS - VERTICAL ====================
// DOWN (negative values)
static const float ROTATION_V_DOWN_SMALL_DEG   = -1.0f;    // Small down movement
static const float ROTATION_V_DOWN_MEDIUM_DEG  = -2.0f;   // Medium down movement
static const float ROTATION_V_DOWN_LARGE_DEG   = -6.0f;   // Large down movement

// UP (positive values)
static const float ROTATION_V_UP_SMALL_DEG     = 1.0f;     // Small up movement
static const float ROTATION_V_UP_MEDIUM_DEG    = 4.0f;    // Medium up movement
static const float ROTATION_V_UP_LARGE_DEG     = 8.0f;    // Large up movement

// Classification thresholds for vertical (independent from horizontal)
static const float THRESHOLD_V_SMALL_MED  = 1.85f;    // Boundary between small and medium
static const float THRESHOLD_V_MED_LARGE  = 4.20f;    // Boundary between medium and large

// Base position and conversion factor
static const int BASE_V_PULSE = 375;
static const float COUNTS_PER_DEG_V = 10;

// ==================== PCA9685 CHANNELS ====================
static const uint8_t CH_EYE_HORIZONTAL = 1;  // Horizontal (left/right)
static const uint8_t CH_EYE_VERTICAL = 2;    // Vertical (up/down)

// ==================== TIMING ====================
static const unsigned long INTERVAL_MS = 500;      // Interval between gaze updates
static const unsigned long REPLAY_PAUSE_MS = 10000; // Pause between replay cycles

// ==================== HORIZONTAL GAZE DATA (200 points) ====================
static const float gazeDataHorizontal[] PROGMEM = {
  -1.145346022323942, -2.606369011349846, -5.162198209731667, 2.443729164023171,
  1.210056288383687, -4.789633851611132, -0.642032154167772, -2.407891042058527,
  2.056147259431995, 4.310019494353178, -2.267562350232563, -0.552065094563620,
  2.972146358790134, 2.790453297355818, 1.518745033558268, 3.759276527126016,
  3.018514757498004, -1.472602627573323, -1.874033425901665, -0.986964029198695,
  -3.311811163492390, 5.383263304531063, 5.146473197105077, -17.028129499870719,
  -9.098518162450489, 9.742693412771349, 1.217951388634177, -1.576794869372144,
  -4.758098726128040, -3.965394490221087, -3.680909660020575, -3.610133775696120,
  4.995998393415974, -5.274705539658257, 4.646966186445749, -2.293640112265638,
  -3.839082492790642, 5.680954942680096, -6.288063333519219, 12.906923190383825,
  -3.530658643692998, -7.991884465828775, -4.657483453771803, 2.768963705225699,
  -6.164443234009402, 4.682878872726981, -1.899642101268270, -1.686493379102615,
  -7.152503499796435, 2.151607307903419, -6.104497139783870, 3.786474934864760,
  -4.718478807128806, 2.344591634120048, -2.611654385463737, 2.450675365760793,
  -1.832651686732957, -4.360821076730975, 1.252377768134124, -1.099815473709487,
  -2.684426811695054, -2.245446679677933, 2.785452607719346, 3.063549872785574,
  -1.225248052930475, -3.934244560414715, -0.085410309915144, -0.963257205784508,
  -4.458566320992031, -0.324454062817225, -3.622101932557452, 4.153580670757787,
  -3.203095999980750, 3.208438800472418, -2.693315722362943, -1.208946265036317,
  2.209774820992152, 3.482663004916477, -2.391041120752393, -8.271697375401324,
  1.891515046407424, 2.841838555539120, 2.803427857257697, -3.578777030060264,
  -7.980426585847049, 6.977943081710372, -10.076376931473515, -1.799884287171572,
  0.242800697120355, -2.479074670609549, -2.273625971200339, -5.495326811393831,
  1.454981350649319, 4.954912408881256, -2.138598485149881, -3.044825353848044,
  3.985944032964358, -9.931600420863461, -10.691966356497993, 2.823819567937655,
  -2.824994327592993, 2.064453340076234, 3.614257700012899, -2.511524186665536,
  -7.746506737431229, -6.126929940366644, -2.596694565854051, 2.724606809438022,
  -2.324560358697152, -1.665475897149701, -3.426937856374206, -0.731606415835177,
  1.132567202661647, 0.914769150094596, -4.706487947769688, -0.354051930760491,
  -3.446501408898403, -2.242658957110763, 0.149070484899884, 1.869031040800024,
  -1.863093770553974, 4.882416681353302, 1.103449049123455, -6.660260991405679,
  -5.836173296201267, -1.634198989605089, 3.811149885931329, 4.927858629217088,
  -0.900041364803602, 3.906512409083858, -0.697645604894468, -7.766162412067420,
  -2.143623949551912, -0.631175921640903, 0.302933064694386, -0.379840544008982,
  0.284892654244409, -1.108811376888454, -6.113586855510359, 1.234600585415219,
  -5.363290143920462, 0.945689080420575, 1.833425684263631, -3.282971675876627,
  3.319901926795467, -6.152706576333603, 1.053033208135655, -2.904391646160406,
  -3.015280401153721, -0.684309440059149, -0.646592446388976, -1.107589036581739,
  2.553169512783640, 1.210923072719911, 2.448033095190979, -8.487514037669971,
  -3.041471355521523, -1.102948769942946, 1.296636730212493, -7.057642283089049,
  2.445376657319933, -1.249085433172539, -3.477585052922052, 2.980342657697859,
  -1.149461049745774, -1.449819389848038, -2.168667051924839, -1.147910935856032,
  1.262243709495344, -1.363307175060420, 1.944649188048407, 1.375659825875648,
  -1.099049027807041, 1.403534028650203, -1.219086296882256, 0.451698047239962,
  -0.391143098409241, -1.288070173316012, 3.913439064877013, 4.914803550605628,
  -0.867002402227225, -8.896930834592087, 8.226928073876067, -1.469313727303813,
  -2.576995052651185, -1.847271744910422, 5.223376262043010, 2.367371005138303,
  -1.158144471228255, -0.976579810945930, 2.337083030617295, 2.921608579716358,
  -2.206903936606333, -2.205445349425778, 3.567081111477524, -2.493202643346969,
  4.271866987163429, -0.914088838093996, 6.058697910672876, -3.592415530204280,
  -5.283320495317721, 2.415297773039462, -1.326650352007125, 3.947400930140436,
  -1.572554775891167, 3.318991685350124, -0.488586562129587, -0.540230869099442
};

// ==================== VERTICAL GAZE DATA (208 points) ====================
// Same structure as horizontal but independent data for vertical gaze
static const float gazeDataVertical[] PROGMEM = {
  -0.591641145617329,-1.327870376476801,1.310314719683818,-0.215963753047435,-0.455176882157574,0.209478822303105,-0.025497733695582,-0.890801282971529,1.401404736354101,-0.123817168906377,-0.619142292520850,0.019887068633735,-1.476800570577012,-1.428188455222537,0.259634641717837,-0.679618064632820,-0.862665346035951,-0.347964401610666,0.101875228199012,0.335297508362487,0.237330377519300,-0.790010306714355,0.131550541606528,2.217635448145248,4.140437360898424,0.552092000199647,0.133604267327962,0.120497111041196,-1.299764848330391,-0.935030777509255,-1.432638427744925,0.834251165478801,1.449906179394765,1.126779991664157,1.708507048245549,-0.134732286128814,2.808799678446910,0.101075842034828,0.895874379096270,0.976386976391737,-0.553763639558230,0.407960413375668,0.802102676272450,-1.319766175813476,-0.594333921811140,-1.197856721689629,0.040981432026116,-0.522138449684098,-0.441026539750884,0.826508857105093,0.221841311293035,0.065390391270610,0.112541834214724,-1.191244156112292,-0.429563194769791,-1.410120012118497,0.504165719859676,-0.645890951397329,0.592802683731353,-0.715527746256723,-1.266829034177089,1.439892829946222,-2.634997672686093,-2.202293037808357,-0.160778500574924,-0.284945201687677,-0.429505896778639,0.247482440390528,0.128483188164107,-0.250173431661343,-0.373315832225650,-2.011432477240753,-0.239701875350713,0.084579784098419,-0.648557270244960,-0.913189742901097,-1.059744457050314,0.701876194962939,0.497824157977707,-0.419493739061811,0.465384483867338,-1.699572530833704,-1.573175906988501,2.668746598080390,0.556483909192676,-1.377367498620697,3.115360011531568,-1.360834623558016,-0.982038280474255,-0.781157693330157,-0.731795686631520,-3.109753202513550,-0.522803063047320,0.146148835215635,-0.699208906886867,0.388568389782161,-0.309929961107805,1.043355772460047,4.175937185068545,0.151596184045482,-0.483238826919459,-1.023845480063665,-0.969658790815247,-1.446670943595869,-1.921614796402042,3.029342999471688,-0.170915127455507,-1.218657529499325,-1.143279752551198,0.976206940550027,0.603168071466060,-0.135378249589178,-0.708709072751016,0.091461419511714,0.055093898406783,-0.006937170906521,-0.325976472373827,-1.858595516637697,-0.818323258298534,-1.052320472634952,-0.634580571143508,-2.650794833582981,-0.679001804328882,0.109832302626075,3.558978521075027,-0.303824059911659,1.610784263194149,-1.445380878098920,-0.120533229179983,-2.690973599304421,-0.440176270443567,-2.060264242746077,-1.459414929367170,0.248536023356930,-0.123785667400598,0.436359671154073,0.453779024703379,0.524306728729268,1.698322001203318,-0.445798183222490,2.680787472763357,0.613814027025672,0.922018695683048,-1.015740717086614,-0.148782137389448,0.792162356641220,-0.312977876499748,2.425256617169858,-0.158092183865074,-0.167078693053194,-0.084371766935531,0.260898387801061,0.490858618359952,-0.014710602721753,0.487510343878269,2.175449799854032,0.320550080667389,0.225393887305525,0.201169498294090,-1.247820699359380,-0.427097709178225,0.153448712494223,2.107878755839578,-1.000177227110087,-0.031397721079757,0.828645511211495,0.885855988050108,0.315060639559797,-0.405103557893437,0.498424788495223,-0.733431715465297,-0.233608748373436,0.350094798822187,-0.201700688649725,1.064746701563025,-0.031036823362184,-0.003169089992046,-0.470018318209553,0.005575628551441,-0.915041742312658,0.492217515588331,2.620571245849958,-1.635773131926503,0.200657550616216,-0.158816964054628,-0.190474420276846,-0.925382583335077,0.009936760050062,-0.101116331769214,-0.236242888343017,-0.144000462887105,0.093553685889823,0.288157940280178,1.573746582455307,-0.540606358687842,-1.092574875820723,0.305718067520569,-0.872383818867432,-0.762969354120972,-0.143177691448740,-0.527164722494165,-0.934694786274346,0.347081569260795,-0.138037793243531,-0.228700751167956,-0.595839714011099,0.045900578488536,-0.034142930562958
};

static const size_t GAZE_DATA_SIZE_H = sizeof(gazeDataHorizontal) / sizeof(gazeDataHorizontal[0]);
static const size_t GAZE_DATA_SIZE_V = sizeof(gazeDataVertical) / sizeof(gazeDataVertical[0]);

// ==================== STATE VARIABLES ====================
size_t dataIndex = 0;
unsigned long lastUpdateTime = 0;
unsigned long replayCycleStart = 0;
bool inReplayPause = false;

// ==================== HELPER FUNCTIONS ====================

// Classify horizontal gaze data (left/right) into three categories
// Does NOT use absolute value - preserves sign information
// Returns: 0=small, 1=medium, 2=large
static uint8_t classifyGazeHorizontal(float gazeValue) {
  float absValue = (gazeValue >= 0) ? gazeValue : -gazeValue;
  
  if (absValue < THRESHOLD_H_SMALL_MED) {
    return 0; // Small
  } else if (absValue < THRESHOLD_H_MED_LARGE) {
    return 1; // Medium
  } else {
    return 2; // Large
  }
}

// Map horizontal gaze classification to rotation angle (considers sign)
// Negative values map to LEFT, positive values map to RIGHT
static float getRotationAngleH(uint8_t gazeClass, float gazeValue) {
  bool isNegative = (gazeValue < 0);
  
  switch (gazeClass) {
    case 0: return isNegative ? ROTATION_H_LEFT_SMALL_DEG : ROTATION_H_RIGHT_SMALL_DEG;
    case 1: return isNegative ? ROTATION_H_LEFT_MEDIUM_DEG : ROTATION_H_RIGHT_MEDIUM_DEG;
    case 2: return isNegative ? ROTATION_H_LEFT_LARGE_DEG : ROTATION_H_RIGHT_LARGE_DEG;
    default: return 0.0f;
  }
}

// Classify vertical gaze data (up/down) into three categories
// Does NOT use absolute value - preserves sign information
// Returns: 0=small, 1=medium, 2=large
static uint8_t classifyGazeVertical(float gazeValue) {
  float absValue = (gazeValue >= 0) ? gazeValue : -gazeValue;
  
  if (absValue < THRESHOLD_V_SMALL_MED) {
    return 0; // Small
  } else if (absValue < THRESHOLD_V_MED_LARGE) {
    return 1; // Medium
  } else {
    return 2; // Large
  }
}

// Map vertical gaze classification to rotation angle (considers sign)
// Negative values map to DOWN, positive values map to UP
static float getRotationAngleV(uint8_t gazeClass, float gazeValue) {
  bool isNegative = (gazeValue < 0);
  
  switch (gazeClass) {
    case 0: return isNegative ? ROTATION_V_DOWN_SMALL_DEG : ROTATION_V_UP_SMALL_DEG;
    case 1: return isNegative ? ROTATION_V_DOWN_MEDIUM_DEG : ROTATION_V_UP_MEDIUM_DEG;
    case 2: return isNegative ? ROTATION_V_DOWN_LARGE_DEG : ROTATION_V_UP_LARGE_DEG;
    default: return 0.0f;
  }
}

// Convert rotation angle to PWM pulse value (horizontal)
static int angleToPulseH(float angleDeg) {
  int pulse = BASE_H_PULSE + (int)(angleDeg * COUNTS_PER_DEG_H + 0.5f);
  
  if (pulse < 0) return 0;
  if (pulse > 4095) return 4095;
  return pulse;
}

// Convert rotation angle to PWM pulse value (vertical)
static int angleToPulseV(float angleDeg) {
  int pulse = BASE_V_PULSE + (int)(angleDeg * COUNTS_PER_DEG_V + 0.5f);
  
  if (pulse < 0) return 0;
  if (pulse > 4095) return 4095;
  return pulse;
}

// Set horizontal position
static void setEyeHorizontalPulse(int pulse) {
  pwm.setPWM(CH_EYE_HORIZONTAL, 0, pulse);
}

// Set vertical position
static void setEyeVerticalPulse(int pulse) {
  pwm.setPWM(CH_EYE_VERTICAL, 0, pulse);
}

// ==================== ARDUINO LIFECYCLE ====================

void setup() {
  Serial.begin(9600);
  delay(100);
  
  Serial.println("\n========== 2D EYE MODEL GAZE REPLAY (HORIZONTAL + VERTICAL) ==========");
  Serial.println("Starting eye model control...");
  Serial.print("Horizontal gaze data points: ");
  Serial.println(GAZE_DATA_SIZE_H);
  Serial.print("Vertical gaze data points: ");
  Serial.println(GAZE_DATA_SIZE_V);
  
  pwm.begin();
  pwm.setPWMFreq(60);
  
  // Initialize to center position (0 degrees)
  setEyeHorizontalPulse(BASE_H_PULSE);
  setEyeVerticalPulse(BASE_V_PULSE);
  delay(500);
  
  Serial.println("Eye initialized at center position.");
  Serial.println("Starting replay...\n");
  
  lastUpdateTime = millis();
  replayCycleStart = millis();
}

void loop() {
  unsigned long currentTime = millis();
  
  // Check if in replay pause
  if (inReplayPause) {
    if (currentTime - replayCycleStart >= REPLAY_PAUSE_MS) {
      // Resume replay
      inReplayPause = false;
      dataIndex = 0;
      replayCycleStart = currentTime;
      Serial.println("\n--- REPLAY CYCLE RESTART ---\n");
    }
    delay(10);
    return;
  }
  
  // Update gaze every INTERVAL_MS
  if (currentTime - lastUpdateTime >= INTERVAL_MS) {
    lastUpdateTime = currentTime;
    
    // Use the minimum of the two dataset sizes to stay within bounds
    size_t maxIndex = (GAZE_DATA_SIZE_H < GAZE_DATA_SIZE_V) ? GAZE_DATA_SIZE_H : GAZE_DATA_SIZE_V;
    
    if (dataIndex < maxIndex) {
      // ========== HORIZONTAL CONTROL (NO DELAY) ==========
      float gazeValueH = pgm_read_float(&gazeDataHorizontal[dataIndex]);
      uint8_t gazeClassH = classifyGazeHorizontal(gazeValueH);
      float rotationAngleH = getRotationAngleH(gazeClassH, gazeValueH);
      int pulseH = angleToPulseH(rotationAngleH);
      setEyeHorizontalPulse(pulseH);
      
      // ========== VERTICAL CONTROL (IMMEDIATELY AFTER, NO DELAY) ==========
      float gazeValueV = pgm_read_float(&gazeDataVertical[dataIndex]);
      uint8_t gazeClassV = classifyGazeVertical(gazeValueV);
      float rotationAngleV = getRotationAngleV(gazeClassV, gazeValueV);
      int pulseV = angleToPulseV(rotationAngleV);
      setEyeVerticalPulse(pulseV);
      
      // Debug output (single line with direction indicators)
      Serial.print("[")
      Serial.print(dataIndex);
      Serial.print("] H: ");
      Serial.print(gazeValueH > 0 ? "R" : "L");  // Direction indicator
      Serial.print(" ");
      Serial.print(gazeValueH, 2);
      Serial.print("° Class");
      Serial.print(gazeClassH);
      Serial.print(" Angle");
      Serial.print(rotationAngleH, 1);
      Serial.print("° PWM");
      Serial.print(pulseH);
      Serial.print(" | V: ");
      Serial.print(gazeValueV > 0 ? "U" : "D");  // Direction indicator
      Serial.print(" ");
      Serial.print(gazeValueV, 2);
      Serial.print("° Class");
      Serial.print(gazeClassV);
      Serial.print(" Angle");
      Serial.print(rotationAngleV, 1);
      Serial.print("° PWM");
      Serial.println(pulseV);
      
      dataIndex++;
    } else {
      // End of replay, enter pause
      inReplayPause = true;
      replayCycleStart = currentTime;
      
      Serial.println("\n--- REPLAY COMPLETE, PAUSING 10 SECONDS ---");
      setEyeHorizontalPulse(BASE_H_PULSE);  // Return to center
      setEyeVerticalPulse(BASE_V_PULSE);    // Return to center
    }
  }
}
