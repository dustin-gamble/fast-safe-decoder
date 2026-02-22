/****************************************************************
Combation Safe Cracker

Sparrows Vault Safe
T-Motor U-8 motor
Spark fun: Teensy 4.0, analog mic

Odrive:
In [14]: odrv0.axis0.controller.config
Out[14]:
control_mode = 3 (int)
pos_gain = 20.0 (float)
vel_gain = 0.000699999975040555 (float)
vel_integrator_gain = 0.006000000052154064 (float)
vel_limit = 7000.0 (float)
vel_limit_tolerance = 100.0 (float)
vel_ramp_rate = 10000.0 (float)
setpoints_in_cpr = False (bool)

Started 1 Feb 2020

by
Dustin Gamble

Today is 16 March 2020 
14 March 2020: First complete safe crack
16 March 2020:  Second complete safe crack, v0.33
***************************************************************/


#include <Wire.h>

#include "SparkFun_Qwiic_Keypad_Arduino_Library.h" //Click here to get the library: http://librarymanager/All#SparkFun_keypad
KEYPAD keypad1; //Create instance of this object

#include "EncoderStepCounter.h"


#define ENCODER_PIN1 2
#define ENCODER_INT1 digitalPinToInterrupt(ENCODER_PIN1)
#define ENCODER_PIN2 3
#define ENCODER_INT2 digitalPinToInterrupt(ENCODER_PIN2)

// Create instance for one half step encoder
EncoderStepCounter encoder(ENCODER_PIN1, ENCODER_PIN2, HALF_STEP);


#define SERIAL_PORT SerialUSB
#define HWSERIAL Serial1
#define DISPLAY_ADDRESS1 0x72 //This is the default address of the OpenLCD


String input1;
String input2;
float dial0;
float dial1;
float dial2;
float dial22;
int dial33;
float dial33_2;
int dial33_44;
float dial33_244;
float dial44;
int dial55;
int dialsetpoint;
float diallow;
float diallow5;
float dialhigh;
int dialhighpoint;
float dialhigha;
float diallowa;
float bestdial;
float dialcompare;
float dialprevious;
float mic;
float mic1;
float mic1d;
float mic2;
float mic2d;
float mic1dd;
float mic1_2;
float mic1d_2;
float mic2_2;
float mic2d_2;
float mic1dd_2;
float loopstart;
float looptime;
double timestart;
double timeend;
float tts;
float tte;
int sync;
int sync2;
long rawencoder;
long secondencoder;
signed long position = 0;
char button;
int runflag;
float timeoutstart;
int endflag;
float dialopen;
int dialstep;



void setup() {

//print to computer for dignostic
  SERIAL_PORT.begin(115200);
//setup serial link to Odrive
  HWSERIAL.begin(115200);

  encoder.begin();
  // Initialize interrupts
  attachInterrupt(ENCODER_INT1, interrupt, CHANGE);
  attachInterrupt(ENCODER_INT2, interrupt, CHANGE);

  keypad1.begin();


//initialize variables
  dialhighpoint = 25*81.92;
  dialhigh = -100;
  bestdial = 100;

//wait for odrive to boot
delay(1000);
//command o drive to zero position
HWSERIAL.print("p 0 0");

delay(100);

//OpenLCD

  Wire.beginTransmission(DISPLAY_ADDRESS1);
  Wire.write('|'); //Put LCD into setting mode
  Wire.write('-'); //Send clear display command

  Wire.write('|'); //Put LCD into setting mode
  Wire.write('+'); //Send the Set RGB command
  Wire.write(0xFF); //Send the red value
  Wire.write(0xFF); //Send the green value
  Wire.write(0xFF); //Send the blue value
  Wire.print("Dial: ");

  
  Wire.endTransmission();


  //for debugging
  dial33 = 72;
  dial44 = 17;
  dial33_44 = 17;
  button = 0;
  runflag = 0;
  endflag = 0;
  dialstep = 2;

}


void interrupt() {
  encoder.tick();
}



