#include <Arduino.h>
#include <math.h>
#include <MD_AD9833.h>
#include <SPI.h>

// Pins
#define DATA    11      // SPI data
#define CLK     13      // SPI clock number
#define FSYNC1  28      // SPI fsync 1
#define FSYNC2  29      // SPI fsync 2

MD_AD9833	FG1(FSYNC1);  // Hardware SPI 1
MD_AD9833	FG2(FSYNC2);  // Hardware SPI 2

float f1 = 1000;
float f2 = 2;
uint16_t p1 = 0;
uint16_t p2 = 0;

void setup() {
  FG1.begin();
  //FG2.begin();
  
  FG1.setMode(MD_AD9833::MODE_SINE);
  FG1.setFrequency(MD_AD9833::CHAN_0, f1);
  //FG1.setPhase(MD_AD9833::CHAN_0, p1);

  
  //FG2.setMode(MD_AD9833::MODE_SINE);
  //FG2.setFrequency(MD_AD9833::CHAN_0, f2);
  //FG2.setPhase(MD_AD9833::CHAN_0, p2);
  
  pinMode(32, OUTPUT);

  //Serial.begin(9600);

}

void loop() {
  digitalWrite(32, HIGH);
  delay(250);
  digitalWrite(32, LOW);
  delay(250);
  Serial.println(0);
}