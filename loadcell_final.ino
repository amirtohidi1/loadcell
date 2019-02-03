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
SevSeg sevseg; //Instantiate a seven segment controller object

#define DOUT  3
#define CLK  2

HX711 scale(DOUT, CLK);

float datanumber;

//Change this calibration factor as per your load cell once it is found you many need to vary it in thousands
float calibration_factor = -96650; //-106600 worked for my 40Kg max scale setup
float zero = 0;
int zeroLoop = 20;
float v1 = 931;

int cyclesabet = 0;
int cyclesabet_check = 100;
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

  for (int i = 0 ; i < 1000 ; i++)
  {

    sevseg.refreshDisplay();
  }
  zero = scale.read_average(zeroLoop);

}

//=============================================================================================
//                         LOOP
//=============================================================================================
int datanew = 0;
void loop() {
  //
  //Serial.print("new=");
  //Serial.print(datanew);
  //    Serial.println();
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

  checkZeroAndCal();

  float data_fetch = 0 ;
  int loopvazn = 30;
  for (int y = 0 ; y < loopvazn ; y++)
  {
    data_fetch = data_fetch + scale.read_average(1);
    showSegment();
  }

  data_fetch = (data_fetch / loopvazn);
  data_fetch = data_fetch - zero;

  datanew  = (calibre_range(data_fetch));
}


//=============================================================================================


float calibre_range(float val)
{
  return val / v1;
}

/**
   ShowSegment
*/
void showSegment()
{
  showSegment(1);
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
  char str[] = {'C', 'A', 'L'};
  sevseg.setChars(str);
  for (int i = 0 ; i < 500 ; i++)
  {
    sevseg.refreshDisplay(); // Must run repeatedly
  }

  str[0] = 's';
  str[1] = 'e';
  str[2] = 't';
  sevseg.setChars(str);
  for (int i = 0 ; i < 500 ; i++)
  {
    sevseg.refreshDisplay(); // Must run repeatedly
  }

  str[0] = '0';
  str[1] = '0';
  str[2] = '0';
  sevseg.setChars(str);
  for (int i = 0 ; i < 1000 ; i++)
  {
    sevseg.refreshDisplay(); // Must run repeatedly
  }


  int loopvazn = 50;
  float data_fetch = 0;

  for (int y = 0 ; y < loopvazn ; y++)
  {
    data_fetch = data_fetch + scale.read_average(1) ;
    showSegment();
  }

  Serial.println("data_fetch=");
  Serial.println(data_fetch);

  zero = (data_fetch / loopvazn);
  data_fetch = 0;


  str[0] = 's';
  str[1] = 'e';
  str[2] = 't';
  sevseg.setChars(str);
  for (int i = 0 ; i < 500 ; i++)
  {
    sevseg.refreshDisplay(); // Must run repeatedly
  }

  str[0] = '2';
  str[1] = '0';
  str[2] = '0';
  sevseg.setChars(str);
  for (int i = 0 ; i < 1000 ; i++)
  {
    sevseg.refreshDisplay(); // Must run repeatedly
  }

  data_fetch = 0 ;
  for (int y = 0 ; y < loopvazn ; y++)
  {
    data_fetch = data_fetch + scale.read_average(1) - zero ;
    showSegment();
  }

  Serial.println("data_fetch=");
  Serial.println(data_fetch);

  float t1  = (data_fetch / loopvazn);


  parametr1.v200 = t1 / 200;

  str[0] = 'C';
  str[1] = 'A';
  str[2] = 'L';
  sevseg.setChars(str);
  for (int i = 0 ; i < 500 ; i++)
  {
    sevseg.refreshDisplay(); // Must run repeatedly
  }

  str[0] = 'E';
  str[1] = 'N';
  str[2] = 'D';
  sevseg.setChars(str);
  for (int i = 0 ; i < 500 ; i++)
  {
    sevseg.refreshDisplay(); // Must run repeatedly
  }


  EEPROM.put( 0, parametr1 );
}
