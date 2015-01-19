
/* 
New brain for a Sanyo breadmaker "The Bread Factory Plus" (Model SBM-20)
(many other models are similar)

circuit and documentation on www.elfnor.com/Arduino Bread Details.html
code https://github.com/elfnor/arduino-bread

This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License.

elfnor
*/

// consts and structure to define a set of times for loaf programs
// if a value is set to zero that part is skipped.
// the heat cycle time is only used to estimate completion time
// actually heat cycle continues until bake temperature is reached



struct PGRMS
  {
    int min_pulseMix;
    int min_contMix;
    int min_rise1;
    int min_punchDown;
    int min_rise2;
    int min_heat;
    int min_bake;
  };
  
  
const int N_PGRMS = 6;

const PGRMS pgrms[N_PGRMS] = 
    {
      {6, 29, 30, 1, 53, 8, 42},  // 0 : normal loaf
      {6, 29, 0, 0, 0, 0, 0},     // 1 : mix only
      {0,  0, 1000, 0, 0, 0, 0},  // 2 : very long rise
      {6, 29, 1000, 0, 0, 0, 0},  // 3 : mix plus long rise
      {0, 0, 0, 0, 0, 8, 42},     // 4 : bake only
      {3,  3, 10, 0,  0, 0,  0},  // 5 : test nonsense     
    };
    
    

#include <Bounce2.h>
#include <Arduino.h>
#include <TM1637Display.h>

#define UP_BUTTON_PIN 0
#define DOWN_BUTTON_PIN 1
#define SELECT_BUTTON_PIN 2
#define START_BUTTON_PIN 3
#define STOP_BUTTON_PIN 4

#define heater 6
#define motor 7
#define buzzer 8
#define DIO 9 //TM1637 
#define CLK 10  //TM1637 4 digit display

#define LED_PIN 13

const int thermistor = A0;      // Analog input pin that the thermistor is connected to    

//instantiate a TM1637 object
TM1637Display display(CLK, DIO);

// Instantiate a Bounce object
Bounce start_deb = Bounce(); 
Bounce stop_deb = Bounce();
Bounce up_deb = Bounce();
Bounce down_deb = Bounce();
Bounce select_deb = Bounce();


boolean stop_pushed = false;

long min_start;
long min_total;
long min_delay = 0;

int n = 0; //selected program


void setup() {
  pinMode(motor, OUTPUT);
  pinMode(heater, OUTPUT);
  pinMode(buzzer, OUTPUT);
  
  // Setup the button with an internal pull-up :
  pinMode(START_BUTTON_PIN,INPUT_PULLUP);
  pinMode(STOP_BUTTON_PIN, INPUT_PULLUP);
  pinMode(UP_BUTTON_PIN, INPUT_PULLUP);
  pinMode(DOWN_BUTTON_PIN, INPUT_PULLUP);
  pinMode(SELECT_BUTTON_PIN, INPUT_PULLUP);

  // After setting up the button, setup the Bounce instance :
  start_deb.attach(START_BUTTON_PIN);
  start_deb.interval(5); // interval in ms
  
  stop_deb.attach(STOP_BUTTON_PIN);
  stop_deb.interval(5); // interval in ms
  
  up_deb.attach(UP_BUTTON_PIN);
  up_deb.interval(5); // interval in ms
  
  down_deb.attach(DOWN_BUTTON_PIN);
  down_deb.interval(5); // interval in ms
  
  select_deb.attach(SELECT_BUTTON_PIN);
  select_deb.interval(5); // interval in ms

  //Setup the LED :
  pinMode(LED_PIN,OUTPUT);
  digitalWrite(motor, 0);
  digitalWrite(heater,0);
  
  //TM1637 4 digit display
  display.setBrightness(0xF);
  uint8_t data[] = { 0x0, 0x0, 0x0, 0x0 };
  display.setSegments(data);
  setDisplay(n);
  
    // 10 second delay to make it easier to upload new programs  
  delay(10000);   
  // initialize serial communications at 9600 bps:  
  Serial.begin(9600);
}

