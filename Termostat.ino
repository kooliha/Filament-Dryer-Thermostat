
// Setup display
#include <Arduino.h>
#include <FourBitLedDigitalTube.h>
const uint8_t dio {A3}; // Pin A3 of Arduino Nano
const uint8_t rclk {A1}; // Pin A4 of Arduino Nano
const uint8_t sclk {A2}; // Pin A5 of Arduino Nano
TM74HC595LedTube myLedDisp(sclk, rclk, dio);


//setup buttons
const int commonPin = 2;
const int buttonPins[] = {7,8,9};
unsigned  long lastFire = 0;


// Fan
int Fan = 6;

//Led to show status
int ModeLed = 3;



float RealTemperature = 0;
float CurrentHumidity = 0;
float TargetTemperature = 60.0;
float HeaterStartus = 0; //0-off, 1-on
float Auto = 1; //0-off, 1-on
int DisplayAction = 0; // show nothing 1-  show off, 2 -show 
int DisplayScreen = 0; 


// temperature
#include <Adafruit_AHTX0.h>
Adafruit_AHTX0 aht;

//What is long cklick duration?
unsigned int buttonPressed = 0;
unsigned long buttonPressStartTime = 0;
const unsigned long DoubleClickDuration = 500; // Adjust as needed



void setup()
{


pinMode(Fan, OUTPUT);
digitalWrite(Fan, LOW);

pinMode(ModeLed, OUTPUT);
digitalWrite(ModeLed,LOW);

//temp pin high

pinMode(11, OUTPUT);
digitalWrite(11, HIGH);

aht.begin();



Serial.begin(9600);

//Buttons setup
configureCommon(); // Setup pins for  interrupt
attachInterrupt(digitalPinToInterrupt(commonPin), pressInterrupt,  FALLING);


myLedDisp.begin();
//myLedDisp.stop();

}




void loop()
{
Display();
Heat();
ReadTemperature();
Termostat();
ModeLedCheck();

// If single click
if (buttonPressed == 1 && (millis()-buttonPressStartTime)>=DoubleClickDuration) {
  buttonPressStartTime = 0;
  buttonPressed = 0;
  if (DisplayScreen == 0){
    DisplayScreen = 1;
  }
  else {DisplayScreen = 0;}
  } 

}
  



void ModeLedCheck() {

if (Auto==1) 
{
digitalWrite(ModeLed, HIGH);
}
else
digitalWrite(ModeLed, LOW);

}



void Termostat() {

if (RealTemperature<TargetTemperature && Auto == 1) // Keep on Until Temperature is not up if Mode is on
{
  if (HeaterStartus == 0){
  HeaterStartus = 1;
  DisplayAction = 2;
  // else its on
  }
}

else if (RealTemperature>=TargetTemperature && Auto == 1) // Keep on Until Temperature is not up if Mode is on
{
  if (HeaterStartus == 1){
  HeaterStartus = 0;
  DisplayAction = 1;
  // else its on
}
}

}





void ReadTemperature() {
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);

  RealTemperature = temp.temperature;
  CurrentHumidity = humidity.relative_humidity;

}





void Heat() {
  if (HeaterStartus == 0) {
    digitalWrite(Fan, LOW);
  } else {
    digitalWrite(Fan, HIGH);
  }
}







void Display()
{

if (DisplayScreen == 0)
{
String TempDisplay = String(RealTemperature,1) + "t";
myLedDisp.print(TempDisplay);


//myLedDisp.print("t",1);
}
else if (DisplayScreen == 1)
{

String TempDisplay = String(CurrentHumidity,1) + "H";
myLedDisp.print(TempDisplay);
}


      if (DisplayAction == 1){// if need to turn off
       DisplayAction = 0;
        myLedDisp.print("OFF");
        delay (1000);
        myLedDisp.clear();
      }
      if (DisplayAction == 2) {//if need to turn On
        DisplayAction = 0;
        myLedDisp.print("On");
        delay (1000);
        myLedDisp.clear();
      }
      if (DisplayAction == 3) {//show target temperature
        DisplayAction = 0;
        myLedDisp.print(TargetTemperature);
        delay (700);
        myLedDisp.clear();
        Auto = 1;
      }





}






void  pressInterrupt() { // ISR
  if (millis() - lastFire < 200) { // Debounce
    return;
  }
  lastFire = millis();

  configureDistinct(); // Setup  pins for testing individual buttons

  for (int i = 0; i < sizeof(buttonPins)  / sizeof(int); i++) { // Test each button for press
    if (!digitalRead(buttonPins[i]))  {
      press(i);
    }
  }

  configureCommon(); // Return to original  state
}

void configureCommon() {
  pinMode(commonPin, INPUT_PULLUP);

  for (int i = 0; i < sizeof(buttonPins) / sizeof(int); i++) {
    pinMode(buttonPins[i],  OUTPUT);
    digitalWrite(buttonPins[i], LOW);
  }
}

void configureDistinct()  {
  pinMode(commonPin, OUTPUT);
  digitalWrite(commonPin, LOW);

  for  (int i = 0; i < sizeof(buttonPins) / sizeof(int); i++) {
    pinMode(buttonPins[i],  INPUT_PULLUP);
  }
}


void press(int button) { // Our handler

  switch (button) {
    case 0:
      TargetTemperature += 1;
       DisplayAction = 3;
      break;

    case 1:
      TargetTemperature -= 1;
      DisplayAction = 3;
      break;

    case 2:

if (buttonPressed == 0) {
  buttonPressStartTime = millis();
  buttonPressed = 1;
} 



// Dobule cklick processing - switching to manual modede
else if (buttonPressed == 1 && (millis() - buttonPressStartTime) <= DoubleClickDuration) {
  buttonPressStartTime = 0;
  buttonPressed = 0;
  Auto = 0;
  
  
  if (HeaterStartus == 0){
  
  HeaterStartus = 1;
  DisplayAction = 2;
  }

  else {
  HeaterStartus = 0;
  DisplayAction = 1;
  }

} 

      
   break;
    default:
      break;
  }
  }






