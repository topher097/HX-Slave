// Include libraries
#include <Arduino.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <Bounce2.h>
#include <sstream>
#include <vector>

// Header files
#include <defaultPiezo.h>
#include <slaveUART.h>

// Namespaces
using namespace defaultPiezoProperties;
using namespace std;

// Bounce button
Bounce restartButton = Bounce();        // Initialate restart button

// Hardware serial (UART)
#define MASTER_SERIAL Serial1

// Pins
// off-limits: 6, 7, 8, 10, 11, 12, 13, 15, 18, 19, 20, 21, 23 
#define RX1     0           // Serial 1 RX
#define TX1     1           // Serial 1 TX
#define BLINK   3           // Status LED pin
#define ENABLE1 4           // Enable driver 1
#define ENABLE2 5           // Enable driver 2
#define RESTART 14          // Restart waveform button

// Blink LED
IntervalTimer blinkTimer;   // Timer object for status LED
int ledState = LOW;         
const int blinkDelay = 250; // Blink delay in ms


// Audio driver definition
AudioSynthWaveformSineHires   sine1;          
AudioSynthWaveformSineHires   sine2;          
AudioOutputI2S                i2s1;           
AudioConnection               patchCord1(sine1, 0, i2s1, 0);
AudioConnection               patchCord2(sine2, 0, i2s1, 1);
AudioControlSGTL5000          sgtl5000_1;     

// Blink the status LED 
void blinkLED() {
  if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }
  digitalWrite(BLINK, ledState);
  //Serial.println("BLINK");
}

// When this function is called it updates the sine wave properties with the most updated values,
// so if there is a signal from the master teensy that changes a property of the wave, the output 
// signal for both channels are updated.
void modifySignal(){
  AudioNoInterrupts();
  sine1.frequency(frequency1);              // Frequency of the left channel sine wave; in Hz
  sine2.frequency(frequency2);              // Frequency of the right channel sine wave; in Hz
  sine1.amplitude(amplitude1);              // Amplitude of left channel sine wave; 0-1.0
  sine2.amplitude(amplitude2);              // Amplitude of right channel sine wave; 0-1.0
  sine1.phase(phase1);                      // Phase angle of the left channel wave; 0-360 degrees
  sine2.phase(phase2);                      // Phase angle of the right channel wave; 0-360 degrees
  AudioInterrupts();

  digitalWrite(ENABLE1, enable1);           // Enable pin for piezo driver 1
  digitalWrite(ENABLE2, enable2);           // Enable pin for piezo driver 2
}

void setup() {  
  // Pin mode setup
  pinMode(ENABLE1, OUTPUT);
  pinMode(ENABLE2, OUTPUT);
  pinMode(BLINK, OUTPUT);

  // Serial communications
  Serial.begin(9600);             // Serial for USB
  MASTER_SERIAL.begin(9600);      // Serial to connect to master teensy

  // Blink interupt
  blinkTimer.begin(blinkLED, blinkDelay*1000);      // Run the blinkLED function at delay speed
  blinkTimer.priority(1);                           // Priority 0 is highest, 255 is lowest

  // Audio driver setup
  AudioMemory(20);
  sgtl5000_1.enable();                      // Enable adio driver
  sgtl5000_1.unmuteLineout();               // Enable Line Out pins
  sgtl5000_1.lineOutLevel(14);              // Corresponds to p-p voltage of ~2.98V*amplitude
  sine1.frequency(frequency1);              // Frequency of the left channel sine wave; in Hz
  sine2.frequency(frequency2);              // Frequency of the right channel sine wave; in Hz
  sine1.amplitude(amplitude1);              // Amplitude of left channel sine wave; 0-1.0
  sine2.amplitude(amplitude2);              // Amplitude of right channel sine wave; 0-1.0
  sine1.phase(phase1);                      // Phase angle of the left channel wave; 0-360 degrees
  sine2.phase(phase2);                      // Phase angle of the right channel wave; 0-360 degrees

  // Enable the piezo drivers
  digitalWriteFast(ENABLE1, enable1);
  digitalWriteFast(ENABLE2, enable2);

  // Setup restart button
  restartButton.attach(RESTART, INPUT);     // Attach the debouncer to the pins
  restartButton.interval(25);               // Bounce delay in ms
}

void loop() {
  // To-Do:
    // Check if serial bytes are available
      // If there are, decode them
      // Save information to appropriate variables
      // call the modifySignal() function to change the signal
      // send new wave properties back to master teensy to confirm reciept 
  restartButton.update();
  if (restartButton.fell()){
    frequency1 = default_frequency1;    // Frequency of left channel piezo in Hz
    frequency2 = default_frequency2;    // Frequency of right channel piezo in Hz
    amplitude1 = default_amplitude1;    // Amplitude of sine wave 1 (left cahnnel); 0-1
    amplitude2 = default_amplitude2;    // Amplitude of sine wave 2 (right channel); 0-1
    phase1 = default_phase1;            // Phase of left channel signal in degrees
    phase2 = default_phase2;            // Phase of right channel signal in degrees
    enable1 = default_enable1;          // Enable pin for piezo driver 1
    enable2 = default_enable2;          // Enable pin for piezo driver 2
    modifySignal();
  }

  String incomingByte;
  if (MASTER_SERIAL.available() > 0) {
    incomingByte = MASTER_SERIAL.readString();
    Serial.println(incomingByte);
    decodeSlaveUART(incomingByte.c_str());                   // Decode the string sent from master teensy
    modifySignal();                                          // Modify the output signal from audio board given new properties
    MASTER_SERIAL.print("UART received:");
    MASTER_SERIAL.println(encodeSlaveUART());                // Send encoded string back to master teensy
  }
}