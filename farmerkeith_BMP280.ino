// created 15 Sep 2017 
// last update 20 Sep 2017

#include "farmerkeith_BMP280.h"
#include "Wire.h"
const byte ledPin = 2; // set pin D4/GPIO2 for the on-board LED
const long maxLoops = 1000; // maximum number of times around loop()
long loopCounter = 0; // variable to count times around loop()
long bmpMillis =0;
byte F3reg0; // variable to hold mirror of bmp0 register 0xF3 
byte lastF3=0; // variable to hold previous value of F3
int  F3checkCounter=0; // variable to count how many times F3 was read
const long measureDuration = 1000; // duration of each measurement
const long repeatMeasureInterval = 10000; // wait between sets of measurements
long measureTime ; // variable for the time limit on the current measurement
//double pressure0=0, temperature0=0;
long   pressure0raw=0, temperature0raw=0;
double pressure0=0, temperature0=0, pressure0Mean=0;
double pressure1=0, temperature1=0, pressure1Mean=0;
long measureCounter = 0;
byte endFlag = 0;
double deltaPmax=-1000, deltaPmin=1000, deltaPmean=0;
const double Poffset = -0.38;
double dPadj2=0, dPadj=0, dPvariance;

bmp280 bmp0(0,0); // bmp address index, debugging
bmp280 bmp1(1,0); // bmp address index, debugging

void setup() {
  Serial.begin(115200);
  Serial.println("\nStart of farmerkeith_BMP280 test sketch");
  // initialise WeMos pins
  pinMode(ledPin, OUTPUT);      // sets the digital pin as output
  digitalWrite(ledPin, LOW);    // turns the LED on
  Wire.begin(); // start I2C interface
  bmp0.begin(); // initialise bmp0
  bmp0.updateF4Control16xSleep(); // put BMP0 to sleep
  bmp1.begin(); // initialise bmp1
  bmp1.updateF4Control16xSleep(); // put BMP1 to sleep
  Serial.println (millis());
  bmpMillis = millis();
  F3reg0 = bmp0.readRegister(0xF3);    // get F3 value
  lastF3 = F3reg0;
  Serial.print ("BMP0 register 0xF3= ");
  Serial.println (F3reg0, BIN);
  bmpMillis=millis();
  bmp0.updateF4Control(1, 1, 3); //  1 temperature, 1 pressure, continuous
  bmp0.updateF5Config(0,0,0); // 0.5ms standby, IIR filter OFF, I2C
  bmp1.updateF4Control(1, 1, 3); //  1 temperature, 1 pressure, continuous
  bmp1.updateF5Config(0,0,0); // 0.5ms standby, IIR filter OFF, I2C
    Serial.print ("BMP0 and BMP1 configured 1,1,3,0,0,0. Time= ");
    Serial.print (millis() - bmpMillis);
    Serial.print (" millis= ");
    Serial.println (millis());
  measureTime = millis()+measureDuration; // set time limit for measurement

  Serial.print ("exit setup() at ");
  Serial.println (millis());
} // end of void setup()

