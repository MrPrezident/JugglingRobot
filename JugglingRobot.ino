#include <Wire.h>
#include <Adafruit_MotorShield.h>

Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 
Adafruit_DCMotor magnet1 = *AFMS.getMotor(4);
Adafruit_DCMotor magnet2 = *AFMS.getMotor(2);


#define STEP1_PIN 10
#define DIR1_PIN 11
#define STEP2_PIN 5
#define DIR2_PIN 6
#define OPTICAL_SENSOR1_PIN 2
#define OPTICAL_SENSOR2_PIN 3

#define LEFT 0
#define RIGHT 1


unsigned long last_micro1;
unsigned long last_micro2;
unsigned long curr_micro;
unsigned long next_micro1;
unsigned long next_micro2;

int step1 = LOW;
int step2 = LOW;

#define NUM_THOW_TYPES 8
#define ARRAY_MAX 2000
int left_throw_array[NUM_THOW_TYPES][ARRAY_MAX+1];
int right_throw_array[NUM_THOW_TYPES][ARRAY_MAX+1];
int left_hand_index = 0;
int right_hand_index = 0;
int left_hand_throw_type = 0;
int right_hand_throw_type = 0;

#define SS_PERIOD 3
int siteswap_array[SS_PERIOD] = {5, 3, 4};
int left_hand_ss_index = 0;
int right_hand_ss_index = 1;

float total_steps = ARRAY_MAX;
float catch_speed = 300;
float half_period = 400000;
float prethrow_hold = 100;

void setup() 
{

  pinMode(DIR1_PIN, OUTPUT);     
  pinMode(STEP1_PIN, OUTPUT);
  pinMode(DIR2_PIN, OUTPUT);     
  pinMode(STEP2_PIN, OUTPUT);

  pinMode(OPTICAL_SENSOR1_PIN, INPUT_PULLUP);
  pinMode(OPTICAL_SENSOR2_PIN, INPUT_PULLUP);
  
  digitalWrite(DIR1_PIN, LOW);
  digitalWrite(STEP1_PIN, LOW);
  digitalWrite(DIR2_PIN, HIGH);
  digitalWrite(STEP2_PIN, LOW);
  
  AFMS.begin();  // create with the default frequency 1.6KHz
  

  goToStart(90, 1024);
  delay(500);
  setup534(); 
  
  unsigned long int beat_length = half_period*2;
  
  //Inside throws
//  digitalWrite(DIR1_PIN, LOW);
//  digitalWrite(DIR2_PIN, HIGH);
  //Outside throws
  digitalWrite(DIR1_PIN, HIGH);
  digitalWrite(DIR2_PIN, LOW);
 

  last_micro1 = micros();
  last_micro2 = last_micro1;
  next_micro1 = last_micro1;
  next_micro2= last_micro2 + beat_length;
}

void loop(){
  myloop();
}

void myloop() 
{
  if(micros() >= next_micro1){
    digitalWrite(STEP1_PIN, step1);
    last_micro1 = next_micro1;
    next_micro1 = next_micro1 + left_throw_array[siteswap_array[left_hand_ss_index]][left_hand_index];
    if(step1 == LOW) {
      if(++left_hand_index >= ARRAY_MAX){
        left_hand_index = 0;
        left_hand_ss_index = (left_hand_ss_index + 2) % SS_PERIOD;
      }
      
      if(left_hand_index == 400){
        magnet1_off();
      } else if(left_hand_index == 1700) {
        magnet1_on();
      }

      step1 = HIGH;
    } else {
      step1 = LOW;
    }
  }
  
  if(micros() >= next_micro2){
    digitalWrite(STEP2_PIN, step2);   
    last_micro2 = next_micro2;
    next_micro2 = next_micro2 + right_throw_array[siteswap_array[right_hand_ss_index]][right_hand_index];
    if(step2 == LOW) {
      if(++right_hand_index >= ARRAY_MAX){
        right_hand_index = 0;
        right_hand_ss_index = (right_hand_ss_index + 2) % SS_PERIOD;
      }
      
      if(right_hand_index == 400){
        magnet2_off();
      } else if(right_hand_index == 1700) {
        magnet2_on();
      }
      
      step2 = HIGH;
    } else {
      step2 = LOW;
    }
  }

}