void loop() {
  
  //delay to slow down loop
  delay(10);

  //collect keypad input
  keypad1.updateFIFO();  
  button = keypad1.getButton();
  Serial.print("Button value: ");
  Serial.print(button);
  Serial.print(" , ");

  //encoder capture per run
  signed char pos = encoder.getPosition();
  if (pos != 0) {
    position += pos;
    encoder.reset();

  }

  dial2 = position / (20.48*2);
  Serial.print(dial2);
  Serial.print(", ");

//collect analog microphone reading
  mic = analogRead(0);

  // odrive getting encoder status
  HWSERIAL.println("f 0");
  input1 = HWSERIAL.readStringUntil(' ');
  input2 = HWSERIAL.readStringUntil('\n');
  

  dial0 = input1.toInt();
  dial1 = dial0 / 81.92;

  position = dial0/2;

  Serial.print(dial1);
  Serial.print(", ");
  Serial.print(mic);
  Serial.print(", ");


//real time dial spinning commands
  if(button == '*'){
    tts = millis();
      runflag = 10;
  }


    if(button == '7'){
        dialsetpoint = dialsetpoint+4000;
  }

      if(button == '8'){
        dialsetpoint = dialsetpoint-4000;
  }
  

  
  if(button == '4'){
  
    dialsetpoint = dialhighpoint;
      diallow = 103;
      diallowa = 0;
      diallow5 = 104;
      mic1=100;
      mic1d=105;
      mic2=100;
      mic2d=106;
    
    //command dial position
    
    HWSERIAL.print("p 0 ");
    HWSERIAL.println(dialsetpoint);
    
    //start logging spin loop, for loop
    
            while(dial0 <= (dialsetpoint-200)){
              
                    //new encoder
                      signed char pos = encoder.getPosition();
                      if (pos != 0) {
                        position += pos;
                        encoder.reset();
                       // Serial.println(position);
                      }
                    dial1 = position / (20.48*2);
                    dial0 = dial1 * 81.92;
                   Serial.print(dial1);
                   Serial.print(" , ");
                    

                  mic = analogRead(0);

                  Serial.println(mic);
            
                  //mic high points
                  if(mic > 700 && dial1 > 1 && dial1 <6){
                    mic1 = mic;
                    mic1d = dial1;
                  }
                  
                  if(mic > 700 && dial1 > 13 && dial1 <17){
                    mic2 =  mic;
                    mic2d = dial1;
                  }
      
    }
    
  }
if(button == '5'){

dialsetpoint = -500;

  dialhigh = 110;
  diallow = 111;

  diallowa = 0;
  diallow5 = 112;


  mic1=100;
  mic1d=113;
  mic2=100;
  mic2d=114;

    //command dial position
    
    HWSERIAL.print("p 0 ");
    HWSERIAL.println(dialsetpoint);
    
    //start logging spin loop, for loop
    
            while(dial0 >= (dialsetpoint+200)){
            
            //collect

                    //new encoder
                      signed char pos = encoder.getPosition();
                      if (pos != 0) {
                        position += pos;
                        encoder.reset();
                       // Serial.println(position);
                      }
                    dial1 = position / (20.48*2);
                    dial0 = dial1 * 81.92;
                    
                   Serial.print(dial1);
                  Serial.print(" , ");
                   

                  mic = analogRead(0);
                  
                    Serial.println(mic);

                        //mic high points
                  if(mic > 700 && dial1 > 2 && dial1 <6){
                    mic1 = mic;
                    mic1d = dial1;
                  }
                  
                  if(mic > 700 && dial1 > 13 && dial1 <17){
                    mic2 =  mic;
                    mic2d = dial1;
                  }

          }
      }

  Serial.print(", ");
  Serial.print(dialsetpoint/81.92);
  
  HWSERIAL.print("p 0 ");
  HWSERIAL.println(dialsetpoint);
  
  HWSERIAL.flush();
  // encoder status complete
  
  
  //mic high points
  if(mic > 600 && mic > mic1  && dial1 > 2 && dial1 <6){
        mic1 = mic;
        mic1d = dial1;
      }
  
  if(mic > 600 && mic > mic2  && dial1 > 13 && dial1 <17){
        mic2 =  mic;
        mic2d = dial1;
      }

    Serial.print(",mic CP1 , ");
    Serial.print(mic1d);
    Serial.print(", ");
    Serial.print(",mic CP2 , ");
    Serial.print(mic2d);
    Serial.print(", ");

  dialprevious = dial1;




 //////////////////////run big safe program////////////////////////////////////////////////////////////
          if(button == '1' || runflag == 10){

            Wire.beginTransmission(DISPLAY_ADDRESS1);
            Wire.write('|'); //Put LCD into setting mode
            Wire.write('-'); //Send clear display command
          
            Wire.write('|'); //Put LCD into setting mode
            Wire.write('+'); //Send the Set RGB command
            Wire.write(0xFF); //Send the red value
            Wire.write(0xFF); //Send the green value
            Wire.write(0xFF); //Send the blue value
            Wire.print("C:    -    -    ");
          
            Wire.endTransmission();

          
              Serial.println();
              Serial.println();
              Serial.println();
          
          loopstart = millis();

//reset safe
                        dialsetpoint = -50000;
                        HWSERIAL.print("p 0 ");
                        HWSERIAL.println(dialsetpoint);


                        while(dial0 >= dialsetpoint+100){
                            
                            //collect
                
                                    //new encoder
                                      signed char pos = encoder.getPosition();
                                      if (pos != 0) {
                                        position += pos;
                                        encoder.reset();
                                       // Serial.println(position);
                                      }
                                    dial1 = position / (20.48*2);
                                    dial0 = dial1 * 81.92;
                                    //Serial.println(dial0);
                                    //delay(1);
                        }

                        dialsetpoint = 0;
                        HWSERIAL.print("p 0 ");
                        HWSERIAL.println(dialsetpoint);


                        while(dial0 <= dialsetpoint-200){
                            
                            //collect
                
                                    //new encoder
                                      signed char pos = encoder.getPosition();
                                      if (pos != 0) {
                                        position += pos;
                                        encoder.reset();
                                       // Serial.println(position);
                                      }
                                    dial1 = position / (20.48*2);
                                    dial0 = dial1 * 81.92;
                                    //Serial.println(dial0);
                                    //delay(1);
                        }






          
            
          mic1dd = -101;
          
          Serial.println();
          Serial.println("Starting cycle program, talk to me safe");
          
 //         HWSERIAL.println("p 0 0");
          

          
          delay(100);
          
              for(int i=0; i <=70;){
          
                      dialsetpoint = (20 + i)*81.92;
                      dial22 = dialsetpoint/81.92;
                      Serial.print(dialsetpoint);
                      Serial.print(", ");
                      Serial.print(dial22);
                      delay(100);
                      
              //first time        
                      HWSERIAL.print("p 0 ");
                      HWSERIAL.println(dialsetpoint);
                      
                      while(dial0 <= (dialsetpoint-100)){
                              //Serial.println("trying");
                              signed char pos = encoder.getPosition();
                                if (pos != 0) {
                                  position += pos;
                                  encoder.reset();
                                 // Serial.println(position);
                                }
                              dial1 = position / (20.48*2);
                              dial0 = dial1 * 81.92;
                      }
          
                        delay(200);
                        dialsetpoint = -500;
                        HWSERIAL.print("p 0 ");
                        HWSERIAL.println(dialsetpoint); 
          
                          
                            //contact point section, replace with function
          
                              mic1=100;
                              mic1d=-405;
                              mic2=100;
                              mic2d=114;
                              //start logging spin loop, for loop
              
                      while(dial0 >= (dialsetpoint+100)){
                      
                      //collect
          
                              //new encoder
                                signed char pos = encoder.getPosition();
                                if (pos != 0) {
                                  position += pos;
                                  encoder.reset();
                                 // Serial.println(position);
                                }
                              dial1 = position / (20.48*2);
                              dial0 = dial1 * 81.92;
                              
                            mic = analogRead(0);
                            
                                              //mic high points
                            if(mic > 700 && dial1 > 2 && dial1 <6){
                              mic1 = mic;
                              mic1d = dial1;
                            }
                            
                            if(mic > 700 && dial1 > 13 && dial1 <17){
                              mic2 =  mic;
                              mic2d = dial1;
                            }
                      
                      }
              //end loop
          
                              
                            //end of contact point section
                            
                      Serial.print(", ");
                      Serial.print(mic1d);
                      Serial.print(", ");
//                      Serial.print(mic2d);
          
          
                      //take best value
                      if(mic1dd < mic1d){
                        mic1dd = mic1d;
                        dial33 = dial22;
                      }
                      Serial.print("Best, ");
                      Serial.print(mic1dd);
                      Serial.print(", ");
                      Serial.println(dial33);
                      delay(200);
          
                          Wire.beginTransmission(DISPLAY_ADDRESS1);

                          Wire.write(254);
                          Wire.write(128 + 64 + 0);
                          Wire.print("Run: ");
          
                          Wire.write(254);
                          Wire.write(128 + 0 + 13);
                          Wire.print(dial33);
                        
                          Wire.write(254);
                          Wire.write(128 + 64 + 6);
                          Wire.print(mic1d);

                          Wire.write(254);
                          Wire.write(128 + 64 + 11);
                          Wire.print(dial22);
                        
                          Wire.endTransmission();
             
                        i = i + dialstep;
              }
          
              looptime = (millis() - loopstart)/60000;
              Serial.println();
              Serial.println();
              Serial.println();
              Serial.print("Total run time:   ");
              Serial.print(looptime);
              Serial.println("  Minutes!  That was quick!");


                          Wire.beginTransmission(DISPLAY_ADDRESS1);

                          Wire.write(254);
                          Wire.write(128 + 64 + 0);
                          Wire.print("T:");
                          
                          Wire.write(254);
                          Wire.write(128 + 64 + 3);
                          Wire.print(looptime);
          
                          Wire.write(254);
                          Wire.write(128 + 64 + 6);
                          Wire.print(" Minutes" );
                        
                          Wire.endTransmission();


          if(runflag == 10){
          runflag = 9;
          }
          }





///////////////////////////////second big safe program////////////////////////////////////////////////////////////////

          if(button == '2' || runflag == 9){


            Wire.beginTransmission(DISPLAY_ADDRESS1);
            Wire.write('|'); //Put LCD into setting mode
            Wire.write('-'); //Send clear display command
          
            Wire.write('|'); //Put LCD into setting mode
            Wire.write('+'); //Send the Set RGB command
            Wire.write(0xFF); //Send the red value
            Wire.write(0xFF); //Send the green value
            Wire.write(0xFF); //Send the blue value
            Wire.print("C:    -    -    ");
          
            Wire.endTransmission();

          
              Serial.println();
              Serial.println();
              Serial.println();
          
          loopstart = millis();

          Serial.println();
          Serial.println("Resetting Dial with 4 Clockwise turns. (-400)");

        //reset dial assuming we are starting at 0
//        dialsetpoint = -8192 * 4;
//        HWSERIAL.print("p 0 ");
//        HWSERIAL.println(dialsetpoint);
//        
//        while(dial0 >= (dialsetpoint+10)){
//            
//            //collect
//
//                    //new encoder
//                      signed char pos = encoder.getPosition();
//                      if (pos != 0) {
//                        position += pos;
//                        encoder.reset();
//                       // Serial.println(position);
//                      }
//                    dial1 = position / (20.48*2);
//                    dial0 = dial1 * 81.92;
//                }
//          //dial has been reset
          
            
          mic1dd = 101;
           mic1dd_2 = -400;
          
          Serial.println();
          Serial.println("Starting 2nd cycle program, talk to me safe");
                   
          
          delay(500);
          
              for(int i=0; i <=98;){

                        Serial.print("Loop Start: ");
                        dialsetpoint = -8192 * 4- i * 81.92;
                        dial44 = 100-i;
                        Serial.print(dialsetpoint);
                        Serial.print(", ");
                        Serial.print(dial44);
                        Serial.print(", ");
                        
                        HWSERIAL.print("p 0 ");
                        HWSERIAL.println(dialsetpoint);
                        
                        while(dial0 >= dialsetpoint+200){
                            
                            //collect
                
                                    //new encoder
                                      signed char pos = encoder.getPosition();
                                      if (pos != 0) {
                                        position += pos;
                                        encoder.reset();
                                       // Serial.println(position);
                                      }
                                    dial1 = position / (20.48*2);
                                    dial0 = dial1 * 81.92;
                                    //Serial.println(dial0);
                                    //delay(1);
                        }
                        //Serial.print(" First Spin done, Going to 3rd disk:  ");
                      delay(1000);
                      
              //check contact points for 2nd number
              
                      dial33_2 = (dial33) * 81.92 - 8192 * 4;
                      dialsetpoint = dial33_2;
                      Serial.print(dialsetpoint);
                      
                      HWSERIAL.print("p 0 ");
                      HWSERIAL.println(dialsetpoint);

                      
                      while(dial0 <= dialsetpoint-50){
                              //Serial.println("trying");
                              signed char pos = encoder.getPosition();
                                if (pos != 0) {
                                  position += pos;
                                  encoder.reset();
                                 // Serial.println(position);
                                }
                              dial1 = position / (20.48*2);
                              dial0 = dial1 * 81.92;
                      }

                      //Serial.print(dial0);
                      Serial.print(", ");
                      Serial.print(dial1);
                      //Serial.print("Second disk complete, Checking CP: ");
                        delay(500);
                        dialsetpoint = (0) * 81.92 - 8192 * 4;
                        Serial.print(", ");
                        Serial.print(dialsetpoint);
                        HWSERIAL.print("p 0 ");
                        HWSERIAL.println(dialsetpoint); 
                   
          
                          
                            //contact point section, replace with function
          
                              mic1_2=100;
                              //mic1d_2=113;
                              mic1d_2=-400;
                              mic2_2=100;
                              mic2d_2=114;
                              //start logging spin loop, for loop
   //this is broken_i think i fixed it
                      while(dial0 >= (dialsetpoint+200)){
                      
                      //collect
          
                              //new encoder
                                signed char pos = encoder.getPosition();
                                if (pos != 0) {
                                  position += pos;
                                  encoder.reset();
                                }
                              dial1 = position / (20.48*2);
                              dial0 = dial1 * 81.92;


                              
                            mic = analogRead(0);
                            
                                              //mic high points
                            if(mic > 700 && dial1 > -399 && dial1 <-391){
                              mic1_2 = mic;
                              mic1d_2 = dial1;
                            }
                            
                            if(mic > 700 && dial1 > -387 && dial1 <-383){
                              mic2_2 =  mic;
                              mic2d_2 = dial1;
                            }

//                                  Serial.print(dial0);
//                                  Serial.print(", Target >=: ");
//                                  Serial.print(dialsetpoint);
//                                  Serial.print("Mic: ");
//                                  Serial.print(mic);
//                                  Serial.print(", Dial settings: ");
//                                  Serial.println(dial1);
                              

                      
                      }
                      Serial.print(", ");
                      Serial.print(dial1);
                      Serial.print(", ");

                      
              //end loop
          
              delay(500);
                            //end of contact point section
                            
                      //Serial.print(", mic, ");
                      Serial.print(mic1d_2);
                      Serial.print(", ");
                      Serial.print(mic2d_2);
                      Serial.print(", ");
          
          
                      //take best value
                      if(mic1d_2 > mic1dd_2){
                        mic1dd_2 = mic1d_2;
                        dial33_44 = dial44;
                      }
                      Serial.print("Best, ");
                      Serial.print(mic1dd_2);
                      Serial.print(", ");
                      Serial.println(dial33_44);
                      delay(200);
          
//                         Wire.beginTransmission(DISPLAY_ADDRESS1); // transmit to device #1
//          
//          
//                          Wire.write(254);
//                          Wire.write(128 + 64 + 0);
//                          Wire.print(mic1dd);
//                        
//                          Wire.write(254);
//                          Wire.write(128 + 64 + 8);
//                          //curA2d
//                          Wire.print(dial33);
//                        
//                          Wire.endTransmission(); //Stop I2C transmission
                
                              //reset dial assuming we are starting at 0
        //Serial.print("Back to 0 reset");                      
        dialsetpoint = 8192;
        HWSERIAL.print("p 0 ");
        HWSERIAL.println(dialsetpoint);
        
        while(dial0 <= (dialsetpoint-200)){


                    //new encoder
                      signed char pos = encoder.getPosition();
                      if (pos != 0) {
                        position += pos;
                        encoder.reset();
                       // Serial.println(position);
                      }
                    dial1 = position / (20.48*2);
                    dial0 = dial1 * 81.92;
       
                }
               // Serial.print("Reset complete!");
          //dial has been 
          delay(500);


                          Wire.beginTransmission(DISPLAY_ADDRESS1);

                          Wire.write(254);
                          Wire.write(128 + 64 + 0);
                          Wire.print("R:");
          
                          Wire.write(254);
                          Wire.write(128 + 0 + 13);
                          Wire.print(dial33);

                          Wire.write(254);
                          Wire.write(128 + 0 + 8);
                          Wire.print("   ");
                          delay(10);

                          Wire.write(254);
                          Wire.write(128 + 0 + 8);
                          Wire.print(dial33_44);

                          
                        
                          Wire.write(254);
                          Wire.write(128 + 64 + 2);
                          Wire.print(mic1d_2);

                          Wire.write(254);
                          Wire.write(128 + 64 + 11);
                          Wire.print(dial44);
                        
                          Wire.endTransmission();

                   i = i + dialstep;
              }
          
              looptime = (millis() - loopstart)/60000;
              Serial.println();
              Serial.println();
              Serial.println();
              Serial.print("Total run time:   ");
              Serial.print(looptime);
              Serial.println("  Minutes!  That was quick!");
              Serial.print("RunFlag:  ");
              Serial.println(runflag);
              Serial.println("Press 9 on keyboard to exit program.");

                          Wire.beginTransmission(DISPLAY_ADDRESS1);

                          Wire.write(254);
                          Wire.write(128 + 64 + 0);
                          Wire.print(looptime);
          
                          Wire.write(254);
                          Wire.write(128 + 64 + 5);
                          Wire.print(" Minutes  " );
                        
                          Wire.endTransmission();
              
              if(runflag !=9){
                  while(button != '9'){
                  keypad1.updateFIFO();  
                  button = keypad1.getButton();
                  delay(100);
              }
              }
            
            if(runflag == 9){
          runflag = 8;
          }
          
          }

 //////////////////////run 3rd safe program////////////////////////////////////////////////////////////
          if(button == '3' || runflag == 8){

            Wire.beginTransmission(DISPLAY_ADDRESS1);
            Wire.write('|'); //Put LCD into setting mode
            Wire.write('-'); //Send clear display command
          
            Wire.write('|'); //Put LCD into setting mode
            Wire.write('+'); //Send the Set RGB command
            Wire.write(0xFF); //Send the red value
            Wire.write(0xFF); //Send the green value
            Wire.write(0xFF); //Send the blue value
            Wire.print("C:    -    -    ");
          
            Wire.endTransmission();


              Serial.println();
              Serial.println();
              Serial.println();
          
          loopstart = millis();

          Serial.println();
          Serial.println("Resetting Dial with 4 Counter Clockwise turns. (400)");

          //*need to use this
          //mic1dd = 101;
          
          Serial.println();
          Serial.println("Starting cycle program, talk to me safe");
                   
          delay(500);

              for(int i=0; i <=90;){

                        Serial.print("Loop Start: ");
                        dialsetpoint = 8192 * 4+ i * 81.92;
                        dial55 = i;
                        Serial.print(dialsetpoint);
                        Serial.print(", ");
                        Serial.print(dial55);
                        Serial.print(", ");
                        
                        HWSERIAL.print("p 0 ");
                        HWSERIAL.println(dialsetpoint);


                        while(dial0 <= dialsetpoint-200){
                            
                            //collect
                
                                    //new encoder
                                      signed char pos = encoder.getPosition();
                                      if (pos != 0) {
                                        position += pos;
                                        encoder.reset();
                                       // Serial.println(position);
                                      }
                                    dial1 = position / (20.48*2);
                                    dial0 = dial1 * 81.92;
                                    //Serial.println(dial0);
                                    //delay(1);
                        }
                        //Serial.print(" First Spin done, Going to 2rd disk:  ");
                      delay(1000);

        ////go to second number
                      dial33_244 = dial33_44 * 81.92 + 8192 * 2;
                      dialsetpoint = dial33_244;
                      Serial.print(dialsetpoint);
                      Serial.print(", ");
                      
                      HWSERIAL.print("p 0 ");
                      HWSERIAL.println(dialsetpoint);

                      
                      while(dial0 >= dialsetpoint+100){
                              //Serial.println("trying");
                              signed char pos = encoder.getPosition();
                                if (pos != 0) {
                                  position += pos;
                                  encoder.reset();
                                 // Serial.println(position);
                                }
                              dial1 = position / (20.48*2);
                              dial0 = dial1 * 81.92;
                      }
                      
              delay(1000);
      //////go to 3rd number
                      Serial.print("Going to 3rd, ");
      
                      dial33_2 = (dial33) * 81.92 + 8192 * 3;
                      dialsetpoint = dial33_2;
                      Serial.print(dialsetpoint);
                      //Serial.print(", ");
                      
                      HWSERIAL.print("p 0 ");
                      HWSERIAL.println(dialsetpoint);

                      
                      while(dial0 <= dialsetpoint-100){
                              //Serial.println("trying");
                              signed char pos = encoder.getPosition();
                                if (pos != 0) {
                                  position += pos;
                                  encoder.reset();
                                 // Serial.println(position);
                                }
                              dial1 = position / (20.48*2);
                              dial0 = dial1 * 81.92;
                      }
        delay(1000);
      ////check if it opens
                      //Serial.print("Second disk complete, Checking CP: ");
                  for(int i=0; i <=30; i++){
                     //Serial.print(dial0);

                        delay(100);
                        dialsetpoint = 3 * 8192+(15-i)*81.92;
                        //Serial.print(dialsetpoint);
                        //Serial.print(", ");
                        HWSERIAL.print("p 0 ");
                        HWSERIAL.println(dialsetpoint); 
  
                        timeoutstart = millis();                 
                      while(dial0 >= (dialsetpoint+200)){
                      
                      //collect
                               
                              //new encoder
                                signed char pos = encoder.getPosition();
                                if (pos != 0) {
                                  position += pos;
                                  encoder.reset();
                                }
                              dial1 = position / (20.48*2);
                              dial0 = dial1 * 81.92;

                              if(((millis() - timeoutstart)/1000) > 3){
                                i = 50;
                                
                                dial0 = dialsetpoint;
                                endflag = 1;
                                dialopen = dial55;
                                break;
                                
                              }
                              delay(1);
//                              Serial.print("While dial loop:tring to get too");
//                              Serial.print(dial0);
//                              Serial.print(" , ");
//                              Serial.print(dialsetpoint+200);
//                              Serial.print(" , ");                              
//                              Serial.print(endflag);
//                              Serial.print("Time: ");
//                              Serial.println(((millis() - timeoutstart)/1000));
                      }
                  delay(100);
                  
                                if(((millis() - timeoutstart)/1000) > 10){
                                i = 20;
                                dial0 = dialsetpoint+200;
                                endflag = 1;
                                dialopen = dial55;
                                break;
                                }
//                              Serial.print("For loop");
//                              Serial.println(i);
                  }

                  if(endflag != 1){
                        //Serial.print("Back to 0 reset");                      
        dialsetpoint = 0;
        HWSERIAL.print("p 0 ");
        HWSERIAL.println(dialsetpoint);
        
        while(dial0 >= (dialsetpoint+200)){


                    //new encoder
                      signed char pos = encoder.getPosition();
                      if (pos != 0) {
                        position += pos;
                        encoder.reset();
                       // Serial.println(position);
                      }
                    dial1 = position / (20.48*2);
                    dial0 = dial1 * 81.92;
       
                }
               // Serial.print("Reset complete!");
          //dial has been 
                  }
          delay(500);
                  
                  Serial.println();              
                  

                                          Wire.beginTransmission(DISPLAY_ADDRESS1);


          
                          Wire.write(254);
                          Wire.write(128 + 0 + 13);
                          Wire.print(dial33);

                          Wire.write(254);
                          Wire.write(128 + 0 + 8);
                          Wire.print(dial33_44);

                          
                          Wire.write(254);
                          Wire.write(128 + 0 + 3);
                          Wire.print(dial55);

                          Wire.write(254);
                          Wire.write(128 + 64 + 0);
                          Wire.print("T:");
                          
                        
                          Wire.write(254);
                          Wire.write(128 + 64 + 3);
                          Wire.print((millis() - tts)/60000);

                          Wire.write(254);
                          Wire.write(128 + 64 + 8);
                          Wire.print("Minutes");
                          

                        
                          Wire.endTransmission();
                      
                  i = i + dialstep;
                  if(endflag == 1){
                    i = 100;
                  }
                
              }


            if(runflag == 8){
          runflag = 7;
          tte = (millis() - tts)/60000;
          Serial.println();
          Serial.print("Combination:  ");
          Serial.print(dialopen);
          Serial.print(" - ");
          Serial.print(dial33_44);
          Serial.print(" - ");
          Serial.println(dial33);    
          
          Serial.println("Done! Did we open it?");
          Serial.print("Total time: ");
          Serial.print(tte);
          Serial.println(" Minutes!");

            
          }
              Serial.println("Press 9 on keyboard to exit program.");
              while(button != '9'){
              keypad1.updateFIFO();  
              button = keypad1.getButton();
              delay(100);
              }

                      Wire.beginTransmission(DISPLAY_ADDRESS1);
                      Wire.write('|'); //Put LCD into setting mode
                      Wire.write('-'); //Send clear display command
                    
                      Wire.write('|'); //Put LCD into setting mode
                      Wire.write('+'); //Send the Set RGB command
                      Wire.write(0xFF); //Send the red value
                      Wire.write(0xFF); //Send the green value
                      Wire.write(0xFF); //Send the blue value
                      Wire.print("Dial: ");
                    
                      
                      Wire.endTransmission();

            
          }



      //working with LCD screen
        Wire.beginTransmission(DISPLAY_ADDRESS1); // transmit to device #1
        
        Wire.write(254);
        Wire.write(128 + 0 + 5);
        Wire.print(dial1);

        Wire.write(254);
        Wire.write(128 + 0 + 10);
        Wire.print(" ");
      
        Wire.write(254);
        Wire.write(128 + 0 + 11);
        Wire.print(dial2);
        
        Wire.write(254);
        Wire.write(128 + 64 + 0);
        Wire.print(mic1d);
      
        Wire.write(254);
        Wire.write(128 + 64 + 8);
        Wire.print(mic2d);
      
        Wire.endTransmission(); //Stop I2C transmission
      //complete working with LCD
      
Serial.println();

}



//function not in use yet
void safereset(){

//tell odrive to go to -271, about 3 turns
    HWSERIAL.print("p 0 -24000");
    dial1 = 0;
    
    while(dial0 >= -23000){

                          //new encoder
                      signed char pos = encoder.getPosition();
                      if (pos != 0) {
                        position += pos;
                        encoder.reset();
                       // Serial.println(position);
                      }
                    dial1 = position / (20.48*2);
                    dial0 = dial1 * 81.92;
                    
                    Serial.print("Setpoint: -270");
                    Serial.print(", ");
                    Serial.print("Dial readding: ");
                    Serial.println(dial1);

              }

delay(200);

       HWSERIAL.print("p 0 0");
    
    while(dial0 <= (-10)){


                          //new encoder
                      signed char pos = encoder.getPosition();
                      if (pos != 0) {
                        position += pos;
                        encoder.reset();
                       // Serial.println(position);
                      }
                    dial1 = position / (20.48*2);
                    dial0 = dial1 * 81.92;
                    
                    Serial.print("Setpoint: 0");
                    Serial.print(", ");
                    Serial.print("Dial readding:");
                    Serial.println(dial1);

              }

      Serial.println("Safe is reset!!");        

}