void loop() {
  loopCounter++;
  if ((millis() < measureTime) && (endFlag==0)) {
    F3checkCounter++;
    F3reg0 = bmp0.readRegister(0xF3);    // get F3 value
    if(F3reg0!=lastF3){
//      Serial.print ("last BMP0 0xF3= ");
//      Serial.print (lastF3, BIN);
//      Serial.print (" new BMP0 0xF3= ");
//      Serial.print (F3reg0, BIN);
//      Serial.print (" lastF3b3= ");
//      Serial.print (lastF3>>3, BIN);
//      Serial.print (" Time= ");
//      Serial.print (millis() - bmpMillis);
//      Serial.print (" F3checkCounter= ");
//      Serial.print(F3checkCounter);
      if(lastF3>>3==0){
//        Serial.println (" Waiting");
        measureCounter++;
//        pressure0 = bmp0.readPressure (temperature0);
        pressure0raw = bmp0.readRawPressure (temperature0raw);
        pressure0 = bmp0.calcPressure (pressure0raw, temperature0raw, temperature0);
        pressure1 = bmp1.readPressure (temperature1);
        double deltaP = pressure1 - pressure0;
        if (deltaP < deltaPmin) deltaPmin = deltaP;
        if (deltaP > deltaPmax) deltaPmax = deltaP;
        deltaPmean += deltaP;
        dPadj += deltaP-Poffset;
        dPadj2 += (deltaP-Poffset)*(deltaP-Poffset);
        pressure0Mean += pressure0;
        
        Serial.print (" measureCounter= ");
        Serial.print(measureCounter);
        Serial.print (" press0,1, dP=");
        Serial.print (pressure0);
        Serial.print (" ");
        Serial.print (pressure1);
        Serial.print (" ");
        Serial.print (pressure1-pressure0);
        Serial.print (" temp0,1, dT=");
        Serial.print (temperature0);
        Serial.print (" ");
        Serial.print (temperature1);
        Serial.print (" ");
        Serial.println (temperature1-temperature0);
      } else {
//        Serial.println (" Measuring");
      } // end of if(lastF3>>3==1)
//      Serial.print (" ");
      bmpMillis = millis();
//      F3checkCounter=0; 
    } // end of if(F3reg0!=lastF3)
    lastF3=F3reg0;
  } //end of if ((millis() < measureTime) && (endFlag==0))

  if (millis() >= measureTime) {
    if (endFlag ==0){
      endFlag=1;
      measureTime += repeatMeasureInterval; // set up next measurement cycle
      digitalWrite(ledPin, HIGH);    // turns the LED OFF
      bmp0.updateF4Control16xSleep(); // put bmp0 back to sleep
      bmp1.updateF4Control16xSleep(); // put bmp1 back to sleep
      dPvariance = (dPadj2 - dPadj*dPadj/measureCounter)/(measureCounter-1);
      Serial.print ("DeltaP min=");
      Serial.println (deltaPmin);
      Serial.print ("DeltaP mean=");
      Serial.println (deltaPmean/measureCounter,4);
      Serial.print ("DeltaP max=");
      Serial.println (deltaPmax) ;
      Serial.print ("DeltaP adjusted mean=");
      Serial.println (dPadj/measureCounter,4);
      Serial.print ("DeltaP variance=");
      Serial.println (dPvariance,4) ;
      
      pressure0Mean /= measureCounter;
      Serial.print ("Mean pressure bmp0=");
      Serial.println(pressure0Mean); 
      Serial.print ("Calculated altitude=");
      Serial.println (bmp0.calcAltitude(pressure0Mean, 1025.8));
      Serial.print ("Calculated sea level pressure=");
      Serial.println (bmp0.calcNormalisedPressure(pressure0Mean, 644.5));
      
      Serial.println (" bmp0 and bmp1 put to sleep");
      Serial.println ("the end");
      deltaPmax=-1000;
      deltaPmin=1000;
      deltaPmean=0;
      dPadj=0;
      dPadj2=0;
      dPvariance = 0;
      measureCounter=0;
      pressure0Mean=0;
    } else { // end of if (endFlag ==0)
      endFlag=0;
      measureTime = millis()+measureDuration; // set time limit for measurement
      // wake up bmp0 and bmp1
      bmp0.updateF4Control(2, 5, 3); //  2 temperature, 16 pressure, continuous
      bmp0.updateF5Config(0,4,0); // 0.5ms standby, IIR filter 16, I2C
      bmp1.updateF4Control(2, 5, 3); //  22 temperature, 16 pressure, continuous
      bmp1.updateF5Config(0,4,0); // 0.5ms standby, IIR filter 16, I2C

//      F3reg0 = bmp0.readRegister(0xF3);    // get F3 value
//      lastF3 = F3reg0;
      Serial.print (" measureTime changed to ");
      Serial.print (measureTime);
      Serial.print (" millis=");
      Serial.println (millis());
    } // end of else part of if (endFlag ==0)
  } // end of if (millis() >= measureTime)
  
/*
  if (loopCounter == maxLoops) {
    Serial.print ("max loops reached in ");
    Serial.print (millis() - bmpMillis);
    Serial.println (" ms ");
  } // end of if (loopCounter == maxLoops)
*/

} // end of void loop()
