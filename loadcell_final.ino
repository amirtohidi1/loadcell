/*
   circuits4you.com
   2016 November 25
   Load Cell HX711 Module Interface with Arduino to measure weight in Kgs
  Arduino
  pin
  2 -> HX711 CLK
  3 -> DOUT
  5V -> VCC
  GND -> GND

  Most any pin on the Arduino Uno will be compatible with DOUT/CLK.
  The HX711 board can be powered from 2.7V to 5V so the Arduino 5V power should be fine.
*/

#include "HX711.h"  //You must have this library in your arduino library folder
#include "SevSeg.h"
#include <EEPROM.h>

SevSeg sevseg; //Instantiate a seven segment controller object

#define DOUT  3
#define CLK  2

HX711 scale(DOUT, CLK);

float datanumber;

//Change this calibration factor as per your load cell once it is found you many need to vary it in thousands
float calibration_factor = -96650; //-106600 worked for my 40Kg max scale setup
float zero = 0;
int zeroLoop = 20;

int cyclesabet = 0;
int cyclesabet_check = 20;

struct Parametr {
  float m50;
  float v50;
  float m100;
  float v100;
  float m150;
  float v150;
  float m200;
  float v200;
  float m250;
  float v250;
};

Parametr parametr1 = {
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0
};


//=============================================================================================
//                         SETUP
//=============================================================================================
void setup() {
  Serial.begin(9600);

  pinMode(14 , INPUT_PULLUP);
  scale.set_scale();
  scale.tare(); //Reset the scale to 0

  char str[] = {'C', 'A', 'L'};

  Serial.print("tarazo empty: ");

  Serial.print("Zero factor: "); //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
  Serial.println( zero);

  //seven segment setup
  byte numDigits = 3;
  byte digitPins[] = {13, 12, 11}; //Digits: 1,2,3,4 <--put one resistor (ex: 220 Ohms, or 330 Ohms, etc, on each digit pin)
  byte segmentPins[] = {4, 5, 6, 7, 8, 9, 10, 15}; //Segments: A,B,C,D,E,F,G,Period
  bool resistorsOnSegments = true; // 'false' means resistors are on digit pins
  byte hardwareConfig = 2; // See README.md for options
  bool updateWithDelays = true; // Default. Recommended
  bool leadingZeros = false; // Use 'true' if you'd like to keep the leading zeros

  sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments, updateWithDelays, leadingZeros);
  sevseg.setBrightness(50);

  sevseg.setChars(str);
  zero = getDataWithShow(zeroLoop, true, false);
  Serial.println("Zero SET");
  EEPROM.get( 0, parametr1 );
}

//=============================================================================================
//                         LOOP
//=============================================================================================
int datanew = 0;
void loop() {

  checkDataForShow();
  checkZeroAndCal();

  float data_fetch = 0 ;
  int loopvazn = 30;

  data_fetch = getDataWithShow(loopvazn, false , true);
  data_fetch = data_fetch - zero;
  Serial.println(data_fetch);
  datanew  = (calibre_range(data_fetch));
  Serial.println(datanew);
}

/**
   Calibre Range
*/

float calibre_range(float val)
{
  float retData = 0;
  int type = 0;
  
  if (val <= parametr1.m50)
  {
    retData =  val / parametr1.v50 ;
    type = parametr1.v50;
  }
  else  if (val > parametr1.m50 && val <= parametr1.m100)
  {
    retData =  val / parametr1.v100 ;
    type = parametr1.v100;
  }
  else  if (val > parametr1.m100 && val <= parametr1.m150)
  {
    retData =  val / parametr1.v150 ;
    type = parametr1.v150;
  }
  else  if (val > parametr1.m150 && val <= parametr1.m200)
  {
    retData =  val / parametr1.v200 ;
    type = parametr1.v200;
  }
  else
  {
    retData =  val / parametr1.v250 ;
    type = parametr1.v250;
  }

  //  Serial.println(val);
  //  Serial.println(type);
  //  Serial.println(retData);
  return retData;
}


/**
   ShowSegment
*/
void showSegment()
{
  showSegment(1);
}

void showSegment(char c1 , char c2 , char c3 , int loopshow)
{
  char str[] = {'0', '0', '0'};
  str[0] = c1;
  str[1] = c2;
  str[2] = c3;
  sevseg.setChars(str);
  for (int i = 0 ; i < loopshow ; i++)
  {
    sevseg.refreshDisplay(); // Must run repeatedly
  }
}

void showSegment(int loopshow)
{
  static unsigned long timer = millis();

  sevseg.setNumber((int)datanumber, 1);

  for (int i = 0 ; i < loopshow ; i++)
  {
    sevseg.refreshDisplay(); // Must run repeatedly
  }
}