void loop() 
{  
  // Update the Bounce instances : 
  start_deb.update();
  select_deb.update();
  up_deb.update();
  down_deb.update();
  
  
  if ( select_deb.fell() )
    {
    tone(buzzer, 262, 500);
    n = n + 1;
    if ( n >= N_PGRMS) { n = 0; }
    setDisplay(n);
    min_total = pgrms[n].min_pulseMix + pgrms[n].min_contMix + pgrms[n].min_rise1 + pgrms[n].min_rise2 + + pgrms[n].min_heat + pgrms[n].min_bake;
    }  
    
  if ( up_deb.fell() )  
    {
      int min_display;
      min_display = min_total + min_delay;
      displayMins(min_display);
      min_delay = min_delay + 1;
      min_display = min_total + min_delay;
      displayMins(min_display);
    }
    
  if ( down_deb.fell() )  
    {
      int min_display;
      min_display = min_total + min_delay;
      displayMins(min_display);
      min_delay = min_delay - 1;
      if ( min_delay <= 0) { min_delay = 0; }
      min_display = min_total + min_delay;
      displayMins(min_display);
    }   
  
  if ( start_deb.fell() ) 
    {    
    tone(buzzer, 262, 500);
    make_loaf(n);
    }
}


void make_loaf(int n)
{
  stop_pushed = false;
  min_start = millis()/1000;
  min_start = min_start/60;
  
  min_total = min_total + min_delay;
  
  delayUpdate(min_delay); 
  if (stop_pushed){return;}   
  pulseMix(pgrms[n].min_pulseMix);
  if (stop_pushed){return;}
  contMix(pgrms[n].min_contMix);
  if (stop_pushed){return;}
  rise(pgrms[n].min_rise1);
  if (stop_pushed){return;}
  punchDown(1);
  if (stop_pushed){return;}
  rise(pgrms[n].min_rise2);
  if (stop_pushed){return;} 
  heat(pgrms[n].min_heat);
  if (stop_pushed){return;}
  bake(pgrms[n].min_bake); 
  if (stop_pushed){return;} 
  setDisplay(0);
}  



boolean checkStop()
{
  //check if stop button pressed
  // this is called every 1000 mills

  updateDisplayTime();
  
  stop_deb.update();
  if ( stop_deb.fell() ) 
  {    
    digitalWrite(LED_PIN, LOW);
    digitalWrite(motor, LOW);
    digitalWrite(heater, LOW);
    stop_pushed = true;
    tone(buzzer, 196, 500);
    setDisplay(0); 
    return true;
  }  
  return false;
}  

void setDisplay(int n)
{
 //write n to display 
 display.showNumberDec(n, false);
}

void updateDisplayTime()
{ 
  int min_display;
  // set display to show time until loaf completed.
 
  long min_now = millis()/1000;
  min_now = min_now/60;
  min_display = min_total - min_now - min_start; 
  displayMins(min_display);
} 

void displayMins(int min_display)
// set clock display to hh:mm 
{ 
      uint8_t segto;
      int hours = int(min_display/60); 
      int cont = hours*100 + min_display-hours*60;
   
      display.showNumberDec(cont, true);
  
      cont++;
  
      segto = 0x80 | display.encodeDigit((cont / 100)%10);
      display.setSegments(&segto, 1, 1);
}  


void delayUpdate(long minutes)
{
  if (minutes != 0)
    {
      // check stop button every second
      long n = 60 * minutes;
      for (int i=0; i<n;i++)
      {
        delay(1000);
        if (checkStop()){return;}      
      }
    }  
}

void pulseMix(long minutes) {  
  // pulse motor at pulseMixON and pulseMixOFF for mills
  const int pulseMixON = 200; // time in millseconds for ON time of pulse mixing
  const int pulseMixOFF = 1000; // time in millseconds for OFF  time of pulse mixing
  if (minutes != 0)
  {
    long mills = minutes*1000*60; 
    int n =  mills/(pulseMixON + pulseMixOFF);
    for (int i=0; i<n;i++)
       {     
  
         digitalWrite(LED_PIN, HIGH);
         digitalWrite(motor, HIGH);
         delay(pulseMixON);
         digitalWrite(LED_PIN, LOW);
         digitalWrite(motor, LOW);
         delay(pulseMixOFF);
         if (checkStop()){return;}
       } 
  }     
}  

void contMix(long minutes) 
{
  if (minutes != 0)
  {
    // check stop button every second
    long n = 60 * minutes;
    for (int i=0; i<n;i++)
    {
         digitalWrite(LED_PIN, HIGH);
         digitalWrite(motor, HIGH);
         delay(1000);
         if (checkStop()){return;}
    }
    digitalWrite(LED_PIN, LOW);
    digitalWrite(motor, LOW);
  }
}    

