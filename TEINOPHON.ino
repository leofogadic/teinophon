#include <ResponsiveAnalogRead.h>
#include "untitled.h"

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

#define NUMSTRINGS 4
#define NUMCYCLES 50

untitled       violin1; //xy=594.0000076293945,417.0000057220459
untitled       violin2; //xy=594.0000076293945,470.0000057220459
untitled       violin3; //xy=596.0000076293945,527.0000057220459
untitled       violin4; //xy=597,636
AudioMixer4              mixer1;         //xy=941,342
AudioOutputI2S           i2s2;           //xy=1129,337
AudioConnection          patchCord1(violin4, 0, mixer1, 3);
AudioConnection          patchCord5(violin3, 0, mixer1, 2);
AudioConnection          patchCord6(violin2, 0, mixer1, 1);
AudioConnection          patchCord7(violin1, 0, mixer1, 0);
AudioConnection          patchCord8(mixer1, 0, i2s2, 0);
AudioConnection          patchCord9(mixer1, 0, i2s2, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=1261,341
// GUItool: end automatically generated code

untitled *violina[NUMSTRINGS] = { &violin1, &violin2, &violin3, &violin4 };





float middleA = 440;
float frequencies[2][25][7];
int frequencyIndex = 0;
int majorMinor = 0;


ResponsiveAnalogRead str1_1(A12, true);
ResponsiveAnalogRead str1_2(A13, true);
ResponsiveAnalogRead str2_1(A14, true);
ResponsiveAnalogRead str2_2(A15, true);
ResponsiveAnalogRead str3_1(A16, true);
ResponsiveAnalogRead str3_2(A17, true);
ResponsiveAnalogRead str4_1(A18, true);
ResponsiveAnalogRead str4_2(A19, true);
ResponsiveAnalogRead str5_1(A20, true);
ResponsiveAnalogRead str5_2(A21, true);
ResponsiveAnalogRead str6_1(A22, true);
ResponsiveAnalogRead str6_2(A2, true);
ResponsiveAnalogRead str7_1(A3, true);
ResponsiveAnalogRead str7_2(A7, true);

ResponsiveAnalogRead *string[7][2] = {
  { &str1_1, &str1_2 },
  { &str2_1, &str2_2 },
  { &str3_1, &str3_2 },
  { &str4_1, &str4_2 },
  { &str5_1, &str5_2 },
  { &str6_1, &str6_2 },
  { &str7_1, &str7_2 },
};

float stringValue[7][2] = {};

float flatValue[7] = {};
float stringIsFlat[7] = { true, true, true, true, true, true, true };

int countString[7] = {};
float valuesString[7][50] = {};

int stringThreshold = 5;

bool firstLoop = true;

void setup() {
  Serial.begin(9600);
  AudioMemory(10);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.1);
  
  for (int i = 0; i < NUMSTRINGS; i++) {
    for (int j = 0; j < 2; j++) {
      string[i][j]->setActivityThreshold(1);
      string[i][j]->setSnapMultiplier(0.01);
    }
  }

  int powerMajor[7] = { -33, -31, -29, -28, -26, -24, -22};
  int powerMinor[7] = { -36, -34, -33, -31, -29, -28, -25};
  for (int i = 0; i < 25; i++) {
    for (int j = 0; j < 7; j++) {
      frequencies[0][i][j] = middleA * pow(1.059463094359, powerMajor[j] + i);
      frequencies[1][i][j] = middleA * pow(1.059463094359, powerMinor[j] + i);
    }
  }


  delay(1000);

  for (int i = 0; i < NUMSTRINGS; i++)
    for (int j = 0; j < 2; j++)
      string[i][j]->update();

  delay(2000);

  for (int i = 0; i < NUMSTRINGS; i++)
    flatValue[i] = string[i][0]->getValue() + string[i][1]->getValue();

}

void loop() {

  for (int i = 0; i < NUMSTRINGS; i++) {
    for (int j = 0; j < 2; j++) {
      string[i][j]->update();
      stringValue[i][j] = string[i][j]->getValue();
    }
  }

  for (int i = 0; i < NUMSTRINGS; i++)
  {
    if (stringValue[i][0] + stringValue[i][1] > flatValue[i] - stringThreshold)
    {      
      if (countString[i] < 50)
      {
        valuesString[i][countString[i]] = stringValue[i][0] + stringValue[i][1];

        if(countString[i] > 2 && stringIsFlat[i]) 
        {
          if (valuesString[i][countString[i]] > flatValue[i] + 1)
          {
            if (valuesString[i][countString[i]] >= valuesString[i][countString[i-1]] + 0.2 && valuesString[i][countString[i-1]] >= valuesString[i][countString[i-2]] + 0.8)
            {
                countString[i] = 50;
            }
          }
        }
        float checkString = 0;
        for (int j = 0; j < 50; j++)
          checkString += valuesString[i][j];

        if (checkString / 50 > checkString / 50 - 1 && checkString / 50 < checkString / 50 + 1) {
          stringIsFlat[i] = true;
          flatValue[i] = checkString / 50;
        }

        countString[i]++;
      }
      else {

        float checkString = 0;
        for (int k = 0; k < 50; k++) {
          if (valuesString[i][k] > flatValue[i] - 5)
            checkString++;
        }

        if (checkString > 90) {
          stringIsFlat[i] = false;
        }
        else {
          stringIsFlat[i] = true;
        }

        countString[i] = 0;
      }
    }
    else {

      if (stringIsFlat[i]) {
        Serial.print("ENVELOPE ON FOR STRING ");
        Serial.println(i+1);

        violina[i]->setParamValue("gain",0.5);
      }


      stringIsFlat[i] = false;
    }

    if (!stringIsFlat[i]) {

      //violin1.setParamValue("velocity", (reverseNumber(stringValue[i][0] + stringValue[i][1], 0, 1023) - flatValue[i])/1000);

      //Serial.println(float(reverseNumber(stringValue[i][0] + stringValue[i][1], 0, 2000))/1000);
      violina[i]->setParamValue("velocity", (float(reverseNumber(stringValue[i][0] + stringValue[i][1], 0, 1800)))/1000);
      violina[i]->setParamValue("gain", (float(reverseNumber(stringValue[i][0] + stringValue[i][1], 200, 2000)))/1000);

    } else {

      float checkString = 0;
      for (int k = 0; k < 50; k++)
        checkString += valuesString[i][k];

      if (checkString / 50 > checkString / 50 - 1 && checkString / 50 < checkString / 50 + 1) {
         Serial.print("STOP ");
         Serial.println(i+1);
        
        violina[i]->setParamValue("gain",0);
        stringIsFlat[i] = true;
        flatValue[i] = checkString / 50;
      }
    }
  }

  delay(10);
}


float reverseNumber(int num, int min, int max) {
  return (float(max) + float(min)) - float(num);
}