/**
   checkZeroCalibration
*/
void checkZeroAndCal()
{
  if (digitalRead(14) == 0)
  {
    int zerocount = 0;
    while (true)
    {
      if (digitalRead(14) == 0)
      {
        zerocount++;
        delay(500);
        if (zerocount > 6)
        {
          break;
        }
        continue;
      }
      else
      {
        break;
      }
    }

    Serial.println("Serial.println(parametr1.v200);");
    Serial.println(zerocount);

    if (zerocount < 6 && zerocount > 1)
    {
      zero = scale.read_average(zeroLoop);
    }

    if (zerocount > 6)
    {
      Calibration();
    }
  }
}
/*

   Calibration SetData
*/

void Calibration()
{
  showSegment('C' , 'A' , 'L' , 500);


  int loopvazn = 50;
  float data_fetch = 0;

  //SET ZERO
  showSegment('s' , 'e' , 't' , 500);
  showSegment('0' , '0' , '0' , 1000);

  for (int y = 0 ; y < loopvazn ; y++)
  {
    data_fetch = data_fetch + scale.read_average(1) ;
    showSegment();
  }

  zero = (data_fetch / loopvazn);


  //SET 50
  data_fetch = 0;
  showSegment('s' , 'e' , 't' , 500);
  showSegment(' ' , '5' , '0' , 1500);

  for (int y = 0 ; y < loopvazn ; y++)
  {
    data_fetch = data_fetch + scale.read_average(1) - zero ;
    showSegment();
  }
  parametr1.m50 = (data_fetch / loopvazn);
  parametr1.v50  = parametr1.m50 / 50;

  //SET 100
  data_fetch = 0;
  showSegment('s' , 'e' , 't' , 500);
  showSegment('1' , '0' , '0' , 1500);

  for (int y = 0 ; y < loopvazn ; y++)
  {
    data_fetch = data_fetch + scale.read_average(1) - zero ;
    showSegment();
  }
  parametr1.m100 = (data_fetch / loopvazn);
  parametr1.v100  = parametr1.m100 / 100;

  //SET 150
  data_fetch = 0;
  showSegment('s' , 'e' , 't' , 500);
  showSegment('1' , '5' , '0' , 1500);

  for (int y = 0 ; y < loopvazn ; y++)
  {
    data_fetch = data_fetch + scale.read_average(1) - zero ;
    showSegment();
  }
  parametr1.m150 = (data_fetch / loopvazn);
  parametr1.v150  = parametr1.m150 / 150;


  //SET 200
  data_fetch = 0;
  showSegment('s' , 'e' , 't' , 500);
  showSegment('2' , '0' , '0' , 1500);

  for (int y = 0 ; y < loopvazn ; y++)
  {
    data_fetch = data_fetch + scale.read_average(1) - zero ;
    showSegment();
  }
  parametr1.m200 = (data_fetch / loopvazn);
  parametr1.v200  = parametr1.m200 / 200;

  //SET 250
  data_fetch = 0;
  showSegment('s' , 'e' , 't' , 500);
  showSegment('2' , '5' , '0' , 1500);

  for (int y = 0 ; y < loopvazn ; y++)
  {
    data_fetch = data_fetch + scale.read_average(1) - zero ;
    showSegment();
  }
  parametr1.m250 = (data_fetch / loopvazn);
  parametr1.v250  = parametr1.m250 / 250;
  
  EEPROM.put( 0, parametr1 );

  showSegment('C' , 'A' , 'L' , 500);
  showSegment('E' , 'n' , 'd' , 500);

}

/**
   check Data for Status

*/
void checkDataForShow()
{
  datanew = round(datanew);

  if ((datanew < 5 && datanew > -5) &&   cyclesabet < cyclesabet_check)
  {
    datanumber = 0;
    cyclesabet++;
    //    Serial.print("zero adad");
    //    Serial.println();
  }
  else if ( (  (datanew == datanumber) || (datanew ) == (datanumber - 1) || (datanew ) == (datanumber + 1)  ) && cyclesabet < cyclesabet_check)
  {
    //datanumber = round(((datanew + datanumber) / 2));
    cyclesabet++;
    //    Serial.print("kam");
    //    Serial.println();
  }
  else if ( (  (datanew) == (datanumber - 2) || (datanew ) == (datanumber + 2)   ) && cyclesabet < cyclesabet_check)
  {
    datanumber = round(((datanew + datanumber) / 2));
    cyclesabet++;
  }
  else
  {
    //Serial.println();
    datanumber = round(datanew);
    cyclesabet = 0;
  }

}

/**
   get Data
*/
float getDataWithShow(int loopvazn , bool refreshSevSeg , bool refreshShowSegment)
{
  float fetchData = 0 ;
  for (int i = 0 ; i < loopvazn ; i++)
  {
    fetchData = fetchData + scale.read_average(1);
    if (refreshSevSeg)
    {


      sevseg.refreshDisplay();
    } else if (refreshShowSegment)
    {
      showSegment();
    }
    else
    {}
  }

  fetchData = fetchData / loopvazn;

  return fetchData;
}