void rise(long minutes)
{
  // this gives the heater a pulse every 60 seconds
  //the width of the pulse depends on the thermistor reading
  const int thigh = 110;  
  const int tlow = 98;  
  
  int thermistorValue = 0;        // value read from the thermistor 
  
  int t_on = 0;  
  int t_off = 0; 
  if (minutes != 0)
  {
   
    long n = minutes;
    for (int i=0; i<n;i++)
    {
      thermistorValue = analogRead(thermistor);  
      // print the results to the serial monitor:  
      Serial.print("thermistor, " );                         
      Serial.println(thermistorValue);    
             
      if (thermistorValue > thigh)
      {  
        t_off = 59;  
        t_on  = 1;  
      }  
      if (thermistorValue < tlow)
      {   
        t_off = 50;  
        t_on  = 10;  
      }  
      if (thermistorValue <= thigh && thermistorValue >= tlow)
      {  
        t_off = 54;  
        t_on  = 6;   
      }      
      digitalWrite(heater, HIGH);  
      for (int j=0; j<t_on;j++)
      {
        delay(1000);
        if (checkStop()){return;}
      }    
      digitalWrite(heater, LOW);  
      for (int j=0; j<t_off;j++)
      {
        delay(1000);
        if (checkStop()){return;}
      }
    }  
  }  
}  

void punchDown(long minutes)
{
  if (minutes != 0)
  {
     digitalWrite(LED_PIN, HIGH);
     digitalWrite(motor, HIGH);
     delay(5000);
     digitalWrite(LED_PIN, LOW);
     digitalWrite(motor, LOW);
     delay(10000);
     digitalWrite(LED_PIN, HIGH);
     digitalWrite(motor, HIGH);
     delay(5000);
     digitalWrite(LED_PIN, LOW);
     digitalWrite(motor, LOW);
  }  
}

void heat(int minutes)
{
  //turn the heater on until thermistor reaches tlow
  // if minutes == 0 routine is skipped 
  // otherwise it is ignored function continues until tlow is reached
  const int tlow = 680;  
  int thermistorValue = 0;        // value read from the thermistor 
  if (minutes != 0)
  {
    digitalWrite(LED_PIN, HIGH); 
    digitalWrite(heater, HIGH);
  
    do
    {
      delay(1000);
      if (checkStop()){return;}
      thermistorValue = analogRead(thermistor);  
      // print the results to the serial monitor:  
      Serial.print("thermistor, " );                         
      Serial.println(thermistorValue);
    } while (thermistorValue < tlow);
    digitalWrite(LED_PIN, LOW);
    digitalWrite(heater, LOW);  
  }  
}  
     
void bake(long minutes)
{
  // this gives the heater a pulse every 15 seconds
  //the width of the pulse depends on the thermistor reading  
   
  const int thigh = 710;  
  const int tlow = 680;  
  
  int thermistorValue = 0;        // value read from the thermistor 
  
  int t_on = 0;  
  int t_off = 0; 
  if (minutes != 0)
  {
    long n = 4*minutes;
    for (int i=0; i<n;i++)
    {
      thermistorValue = analogRead(thermistor);  
      // print the results to the serial monitor:  
      Serial.print("thermistor, " );                         
      Serial.println(thermistorValue);  
      if (thermistorValue > thigh)
      {  
          t_off = 10;  
          t_on  = 5;  
      }  
      if (thermistorValue < tlow)
      {  
          t_off = 0;  
          t_on = 15;  
      }  
      if (thermistorValue <= thigh && thermistorValue >= tlow)
      {  
          t_off = 6; // 10 0.4*15 
          t_on = 9;  //15 0.6*15
      }     
      digitalWrite(heater, HIGH);  
      for (int j=0; j<t_on;j++)
      {
        delay(1000);
        if (checkStop()){return;}
      }    
      digitalWrite(heater, LOW);  
      for (int j=0; j<t_off;j++)
      {
        delay(1000);
        if (checkStop()){return;}
      }
      
      
    } // for i = 0 to n
    digitalWrite(LED_PIN, LOW);
    digitalWrite(heater, LOW);
    digitalWrite(motor, LOW);
  }  
}  
