#include <Arduino.h>  // Thư viện Arduino cơ bản.
int ls1=8;
int ls2=9;
int S1=41;
int S2=43;
int e1=36;
int e2=38;
int i1=12;
int i2=11;
int H=180;
void forward(){
  digitalWrite(i1,HIGH);
  digitalWrite(i2,LOW);
  analogWrite(e1,H);
  analogWrite(e2,H);
}
void goback(){
  digitalWrite(i1,LOW);
  digitalWrite(i2,HIGH);
  analogWrite(e1,H);
  analogWrite(e2,H);
}
void stop(){
  digitalWrite(i1,LOW);
  digitalWrite(i2,LOW);
  analogWrite(e1,0);
  analogWrite(e2,0);
}
void setup(){
  Serial.begin(9600);
  pinMode(ls1,INPUT_PULLUP);
  pinMode(ls2,INPUT_PULLUP);
  pinMode(S1,INPUT);
  pinMode(S2,INPUT);
  pinMode(i1,OUTPUT);
  pinMode(i2,OUTPUT);
  pinMode(e1,OUTPUT);
  pinMode(e2,OUTPUT);
}
void loop(){
  int rls1=digitalRead(ls1);
  int rls2=digitalRead(ls2);
  int rS1=digitalRead(S1);
  int rS2=digitalRead(S2);

    if(rls1 == 1 && rls2 ==1 && rS1 ==1 && rS2 ==1){
      forward();
    }
    else if(rls1 == 1 && rls2 ==1 && rS1 ==1 && rS2 ==0){
      forward();
    }
    else if(rls1 == 1 && rls2 ==1 && rS1 ==0 && rS2 ==0){
      goback();
    }
    else if(rls1 == 1 && rls2 ==0 && rS1 ==0 && rS2 ==0){
      goback();
    }
    else if(rls1 == 0 && rls2 ==1 && rS1 ==1 && rS2 ==1){
      forward();
    }
    else if(rls1 == 0 && rls2 ==1 && rS1 ==0 && rS2 ==0){
      stop();
    }
    else if(rls1 == 0 && rls2 ==1 && rS1 ==1 && rS2 ==0){
      forward();
    }
    else if(rls1 == 0 && rls2 ==1 && rS1 ==0 && rS2 ==1){
      forward();
    }
    else if(rls1 == 1 && rls2 ==0 && rS1 ==1 && rS2 ==0){
      stop();
    }
    else if(rls1 == 1 && rls2 ==0 && rS1 ==0 && rS2 ==1){
      stop();
    }
    else if(rls1 == 1 && rls2 ==0 && rS1 ==1 && rS2 ==1){
      stop();
    }
    else if(rls1 == 0 && rls2 ==0 && rS1 ==1 && rS2 ==1){
      stop();
    }
  Serial.print(rls1);
  Serial.print(rls2);
  Serial.print(rS1);
  Serial.println(rS1);
  delay(100);
}
