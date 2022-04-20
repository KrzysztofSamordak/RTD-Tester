#include <pt100rtd.h>
#include <Adafruit_MAX31865.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

//enter
#define ENTER_BUTTON_PIN 14 
#define ENTER_BUTTON_CLICKED LOW
#define ENTER_BUTTON_RELEASED HIGH

// RTDSensor
#define RTD_PIN A0
#define RTD_CORRECTION -1.5 //correction acquired at 23*C
#define RTD_MIN_TEMP_LIMIT 19 // minimal temperature limit for OverTempSensor in *C
#define RTD_MAX_TEMP_LIMIT 32 // maximal temperature limit for OverTempSensor in *C
const float K = 0.0125327;
pt100rtd PT100 = pt100rtd();

// display
#define DISPLAY_ROWS 2
#define DISPLAY_COLUMNS 16
LiquidCrystal_I2C lcd(0x27, DISPLAY_COLUMNS, DISPLAY_ROWS);

String displayBuffer[2];

void waitForEnter();
bool readTempFromRTD();
unsigned long readTimeInSeconds();
bool Print(int column, int row, String displayBuffer[2], String message);
void updateDisplay(int numberOfRows, String displayBuffer);


void setup() 
{
  //Serial.begin(115200);
  lcd.begin(DISPLAY_COLUMNS, DISPLAY_ROWS);
  lcd.init();
  lcd.backlight();
  pinMode(ENTER_BUTTON_PIN, INPUT);
}

void loop()
{
  Print(0, 0, displayBuffer, "Nacisnij <ENTER>");
  Print(0, 1, displayBuffer, "aby rozpoczac");
  updateDisplay(2, displayBuffer);
  waitForEnter();
  Print(0, 0, displayBuffer, "Rozpoczynam");
  Print(0, 1, displayBuffer, "pomiar...");
  updateDisplay(2, displayBuffer);
  readTempFromRTD();
  waitForEnter();
}

unsigned long readTimeInSeconds()
{
  return (millis() / 1000);
}

void waitForEnter()
{
  while(digitalRead(ENTER_BUTTON_PIN) == HIGH)
  { 
    delay(10);
  }
  delay(80);
  while(digitalRead(ENTER_BUTTON_PIN) == LOW)
  {
    delay(10);
  }
  delay(80);
}

bool Print(int column, int row, String displayBuffer[2], String message)
{
  bool returnVal = false;
  String tmp;
  int max_position = (message.length() - 1)  + column;
  if(column >= 0 & column < DISPLAY_COLUMNS)
  {
    if(row >= 0 & row <= DISPLAY_ROWS)
    {
      if(max_position < DISPLAY_COLUMNS)
      {
        if(column != 0)
        {
          for(int i = 0; i < column; i++)
          {
            tmp += " ";
          }
          for(int i = column; i <= max_position; i++)
         {
          tmp += message[i - column];
         }
         if(max_position < DISPLAY_COLUMNS - 1)
         {
          for(int j = max_position; j < DISPLAY_COLUMNS; j++)
          {
            tmp += " ";
          }
         }
         displayBuffer[row] = tmp;
         returnVal = true;
        }else
        {
          displayBuffer[row] = message;
          if(max_position < DISPLAY_COLUMNS - 1)
         {
          for(int j = max_position; j < DISPLAY_COLUMNS; j++)
          {
            displayBuffer[row] += " ";
          }
          returnVal = true;
        }
       }
      }
    }
  }
  return returnVal; 
}

void updateDisplay(int numberOfRows, String displayBuffer[2])
{
  if(numberOfRows == 0)
  {
    lcd.setCursor(0,0);
    lcd.print(displayBuffer[0]);
  }else if (numberOfRows == 1)
  {
    lcd.setCursor(0,1);
    lcd.print(displayBuffer[1]);
  }else if(numberOfRows == 2)
  {
    lcd.setCursor(0,0);
    lcd.print(displayBuffer[0]);
    lcd.setCursor(0,1);
    lcd.print(displayBuffer[1]);
  }
  lcd.display();  
}

bool readTempFromRTD()
{
  String message;
  char temperatureTmp[5];
  int counter = 0;
  float Rrtd;
  float readAnalogVal = 0;
  float temp = 0;
  unsigned long refTime = 0;
  bool result = false;
  refTime = readTimeInSeconds();
  // Time for sensor stabilization
  for (int i = 1; i >= (readTimeInSeconds() - refTime); )
  {  
    readAnalogVal = readAnalogVal + map( (analogRead(RTD_PIN)), 0, 1023, 0, 5000);
    delay(10);
  }
  
  //reset values
  readAnalogVal = 0;
  refTime = readTimeInSeconds();
  // Measuring
  for(int i = 1; i >= (readTimeInSeconds() - refTime); )
  {
    readAnalogVal = readAnalogVal + map( (analogRead(RTD_PIN)), 0, 1023, 0, 5000);
    counter++;
    delay(10);
  }
  readAnalogVal = readAnalogVal / counter;
  Rrtd = 100 + readAnalogVal * K;
  temp = PT100.celsius(Rrtd);
  temp = temp + RTD_CORRECTION;  
  if ( temp > RTD_MIN_TEMP_LIMIT && temp < RTD_MAX_TEMP_LIMIT)
  {
    dtostrf(temp, 5, 2, temperatureTmp);
    message = "temp: ";
    message += temperatureTmp;
    message += "*C";
    Print(0, 0, displayBuffer, "RTD - PASS");
    Print(0, 1, displayBuffer, message);
    updateDisplay(2, displayBuffer);
    result = true;
    
  }else
  {
    dtostrf(temp, 5, 2, temperatureTmp);
    message = "temp: ";
    message += temperatureTmp;
    message += "*C";
    Print(0, 0, displayBuffer, "RTD - FAIL");
    Print(0, 1, displayBuffer, message);
    updateDisplay(2, displayBuffer);
    result = false;
    
  }
  return result;
}
