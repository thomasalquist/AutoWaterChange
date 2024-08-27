//AutoWaterChange Arduino #1
#include <avr/wdt.h>

const int Drain_Volume = 13; //define AWC Volume

#define WaterLevel_Pin A0
//define IO
const int DrainVlv_Pin = 7;
const int DrainPump_Pin = 6;
const int Button_Pin = 13;
const int Button_LED_Pin = 11;
const int FillCMD_Pin = 3;

//define variables
boolean Button_State = 0;       //current state of button
boolean Last_Button_State = 0;  //previous state of button

//define machine states
int FillCMD_State = 0;
int DrainVlv_State = 0;
int DrainPump_State = 0;

//define waterlever vars
float WaterLevel_Reading = 0;
float WaterLevel = 0;
float Filled_WaterLevel = 0;
float Drain_WaterLevel = 0;
//set up smoothing function for waterlevel measurements
const int numReadings = 10;
int readings[numReadings];
int readIndex = 0;
int total = 0;
float WL_Reading_Average = 0;

typedef enum AWC_process
{
  Initial,
  Idle,
  Draining,
  Filling,
} ;
AWC_process AWC_Process;

typedef enum AWC_state
{
  Cycle_Idle,
  Cycle_Running,
} ;
AWC_state AWC_State;

void setup() 
{
  wdt_enable(WDTO_8S); //enables watchdog timer with 8s countdown

  Serial.begin(115200);

  pinMode(DrainVlv_Pin, OUTPUT);
  pinMode(DrainPump_Pin, OUTPUT);
  pinMode(Button_Pin, INPUT_PULLUP);
  pinMode(Button_LED_Pin, OUTPUT);
  pinMode(WaterLevel_Pin, INPUT);
  pinMode(FillCMD_Pin, OUTPUT);
}

void loop() 
{
  wdt_reset(); //resets watchdog timer before it resets arduino
  
  delay(1000);
  AWC();
  Button();
  Serial.println("END||");
}

void Button()
{
  Last_Button_State = Button_State;
  Button_State = digitalRead(Button_Pin);

  if(Button_State == LOW && Last_Button_State == HIGH)
  {
    if (AWC_State != Cycle_Running)
    {
      AWC_State = Cycle_Running;
      AWC_Process = Initial;
    }
    else
    {
    }
  }
  else
  {
  }

}

void AWC()
{
  //smooths the analog reading from the level sensor
  total = total - readings[readIndex];
  readings[readIndex] = analogRead(WaterLevel_Pin);
  total = total + readings[readIndex];
  readIndex = readIndex + 1;
  if(readIndex >= numReadings)
  {
    readIndex = 0;
  }

  WaterLevel_Reading = analogRead(WaterLevel_Pin);
  WL_Reading_Average = total / numReadings;
  //calculates current WaterLevel
  WaterLevel = (WL_Reading_Average - 497) / 18.945;

  Serial.print ("Current_WaterLevel=");
  Serial.print (WaterLevel);
  Serial.print ("in||");


/*
  //debug WaterLevel Math
  Serial.print("Reading=");
  Serial.print(WaterLevel_Reading);
  Serial.print("||");
  Serial.print("Smoothed_Reading =");
  Serial.print(WL_Reading_Average);
  Serial.print("||");
*/

  if(AWC_State == Cycle_Running)
  {
    Serial.print ("Filled_WaterLevel =");
    Serial.print (Filled_WaterLevel); 
    Serial.print ("in||"); 

    Serial.print ("Drain Target");
    Serial.print (Drain_WaterLevel);
    Serial.print ("in||"),   
    
    Serial.print("Cycle_Running||");

    digitalWrite(Button_LED_Pin, HIGH);

    if(AWC_Process == Initial) //simply starts the process after the button is pushed
    {
      Filled_WaterLevel = WaterLevel; //defines filled/final waterlevel
      Drain_WaterLevel = Filled_WaterLevel - (Drain_Volume / 2.87703); // constant based on sump dimensions converted from gals to in^3

      Serial.print ("Initial||");
      AWC_Process = Draining; 
    }
    if(WaterLevel > Drain_WaterLevel && AWC_Process == Draining) 
    {
      //turns on drain hardware, confirms fill hardware is off
      digitalWrite (DrainVlv_Pin, HIGH);
      digitalWrite (DrainPump_Pin, HIGH);
      digitalWrite (FillCMD_Pin, LOW);
      Serial.print ("Draining||");
    }

    if(WaterLevel <= Drain_WaterLevel) //change state from draining to filling
    {
      AWC_Process = Filling;
    }
    if(AWC_Process == Filling) 
    {
      //turns off drain hardware, turns on fill hardware
      digitalWrite (DrainVlv_Pin, LOW);
      digitalWrite (DrainPump_Pin, LOW);
      digitalWrite (FillCMD_Pin, HIGH);
      Serial.print ("Filling||");
    }
    if(WaterLevel >= Filled_WaterLevel + 0.2 && AWC_Process == Filling) //0.2 value acts as a filter for reading variability
    {
      //resets everything to idle state
      AWC_Process = Idle;
      AWC_State = Cycle_Idle; 
      digitalWrite (Button_LED_Pin, LOW);
      //turns all hardware off
      digitalWrite (DrainVlv_Pin, LOW);
      digitalWrite (DrainPump_Pin, LOW);
      digitalWrite (FillCMD_Pin, LOW);
      Serial.print ("Idle||");
    }
    else
    {
    }
  }
  else
  {
    AWC_State = Cycle_Idle; 
    digitalWrite (Button_LED_Pin, LOW);
    Serial.print ("Idle||");
  }
}

