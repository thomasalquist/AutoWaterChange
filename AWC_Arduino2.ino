//AutoWaterChange Arduino 2 - Downstairs Arduino
#include <EEPROM.h>
#include <GravityTDS.h>
#include <avr/wdt.h>


GravityTDS gravityTds;

#define TdsSensorPin A1
#define PressureSensor_Pin A0

const int RODI_TDS_Limit = 10; //limit for RODI TDS OK
const int Pressure_Limit_High = 60;//pressure to trigger RODI system off
const int Pressure_Limit_Low = 10; //pressure to trigger RODI system on


const int OKVlv_Pin = 7;
const int NOKVlv_Pin = 6;
const int FillVlv_Pin = 5;
const int CircVlv_Pin = 4;
const int FillCMD_Pin = 8;
const int PumpRly_Pin = 9;
const int RODIPump_CMD_Pin = 10;
const int Button_Pin = 13;
const int Button_LED_Pin = 11;

int Button_State = 0;
int Last_Button_State = 0;

int FillCMD_State = 0;
int CircVlv_State = 0;
int PumpRly_State = 0;
int Man_Circ_State = 0;
int PressureSwitch_State = 0;

float PressureReading = 0;
float PressureVoltage = 0;
float Pressure = 0;
float tdsValue = 0;
float temperature = 25; //static temp coefficient

const unsigned long interval[] = {3000000,600000}; //(Off, On)

unsigned long previousMillis = 0; //define previousMillis as an unsigned long variable

typedef enum{
  RESERVOIR_IDLE,
  CIRCULATION,
  FILL,
}State_Reservoir;
State_Reservoir State_R;

typedef enum{
  RODI_IDLE,
  RODI_OK,
  RODI_NOK,
}State_RODISys;
State_RODISys State_RODI;

void setup() 
{
    wdt_enable(WDTO_8S); //enables watchdogtimer with 8s countdown - if timer doesn't hit 8s (program is frozen) Arduino will reset

    Serial.begin(115200);

      //canned code from GravityTDS example
    gravityTds.setPin(TdsSensorPin);
    gravityTds.setAref(5.0);  //reference voltage on ADC, default 5.0V on Arduino UNO
    gravityTds.setAdcRange(1024);  //1024 for 10bit ADC;4096 for 12bit ADC
    gravityTds.begin();  //initialization

      //pinmodes for each IO
    pinMode(OKVlv_Pin, OUTPUT);
    pinMode(NOKVlv_Pin, OUTPUT);
    pinMode(FillVlv_Pin, OUTPUT);
    pinMode(CircVlv_Pin, OUTPUT);
    pinMode(PressureSensor_Pin, INPUT);
    pinMode(FillCMD_Pin, INPUT);
    pinMode(PumpRly_Pin, OUTPUT);
    pinMode(Button_Pin, INPUT_PULLUP);
    pinMode(Button_LED_Pin, OUTPUT);
    pinMode(RODIPump_CMD_Pin, OUTPUT);
}

void loop()
{  
  wdt_reset(); //resets watchdog timer before it resets arduino

  delay(1000); //slow the chaos 
  RODI(); //RODI loop
  TDS_Measurement(); //TDS loop
  Reservoir(); //Reservoir loop
  Button(); //Button loop
  Serial.println(" END");
}

void TDS_Measurement()
{
  //canned TDS sensor code
  gravityTds.update();  //sample and calculate
  tdsValue = gravityTds.getTdsValue();  // then get the value
  Serial.print("TDS ");
  Serial.print(tdsValue,0);
  Serial.print("PPM||");
}

