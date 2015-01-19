/*
Part of arduino-bread project

Reads voltages of thermistor, motor and heater control signals for breadmaker
connected to its original control board.

This is to monitor original control signals to help in developing an arduino
board replacement.

circuit and documentation on www.elfnor.com/Arduino Bread Details.html
code https://github.com/elfnor/arduino-bread

This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License.

elfnor


 */
#define heater 3
#define motor 2

const int thermistor = A0;      // Analog input pin that the thermistor is connected to 

void setup() {
  // 10 second delay to make it easier to upload new programs
  delay(10000); 
  
  pinMode(motor, INPUT);
  pinMode(heater, INPUT); 
  
  // initialize serial communications at 9600 bps:
  Serial.begin(9600); 
}

void loop() { 
  // read the analog in value:
  int s1 = digitalRead(heater);
  int s2 = digitalRead(motor);
  int s3 = analogRead(thermistor);
      
  // print the results to the serial monitor:                    
  Serial.print(s1);
  Serial.print(",  ");
  Serial.print(s2);  
  Serial.print(",  ");
  Serial.println(s3); 

  // wait before the next loop
  //motor and heater on cycles vary from 3 to 5 seconds when pulsing
  delay(100);         
}            


