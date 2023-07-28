///////////////////////////////////////////////////////////////////////
// Trombone mouse controller
// Need Arduino IDE and Teensyduino https://www.pjrc.com/teensy/td_download.html
// Need MOUSE library installed
// Board type = "Teensy LC"
// USB type = "Keyboard + Mouse + Joystick"
// CPU speed = "48 MHz"
// Port = (select Teensy HID device from list)

#include "Mouse.h"

// The mouse scaling assumes a specific screen size defined below
const int MAX_X = 1920;
const int MAX_Y = 1080;

// This value controls the low pass filtering of the ADC value (sampled once per ms). Smaller value = more smoothing
const double FILTER_NUM = 0.05;

// define the range of ADC values that map to mouse positions on the screen
const int MIN_ADC_YVALUE = 700; 
const int MAX_ADC_YVALUE = 880; 

// Y ADC reading above which trombone is vertical
const int MIN_ACTIVE_YVALUE = 950; 

// hardware pins
const int P_LED = 13;
const int P_TRIG = 12;
const int P_FIRE = 20;
const int P_ADC_X = 0;
const int P_ADC_Y = 2;

//////////////////////////////////////////////////////////////////////
template<int _PIN, int _MAX> class CAccelAxis {
public:  
  int m_pos;
  int m_pos_max;
  int m_min;
  int m_max;
  double m_value;
  double m_target_pos;

  ///////////
  CAccelAxis() {
    reset();
  }
  
  ///////////
  void reset() {
    m_min = MIN_ADC_YVALUE; // should pass in as parameters/template args, meh
    m_max = MAX_ADC_YVALUE;
    m_value = 0;
   }
  
  ///////////
  void run() {
    // perform low pass filtering on the raw reading
    int reading = analogRead(_PIN);
    m_value = (1.0-FILTER_NUM)*m_value + FILTER_NUM*reading;
  
    if(m_value<m_min) {
      m_target_pos = _MAX;
    }
    else if (m_value>m_max) {
      m_target_pos = 0;    
    }
    else {
      m_target_pos = _MAX - ((m_value-m_min) / (m_max-m_min)) * _MAX;    
    }
  }
};

CAccelAxis<P_ADC_X, MAX_X> accelX;
CAccelAxis<P_ADC_Y, MAX_Y> accelY; // not currently used!
int lastTrig = 1;
int trigCount= 0;
float pwm_lev = 0;

//////////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(19200);
  pinMode(23, OUTPUT);
  digitalWrite(23,LOW);
  delay(10);
  digitalWrite(23,HIGH);
  pinMode(P_LED,OUTPUT);
  digitalWrite(P_LED,LOW);
  pinMode(P_TRIG,INPUT_PULLUP);

  // configure screen size
  Mouse.screenSize(MAX_X, MAX_Y);  

  // take control of the mouse:
  Mouse.begin();
  
}

//////////////////////////////////////////////////////////////////////
void loop() {
  accelX.run();
  accelY.run();

  // decide if mouse control is disabled due to vertical orientation 
  int update_mouse = (accelY.m_value < MIN_ACTIVE_YVALUE);
  if(update_mouse) {
    Mouse.moveTo(500, accelY.m_target_pos); // uses absolute position!
  }

  // read state of the hall effect (magnet) sensor
  int thisTrig = digitalRead(P_TRIG);

  // has it changed?
  if(thisTrig != lastTrig) {
    lastTrig = thisTrig;    
    
    if(update_mouse) {
      trigCount = 100;  // this is the timeout in ms that the button remains held each time magnet movement detected
      pwm_lev = 255;  // the brightness level of the LEDs on trombone 
      digitalWrite(P_LED,HIGH);  // LED on the teensy board
      Mouse.set_buttons(1, 0, 0); // press mouse button down!
    }
  } 
  else if(trigCount) {

    // time out the mouse button
    if(!--trigCount) {
      Mouse.set_buttons(0, 0, 0);  
      digitalWrite(P_LED,LOW);
    }
  }

  // set brightness levels of the LEDs on the trombone
  analogWrite(P_FIRE, pwm_lev);

  // the fade rate of the trombone LEDs, smaller number = quicker fade
  pwm_lev *=.99;

  // delay 1ms
  delay(1); 
}


//////
// testing gumph
void xloop() {
  
  //Serial.print(accelX.m_target_pos);
  //Serial.print(",");
  //Serial.print(accelY.m_target_pos);
  //Serial.println(" ");
  //Serial.println(accelY.m_value);
  //Serial.print(" ");
  Serial.print(analogRead(P_ADC_Y));
  //Serial.print(" ");
  //Serial.print(analogRead(2));
  Serial.println(" ");
  delay(50);
}
