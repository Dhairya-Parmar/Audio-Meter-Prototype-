#include <Audio.h> 
#include <Wire.h> 
#include <math.h> 
// ===================================================== 
// AUDIO OBJECTS 
// ===================================================== 
// Separate tone generators 
AudioSynthWaveformSine   sineToneL; 
AudioSynthWaveformSine   sineToneR; 
// White noise generator 
AudioSynthNoiseWhite     maskingNoise; 
// Mixers 
AudioMixer4              
AudioMixer4              
// Audio output 
AudioOutputI2S           
mixerL; 
mixerR; 
i2s1; 
// Audio shield control 
AudioControlSGTL5000     sgtl5000_1;
// ===================================================== 
// AUDIO CONNECTIONS 
// ===================================================== 
// LEFT CHANNEL 
AudioConnection patch1(sineToneL, 0, mixerL, 0); 
AudioConnection patch2(maskingNoise, 0, mixerL, 1); 
// RIGHT CHANNEL 
AudioConnection patch3(sineToneR, 0, mixerR, 0); 
AudioConnection patch4(maskingNoise, 0, mixerR, 1); 
// OUTPUT 
AudioConnection patch5(mixerL, 0, i2s1, 0); 
AudioConnection patch6(mixerR, 0, i2s1, 1); 
// ===================================================== 
// VARIABLES 
// ===================================================== 
int current_kHz = 1; 
float current_dB = -20; 
char currentEar = 'L'; 
// ===================================================== 
// BUTTON 
// ===================================================== 
const int buttonPin = 24; 
bool lastButtonState = HIGH; 
// ===================================================== 
// FUNCTIONS 
// ===================================================== 
// ---------- STOP ALL AUDIO ---------- 
void allSilent() { 
mixerL.gain(0, 0); 
mixerL.gain(1, 0); 
mixerR.gain(0, 0); 
mixerR.gain(1, 0); 
sineToneL.amplitude(0);
sineToneR.amplitude(0); 
} 
 
// ---------- dB TO AMPLITUDE ---------- 
float dBToAmplitude(float dB) { 
 
  float amplitude = pow(10.0, dB / 20.0); 
 
  if (amplitude > 1.0) 
    amplitude = 1.0; 
 
  if (amplitude < 0.0) 
    amplitude = 0.0; 
 
  return amplitude; 
} 
 
// ===================================================== 
// PROCESS COMMAND 
// ===================================================== 
 
void processCommand(String cmd) { 
 
  cmd.trim(); 
 
  // ================================================= 
  // STOP COMMAND 
  // ================================================= 
 
  if (cmd == "0") { 
 
    allSilent(); 
 
    Serial.println("All Silent"); 
 
    return; 
  } 
 
  // ================================================= 
  // FORMAT CHECK 
  // Example: 
  // L,5,-20 
  // ================================================= 
 
  int firstComma  = cmd.indexOf(','); 
  int secondComma = cmd.indexOf(',', firstComma + 1);
  if (firstComma == -1 || secondComma == -1) { 
 
    Serial.println("Invalid Format"); 
 
    return; 
  } 
 
  // ================================================= 
  // PARSE VALUES 
  // ================================================= 
 
  char ear = cmd.charAt(0); 
 
  int freq = 
    cmd.substring(firstComma + 1, secondComma).toInt(); 
 
  float db = 
    cmd.substring(secondComma + 1).toFloat(); 
 
  // ================================================= 
  // FREQUENCY CHECK 
  // ================================================= 
 
  if (freq < 1 || freq > 20) { 
 
    Serial.println("Invalid Frequency"); 
 
    return; 
  } 
 
  // ================================================= 
  // SAVE CURRENT VALUES 
  // ================================================= 
 
  current_kHz = freq; 
  current_dB  = db; 
 
  // ================================================= 
  // dB → AMPLITUDE 
  // ================================================= 
 
  float amp = dBToAmplitude(db); 
 
  // ================================================= 
  // LEFT EAR TEST 
  // =================================================
  if (ear == 'L' || ear == 'l') { 
 
    currentEar = 'L'; 
 
    // LEFT = PURE TONE 
    mixerL.gain(0, 1.0); 
    mixerL.gain(1, 0.0); 
 
    // RIGHT = WHITE NOISE 
    mixerR.gain(0, 0.0); 
    mixerR.gain(1, 1.0); 
 
    // LEFT TONE ACTIVE 
    sineToneL.frequency(freq * 1000); 
    sineToneL.amplitude(amp); 
 
    // RIGHT TONE OFF 
    sineToneR.amplitude(0); 
  } 
 
  // ================================================= 
  // RIGHT EAR TEST 
  // ================================================= 
 
  else if (ear == 'R' || ear == 'r') { 
 
    currentEar = 'R'; 
 
    // LEFT = WHITE NOISE 
    mixerL.gain(0, 0.0); 
    mixerL.gain(1, 1.0); 
 
    // RIGHT = PURE TONE 
    mixerR.gain(0, 1.0); 
    mixerR.gain(1, 0.0); 
 
    // RIGHT TONE ACTIVE 
    sineToneR.frequency(freq * 1000); 
    sineToneR.amplitude(amp); 
 
    // LEFT TONE OFF 
    sineToneL.amplitude(0); 
  } 
 
  else { 
 
    Serial.println("Invalid Ear");
    return; 
  } 
 
  Serial.println("Command Updated"); 
} 
 
// ===================================================== 
// SETUP 
// ===================================================== 
 
void setup() { 
 
  // Serial Monitor 
  Serial.begin(9600); 
 
  // HC-05 Bluetooth 
  Serial7.begin(9600); 
 
  // Button input 
  pinMode(buttonPin, INPUT_PULLUP); 
 
  // Audio memory 
  AudioMemory(30); 
 
  // Enable audio shield 
  sgtl5000_1.enable(); 
 
  // Headphone volume 
  sgtl5000_1.volume(0.5); 
 
  // Default frequencies 
  sineToneL.frequency(1000); 
  sineToneR.frequency(1000); 
 
  // Initially OFF 
  sineToneL.amplitude(0); 
  sineToneR.amplitude(0); 
 
  // White noise level 
  maskingNoise.amplitude(0.4); 
 
  // Start silent 
  allSilent(); 
 
  Serial.println("Audiometer Ready");
}
 
// ===================================================== 
// LOOP 
// ===================================================== 
 
void loop() { 
 
  // ================================================= 
  // BLUETOOTH COMMAND RECEIVE 
  // ================================================= 
 
  if (Serial7.available()) { 
 
    String cmd = Serial7.readStringUntil('\n'); 
 
    processCommand(cmd); 
  } 
 
  // ================================================= 
  // BUTTON DETECTION 
  // ================================================= 
 
  bool currentButtonState = digitalRead(buttonPin); 
 
  // Button pressed 
  if (currentButtonState == LOW && 
      lastButtonState == HIGH) { 
 
    // Debug 
    Serial.println("BUTTON PRESSED"); 
 
    // Create threshold data 
    String thresholdData = 
      "T," + 
      String(currentEar) + "," + 
      String(current_kHz) + "," + 
      String(current_dB); 
 
    // Send to Bluetooth app 
    Serial7.write(thresholdData.c_str()); 
    Serial7.write("\r\n"); 
 
    // Send to Serial Monitor 
    Serial.println(thresholdData); 
 
    delay(500); 
  }
  lastButtonState = currentButtonState; 
}