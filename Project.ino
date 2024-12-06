#include <Adafruit_CircuitPlayground.h>
#include <PulseSensorPlayground.h>

// Pin and Pulse Sensor setup
#define PULSE_INPUT_PIN 7
//virtual pin allows us flexibility to choose the wiring
#define NUM_PIXELS 10
#define FIRST_BREATH_LED 1
#define LAST_BREATH_LED 8
#define BRIGHTNESS 1
// Stress Monitor Variables
PulseSensorPlayground pulseSensor;
bool alive = false;
unsigned long lastAlive;
double HRmax, HRrest;
int mode = 0; // Stress Monitor:1 = Visual, 2 = Audio
int mode2 =0; // Meditation Assistance: 1 = Audio, 2 = Visual
unsigned long startTime = 0; // Stores the start time
const unsigned long duration = 60000; // Total duration: 60 seconds
const unsigned long interval = 15000; // Interval for each quarter: 15 seconds
int bpm;
float relaxedThreshold;
float moderateThreshold;


// Meditation Assistance Variables
double moderateBPM=0; double freeBPM=0; double highBPM=0;
int breathLED = FIRST_BREATH_LED;
int prevBreathLED = FIRST_BREATH_LED;
bool breathToggle = false;
unsigned long lastBreath = 0;
unsigned long ledTimes[] = {500, 500, 500, 500, 500, 500, 500, 500, 500, 500};
int inhaleTones[] = {262, 294, 330, 349, 392, 440, 494, 523};  // Inhale tones: Saregamapadhanisa
int exhaleTones[] = {523, 494, 440, 392, 349, 330, 294, 262};  // Exhale tones: Sanidhapamagaresa

// Program state
int programMode = -1; // -1 = Menu, 1 = Stress Monitor, 2 = Meditation Assistance

void setup() {
  Serial.begin(115200);
  CircuitPlayground.begin();

  // Initialize Pulse Sensor
  pulseSensor.analogInput(PULSE_INPUT_PIN);
  pulseSensor.setThreshold(550);
  pulseSensor.begin();

  // Default light animation on startup
  lightAnimation();

  while (!(CircuitPlayground.leftButton()&& CircuitPlayground.rightButton())){
    continue;
  }
  delay(2000);
  chooseMode();
}

void loop() {
  if (programMode == 1) {
    stressMonitorLoop();
  } else if (programMode == 2) {
    meditationAssistanceLoop();
  }
  if(CircuitPlayground.leftButton()&& CircuitPlayground.rightButton()){
    delay(2000);
    programMode=-1;
    mode=0;
    mode2=0;
    setup();
  }
}

void chooseMode() {
  Serial.println("Welcome to Mart-B!");
  Serial.println("Press Left Button for Stress Monitor, Right Button for Meditation Assistance.");
  while (programMode == -1) {
    if (CircuitPlayground.leftButton()) {
      programMode = 1;
      Serial.println("Stress Monitor Selected.");
      Serial.println("Which functionality of Stress Monitor are you going to use today?");
      Serial.println("Left Button for Visual, Right button for Auditory");
      delay(2000);
      while(mode==0){
        if(CircuitPlayground.leftButton()){
          mode = 1;
          Serial.println("You selected the visual mode, yay!");
        } else if (CircuitPlayground.rightButton()){
          mode = 2;
          Serial.println("You selected the audio mode, yay!");
        }
      }
      enterAge();
      startTime = millis();
    } else if (CircuitPlayground.rightButton()) {
      programMode = 2;
      Serial.println("Meditation Assistance Selected. There are 2 modes for Meditation Assistance. Do you want to do it in peace(right button for no music) or beats?(left button for with music)");
      delay(2000);
      while(mode2==0){
        if(CircuitPlayground.leftButton()){
          mode2 = 1;
          Serial.println("You selected the audio mode, yay!");
        } else if (CircuitPlayground.rightButton()){
          mode2 = 2;
          Serial.println("You selected the non-audio mode, Good choice!");
        }
      }
      enterAge();
      startTime = millis();
    }
  }
}