void goToStart(unsigned int adj1, unsigned int adj2) {
  unsigned long next_micro1 = micros();
  unsigned long next_micro2 = micros();
  unsigned int init_steps1 = 0;
  unsigned int init_steps2 = 0;
  unsigned int extra_steps1 = 0;
  unsigned int extra_steps2 = 0;
  int step1 = LOW;
  int step2 = LOW;
  bool found_home1 = false;
  bool found_home2 = false;
  bool done1 = false;
  bool done2 = false;
  
  int initial1 = digitalRead(OPTICAL_SENSOR1_PIN);
  int initial2 = digitalRead(OPTICAL_SENSOR2_PIN);
  
  
  while(!done1 || !done2) {
    
    if(!done1 && micros() >= next_micro1) {
      next_micro1 = next_micro1 + 500;
      if(step1 == LOW) {
        step1 = HIGH;
        if(found_home1){
          extra_steps1++;
          if(extra_steps1 >= adj1) {
            done1 = true;
          }
        } else {
          init_steps1++;
        }
      } else {
        step1 = LOW;
        if(initial1 == HIGH) {
          if(init_steps1 >= 100) {
            if(digitalRead(OPTICAL_SENSOR1_PIN) == LOW) {
              found_home1 = true;
            }
          }
        } else {
          initial1 = digitalRead(OPTICAL_SENSOR1_PIN);
          init_steps1 = 0;
        }
      }
      digitalWrite(STEP1_PIN, step1);
    }

    if(!done2 && micros() >= next_micro2) {
      next_micro2 = next_micro2 + 500;
      if(step2 == LOW) {
        step2 = HIGH;
        if(found_home2){
          extra_steps2++;
          if(extra_steps2 >= adj2) {
            done2 = true;
          }
        } else {
          init_steps2++;
        }
      } else {
        step2 = LOW;
        if(initial2 == HIGH) {
          if(init_steps2 >= 100) {
            if(digitalRead(OPTICAL_SENSOR2_PIN) == LOW) {
              found_home2 = true;
            }
          }
        } else {
          initial2 = digitalRead(OPTICAL_SENSOR2_PIN);
          init_steps2 = 0;
        }
      }
      digitalWrite(STEP2_PIN, step2);
    }

  }
}

void throwSetup(int hand, unsigned int throw_type, unsigned int throw_step_int, unsigned int throw_speed_int) {
  //globals
  //float total_steps = 2000
  //float catch_speed = 300
  //float half_period = 400000
  //float prethrow_hold = 100
  
  float throw_step = ((float)throw_step_int);
  float prethrow_step = ((float)throw_step_int) - ((float)prethrow_hold);
  float throw_speed = ((float)throw_speed_int);
  float postthrow_speed = round(1.1*throw_speed);

  float prethrow_time = throw_speed * prethrow_hold;
  float half1_time = half_period - prethrow_time;
  float half2_time = half_period;
  
  float mid1_speed = 2*half1_time/prethrow_step - catch_speed/2 - throw_speed/2;
  float mid1_step = prethrow_step/2;

  
  float mid2_speed = 2*half2_time/(total_steps-throw_step) - postthrow_speed/2 - catch_speed/2;
  float mid2_step = (throw_step+total_steps)/2;
  
  // speed1(x) = m1*x + n1
  float m1 = (mid1_speed - catch_speed)/mid1_step;
  float n1 = catch_speed;
  // speed2(x) = m2*x + n2
  float m2 = (throw_speed - mid1_speed)/(prethrow_step - mid1_step);
  float n2 = throw_speed - prethrow_step * m2;
  // speed3(x) = m3*x + n3
  float m3 = (postthrow_speed - mid2_speed)/(throw_step - mid2_step);
  float n3 = postthrow_speed - throw_step * m3;
  // speed2(x) = m4*x + n4
  float m4 = (catch_speed - mid2_speed)/(total_steps - mid2_step);
  float n4 = catch_speed - total_steps * m4;

  //Values between catch and throw
  float remaining_speed = 0;
  for(float x=0; x <= prethrow_step; x++) {
    int i = (int)x;
    //Use x+0.5 instead of x because trapezoid approximation ( x+0.5 = (x+(x+1))/2 )
    float float_speed = (x <= mid1_step) ? m1*(x+0.5)+n1 + remaining_speed : m2*(x+0.5)+n2 + remaining_speed;
    
    int rounded_speed = (int)(round(float_speed));
    remaining_speed = float_speed - ((float)rounded_speed);
    
    if(hand == LEFT){
      left_throw_array[throw_type][i] = rounded_speed;
    } else if(hand == RIGHT) {
      right_throw_array[throw_type][i] = rounded_speed;
    }
  }
  
  //For prethrow hold speed steady
  for(int i=(int)prethrow_step; i < throw_step_int; i++) {
    if(hand == LEFT){
      left_throw_array[throw_type][i] = throw_speed_int;
    } else if(hand == RIGHT) {
      right_throw_array[throw_type][i] = throw_speed_int;
    }
  }
  
  //For values between throw and catch
  remaining_speed = 0;
  for(float x=throw_step; x <= total_steps; x++) {
    int i = (int)x;
    //Use x+0.5 instead of x because trapezoid approximation ( x+0.5 = (x+(x+1))/2 )
    float float_speed = (x <= mid2_step) ? m3*(x+0.5)+n3 + remaining_speed : m4*(x+0.5)+n4 + remaining_speed;
    
    int rounded_speed = (int)(round(float_speed));
    remaining_speed = float_speed - ((float)rounded_speed);
    
    if(hand == LEFT){
      left_throw_array[throw_type][i] = rounded_speed;
    } else if(hand == RIGHT) {
      right_throw_array[throw_type][i] = rounded_speed;
    }
  }
  
}

void setup534(){
  throwSetup(LEFT,5,1047,110);
  throwSetup(LEFT,3,1350,300);
  throwSetup(LEFT,4,1080,150);
  
  throwSetup(RIGHT,5,1060,114);
  throwSetup(RIGHT,3,1350,300);
  throwSetup(RIGHT,4,1095,150);
}

void magnet1_on(){
    magnet1.setSpeed(255);
    magnet1.run(FORWARD);
}

void magnet1_off(){
  magnet1.run(RELEASE);
}

void magnet2_on(){
    magnet2.setSpeed(255);
    magnet2.run(FORWARD);
}

void magnet2_off(){
  magnet2.run(RELEASE);
}