void RODI()
{
  //Read Pressure Sensor
  PressureReading = analogRead(PressureSensor_Pin);
/*
  //debug display Pressure Reading
  Serial.print("Pressure_Reading =");
  Serial.print(PressureReading);
  Serial.print("||");
*/
  //convert Pressure Reading to Pressure Voltage
  PressureVoltage = 0.00488 * PressureReading;
/*
  //debug display Pressure Voltate
  Serial.print("PressureVoltage=");
  Serial.print(PressureVoltage, 3);
  Serial.print("V||");
*/
  //Convert Pressure Sensor Voltage to Pressure
  Pressure = (PressureVoltage * 25) - 12.5;
  Serial.print("Pressure ");
  Serial.print(Pressure,3);
  Serial.print("PSI||");
  
  

  if (Pressure >= Pressure_Limit_High)
  {
    //if Pressure is greater than Pressure Limit system should idle
    PressureSwitch_State = LOW;
//    Serial.print("Pressure_High_IDLE||"); //debug Pressure Switch State
  }
  if (Pressure < Pressure_Limit_Low)
  {
    //if pressure is less than Pressure Limit system should function
    PressureSwitch_State = HIGH;
//    Serial.print("Pressure_Low_ON||"); //debug Pressure Switch State
  }

  if (PressureSwitch_State == LOW) //RODI system is IDLE
  {
      State_RODI = RODI_IDLE; //rewrite State_RODI
      Serial.print("RODI_Idle||");
  }
  if ((PressureSwitch_State == HIGH) && (tdsValue >= RODI_TDS_Limit))  //RODI System is producing NOK water
  {
      State_RODI = RODI_NOK;  //rewrite State_RODI
      Serial.print("RODI_NOK||");
  }
  if ((PressureSwitch_State == HIGH) && (tdsValue <= RODI_TDS_Limit))  //RODI System is producing OK water
  {
      State_RODI = RODI_OK; //rewrite State_RODI
      Serial.print("RODI_OK||");
  }
  else
  {
    //do nothing
  }

  switch(State_RODI)
  {
    case RODI_IDLE: //what to do when RODI is Idle
    {
      digitalWrite (OKVlv_Pin, LOW);
      digitalWrite (NOKVlv_Pin, LOW);
      digitalWrite (RODIPump_CMD_Pin, LOW);
      break;
    }
    case RODI_OK: //what to do when RODI is OK
    {
      digitalWrite (OKVlv_Pin, HIGH);
      digitalWrite (NOKVlv_Pin, LOW);
      digitalWrite (RODIPump_CMD_Pin, HIGH);
      break;
    }
    case RODI_NOK:  //what to do when RODI is NOK
    {
      digitalWrite (OKVlv_Pin, LOW);
      digitalWrite (NOKVlv_Pin, HIGH);
      digitalWrite (RODIPump_CMD_Pin, HIGH);
      break;
    }
  }
}

void Reservoir()
{    //automated reservoir actions, fill from AWC, circulate water 5 minutes every hour
    
  FillCMD_State = digitalRead(FillCMD_Pin);
  unsigned long currentMillis = millis();
  static bool lastState_bo = LOW;

  if(FillCMD_State == HIGH)
  {
    digitalWrite (CircVlv_Pin, LOW);
    digitalWrite (FillVlv_Pin, HIGH);
    digitalWrite (PumpRly_Pin, HIGH);
    digitalWrite (Button_LED_Pin, HIGH);
    Serial.print ("FillCMD HIGH||");
  }
  else // if(FILLCMD_State == LOW)
  { 
    if( (currentMillis - previousMillis >= interval[CircVlv_State] ) ||
        ( HIGH == lastState_bo ) )
    {
      previousMillis = currentMillis; 
        if ((CircVlv_State == LOW) && (PumpRly_State == LOW))
        {
          CircVlv_State = HIGH;
          PumpRly_State = HIGH;
        }
        else
        {
          CircVlv_State = LOW;
          PumpRly_State = LOW;
        }
        digitalWrite (CircVlv_Pin, CircVlv_State);
        digitalWrite (FillVlv_Pin, LOW);
        digitalWrite (PumpRly_Pin, PumpRly_State);      
    }
    if(CircVlv_State == HIGH)
    {
      Serial.print("Circulation_Mode||");
      digitalWrite (Button_LED_Pin, HIGH);
    }
    if(CircVlv_State == LOW)
    {
      Serial.print("Reservoir_Idle||");
      digitalWrite (Button_LED_Pin, LOW);
    }  
  }
  
  lastState_bo = FillCMD_State;
}
void Button()
{
  Last_Button_State = Button_State;
  Button_State = digitalRead(Button_Pin);

  CircVlv_State = digitalRead(CircVlv_Pin);
  FillCMD_State = digitalRead(FillCMD_Pin);

  if(FillCMD_State == LOW)
  {
    if(Button_State == LOW && Last_Button_State == HIGH)
    {      
      //Serial.print("BUTT IS PRESSED ");
          if((CircVlv_State == LOW) && (PumpRly_State == LOW))
          {
            CircVlv_State = HIGH;
            PumpRly_State = HIGH;
            Man_Circ_State = HIGH;
          }
          else
          {
            CircVlv_State = LOW;
            PumpRly_State = LOW;
            Man_Circ_State = LOW;
          }
          digitalWrite(CircVlv_Pin, CircVlv_State);
          digitalWrite(PumpRly_Pin, PumpRly_State);
          Serial.print("Manual_Circulation||");
          Serial.print(Man_Circ_State);

    }
  }
}  