void enterAge() {
  Serial.println("Please enter your age via the Serial Monitor:");
  while (Serial.available() == 0) {
    // Wait for input
  }
  int age = Serial.parseInt();
  Serial.print("Age entered: ");
  Serial.println(age);
  Serial.println("It begins!");
  // Calculate HRmax and HRrest based on age
  HRmax = 206.9 - (0.67 * age); //https://www.heartonline.org.au/resources/calculators/target-heart-rate-calculator
  if (age <= 1) {
    HRrest = 120;
  } else if (age <= 3) {
    HRrest = 110;
  } else if (age <= 5) {
    HRrest = 100;
  } else if (age <= 12) {
    HRrest = 90;
  } else if (age <= 19) {
    HRrest = 75;
  } else if (age <= 64) {
    HRrest = 72;
  } else {
    HRrest = 70;
  }
}

void stressMonitorLoop() {
  static unsigned long intervalStart = millis(); // Tracks the start of the current 15-second interval
  static unsigned long monitoringStart = millis(); // Tracks the start of the 60-second duration
  static int readingCount = 0; // Count of BPM readings in the current interval
  static int bpmSum = 0; // Sum of BPM readings in the current interval
  unsigned long currentTime = millis();

  if (pulseSensor.sawStartOfBeat()) {
    alive = true;
    lastAlive = currentTime;

    bpm = pulseSensor.getBeatsPerMinute();

    if (bpm >= 40 && bpm <= 200) { // Filter out erratic values
      Serial.println(bpm);
      bpmSum += bpm;
      readingCount++;
    }
  }

  // Check if 15 seconds have passed
  if (currentTime - intervalStart >= interval) {
    intervalStart = currentTime;

    if (readingCount > 0) {
      int avgBpm = bpmSum / readingCount;
      bpmSum = 0; // Reset for the next interval
      readingCount = 0;

      relaxedThreshold = HRrest + 0.15 * (HRmax - HRrest);
      moderateThreshold = HRrest + 0.35 * (HRmax - HRrest);

      if (mode == 1) { // Visual mode
        if (avgBpm < relaxedThreshold) {
          lightUpAllLeds(0, 255 * BRIGHTNESS, 0); // Green
        } else if (avgBpm >= relaxedThreshold && avgBpm <= moderateThreshold) {
          lightUpAllLeds(255 * BRIGHTNESS, 255 * BRIGHTNESS, 0); // Yellow
        } else {
          lightUpAllLeds(255 * BRIGHTNESS, 0, 0); // Red
        }
      } else if (mode == 2) { // Audio mode
        if (avgBpm > moderateThreshold) {
          CircuitPlayground.playTone(880, 500); // High pitch for stress
        } else {
          CircuitPlayground.playTone(440, 500); // Low pitch for calm
        }
      }

      Serial.print("Average BPM for this interval: ");
      Serial.println(avgBpm);
    }
  }
  // Check if the 60-second duration has passed
  if (currentTime - monitoringStart >= duration) {
    Serial.println("Stress monitoring complete.");
    delay(7000);
    turnOffAllLEDs();
    programMode = -1; // Return to the menu
    bpmSum=0;
    readingCount = 0;
  }

  // Handle no heartbeat detected for more than 5 seconds
  if (currentTime - lastAlive > 5000 && alive) {
    alive = false;
    Serial.println("No heartbeat detected last 5 seconds!");
  }
}

