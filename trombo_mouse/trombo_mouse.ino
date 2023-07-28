#include "Mouse.h"
const int MAX_X = 1920;
const int MAX_Y = 1080;
const double FILTER_NUM = 0.05;
const int P_LED = 13;
const int P_TRIG = 12;
const int P_FIRE = 20;
const int P_ADC_X = 0;
const int P_ADC_Y = 2;
const int MIN_ACTIVE_YVALUE = 950; // Y ADC reading above which trombone is vertical
template<int _PIN, int _MAX> class CAccelAxis {
public:  
  int m_pos;
  int m_pos_max;
  int m_min;
  int m_max;
  double m_value;
  double m_target_pos;
CAccelAxis() {
  reset();
}
void reset() {
  m_min = 700;
  m_max = 880;
  m_value = 0;
 }
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
CAccelAxis<P_ADC_Y, MAX_Y> accelY;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(19200);
  pinMode(23, OUTPUT);
  digitalWrite(23,LOW);
  delay(10);
  digitalWrite(23,HIGH);
  pinMode(P_LED,OUTPUT);
  digitalWrite(P_LED,LOW);

  pinMode(P_TRIG,INPUT_PULLUP);

  
  // take control of the mouse:
  Mouse.screenSize(MAX_X, MAX_Y);  // configure screen size

  Mouse.begin();
  
}


int lastTrig = 1;
int trigCount= 0;
float pwm_lev = 0;
void loop() {
  // put your main code here, to run repeatedly:
  accelX.run();
  accelY.run();
  int update_mouse = (accelY.m_value < MIN_ACTIVE_YVALUE);
  if(update_mouse) {
    Mouse.moveTo(500, accelY.m_target_pos);
  }

  int thisTrig = digitalRead(P_TRIG);
  if(thisTrig != lastTrig) {
    if(update_mouse) {
      lastTrig = thisTrig;
      trigCount = 100;
      pwm_lev = 255;
      digitalWrite(P_LED,HIGH);
      Mouse.set_buttons(1, 0, 0);
    }
  } 
  else if(trigCount) {
    if(!--trigCount) {
      Mouse.set_buttons(0, 0, 0);  
      digitalWrite(P_LED,LOW);
    }
  }
  analogWrite(P_FIRE, pwm_lev);
  pwm_lev *=.99;
  delay(1);
}

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