void meditationAssistanceLoop() {
  unsigned long timeNow = millis();
  float relaxedThreshold = HRrest + 0.15 * (HRmax - HRrest);
  float moderateThreshold = HRrest + 0.35 * (HRmax - HRrest);
  // Update the pulse sensor reading and check if a beat was detected
  if (pulseSensor.sawStartOfBeat()) {
    alive = true;
    lastAlive = timeNow;

    // Calculate BPM and set stress coherence rating
    int bpm = pulseSensor.getBeatsPerMinute();
    if (bpm >= 200) { // Handle false sound values
      Serial.println("Erratic value discarded!");
    } else {
      Serial.print("*** Heartbeat Detected *** BPM: ");
      Serial.println(bpm);
    }

    if (bpm < relaxedThreshold) {
      stressLights(0,255,0);
    } else if (bpm >= relaxedThreshold && bpm <= moderateThreshold) {
      stressLights(0,0,255);  // Moderate BPM indicates moderate stress
    } else {
      stressLights(255,0,0);  // High BPM indicates high stress
    }
    
    // Move the breath LED
    updateBreathLED();
  }

  // Handle no heartbeat detected for more than 5 seconds
  if (timeNow - lastAlive > 5000 && alive) {
    alive = false;
    Serial.println("No heartbeat detected in the last five seconds");
  }

  // Update LED positions based on breath timing
  unsigned long timeSince = timeNow - lastBreath;

  if (timeSince >= ledTimes[breathLED]) {
    prevBreathLED = breathLED;
    breathLED += breathToggle ? -1 : 1;

    if (breathLED < FIRST_BREATH_LED || breathLED > LAST_BREATH_LED) {
      breathToggle = !breathToggle; // Toggle between inhale and exhale
      breathLED = constrain(breathLED, FIRST_BREATH_LED, LAST_BREATH_LED);
    }
    lastBreath = timeNow;
  }

  // Play tones for the current breath phase
  if (mode2==1 && alive){
  playSoothingBreathMusic();
  }
}


void lightUpAllLeds(int red, int green, int blue) {
  for (int i = 0; i < NUM_PIXELS; i++) {
    CircuitPlayground.setPixelColor(i, red, green, blue);
  }
  CircuitPlayground.strip.show();
}

void turnOffAllLEDs() {
  for (int i = 0; i < NUM_PIXELS; i++) {
    CircuitPlayground.setPixelColor(i, 0, 0, 0);
  }
  CircuitPlayground.strip.show();
}

void updateBreathLED() {
  for (int i = FIRST_BREATH_LED; i <= LAST_BREATH_LED; i++) {
    if (i == breathLED) {
      CircuitPlayground.setPixelColor(i, 200, 200, 200);
    } else if (i == prevBreathLED) {
      CircuitPlayground.setPixelColor(i, 50, 50, 50);
    } else {
      CircuitPlayground.setPixelColor(i, 0, 0, 0);
    }
  }
  CircuitPlayground.strip.show();
}

void playSoothingBreathMusic() {
  unsigned long timeNow = millis();
  unsigned long timeSince = timeNow - lastBreath;

  // Determine tone based on breath phase and current LED
  int toneIndex = breathLED - FIRST_BREATH_LED; // Index of the current tone in the array
  int toneFreq = breathToggle ? exhaleTones[toneIndex] : inhaleTones[toneIndex];

  // Play the tone if within the current LED's timing
  if (timeSince <= ledTimes[breathLED]) {
    CircuitPlayground.playTone(toneFreq, ledTimes[breathLED] / 10); // Synchronized tones
  }
}

void lightAnimation() {
  for (int i = 0; i < 10; i++) {
    CircuitPlayground.setPixelColor(i, random(0, 255*BRIGHTNESS), random(0, 255*BRIGHTNESS), random(0, 255*BRIGHTNESS));
  }
  CircuitPlayground.strip.show();
  delay(1000);
}

void stressLights(int r, int g, int b) {
    // Gradually reduce fade rate for pulse indicator LEDs
  // Apply fade effect to pulse LEDs
  CircuitPlayground.strip.setPixelColor(0,   r / 255*BRIGHTNESS,   g / 255*BRIGHTNESS,   b / 255*BRIGHTNESS);
  CircuitPlayground.strip.setPixelColor(9,   r / 255*BRIGHTNESS,   g / 255*BRIGHTNESS,   b / 255*BRIGHTNESS);
}