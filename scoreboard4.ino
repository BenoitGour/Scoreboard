#include <Metro.h>
#include <EEPROM.h>

int SER_Pin = 9;    //pin 14 on the 75HC595
int RCLK_Pin = 10;  //pin 12 on the 75HC595
int SRCLK_Pin = 11; //pin 11 on the 75HC595

#define B1pin 2     //player 1 button
#define B2pin 3     //player 2 button
#define B3pin 4     //timer button
#define B4pin 5     //period length timer
#define Hornpin 6   //la horn

#define _perLen 0x0FE

Metro tmr = Metro(1000);
boolean  tmrOn;
byte perLen;
int perMLeft;
int perSLeft;

byte P1score;
byte P2score;

byte B1current = LOW;
byte B1last = LOW;
byte B2current = LOW;
byte B2last = LOW;
byte B3current = LOW;
byte B3last = LOW;
byte B4current = LOW;
byte B4last = LOW;

int B1hold=0;
int B2hold=0;
int B3hold=0;
int B4hold=0;
#define numOfRegisterPins 64
boolean registers[numOfRegisterPins];
boolean HornON = false;
int HornTime;
long numbers[10] =
{
  0xFC,  //0
  0x60,  //1
  0xDA,  //2
  0xF2,  //3
  0x66,  //4
  0xB6,  //5
  0xBE,  //6
  0xE0,  //7
  0xFE,  //8
  0xF6,  //9
};

void setup()
{
  pinMode(SER_Pin, OUTPUT);
  pinMode(RCLK_Pin, OUTPUT);
  pinMode(SRCLK_Pin, OUTPUT);

  pinMode(B1pin,INPUT);
  pinMode(B2pin,INPUT);
  pinMode(B3pin,INPUT);
  pinMode(B4pin,INPUT);  

  pinMode(Hornpin,OUTPUT);
  digitalWrite(Hornpin,LOW);

  tmrOn=false;
  
  clearRegisters();
  writeRegisters();

  Serial.begin(9600);
  
  resetTime();
}

void loop()
{
  //read button states
  B1current = digitalRead(B1pin);
  B2current = digitalRead(B2pin);
  B3current = digitalRead(B3pin);
  B4current = digitalRead(B4pin);

  doButton1();
  doButton2();
  doButton3();
  doButton4();
  
  B1last = B1current;
  B2last = B2current;
  B3last = B3current;
  
  doTimer();
  setDisplay();
}

//do player 1 button code
void doButton1()
{
  Serial.print("Button1: ");
  Serial.print("Current: ");
  Serial.print(B1current,BIN);
  Serial.print(" Last: ");
  Serial.print(B1last,BIN);
  
  if (B1current ==HIGH && B1last ==LOW)
  {
     Serial.println("B1 Pressed");
    B1hold = millis(); //time a witch button was pressed
  }
  else if (B1current == LOW && B1last==HIGH)
  {
     Serial.println("B1 Released");
    if (millis() - B1hold <500) // button held for < 500ms
    {
      P1score++;
      if (P1score >= 100) P1score=0;
    }
    else if (millis() - B1hold > 1000) // button held for > 1s
    {
      P1score = 0;
    }
  }
  Serial.print(" Hold: ");
  Serial.println(B1hold);
  B1hold = millis();
}






//do player 2 button code
void doButton2()
{
  if (B2current ==HIGH && B2last ==LOW)
  {
     Serial.println("B2 Pressed");
    B2hold = millis();
  }  
  else if (B2current == LOW && B2last==HIGH)
  {
   Serial.println("B2 Released");
    if (millis() - B2hold <500)
    {
      P2score++;
      if (P2score >= 100) P2score=0;
    }
    else if (millis() - B2hold > 1000)
    {
      P2score = 0;
    }
  }
  
  B2hold=millis();
}









//do timer button code
void doButton3()
{
  if (B3current ==HIGH && B3last ==LOW)
  {
     Serial.println("B3 Pressed");
    B3hold = millis();
  }  
  else if (B3current == LOW && B3last==HIGH)
  {
     Serial.println("B3 Released");
    if (millis() - B3hold <500)
    {
      tmrOn = !tmrOn; //switch timer on / off
    }
    else if (millis() - B3hold > 1000)
    {
      resetTime();
    }
  }
  B3hold=millis();
}








//do period button code
void doButton4()
{
  if (B4current == LOW && B4last==HIGH)
  {
    
    Serial.println("B4 Released");
    
    byte p = EEPROM.read(_perLen);
    p+=5;
    if (p > 60) p=5;
    EEPROM.write(_perLen,p);
    
    resetTime();
    B4last = B4current;
  }
}








//do timer code
void doTimer()
{
  if (tmr.check() == 1)
  {

    if (HornON == true && HornTime >=3)
    {
      digitalWrite(Hornpin,LOW);
      HornON = false; 
     
    }
    else if (HornON == true && HornTime <3) HornTime++;


    if (tmrOn)
    {
      perSLeft--;
      if (perSLeft == -1)
      {
        perSLeft = 59;
        perMLeft --;
      }

      if (perSLeft ==0 && perMLeft ==0)
      {
        tmrOn = false;
        HornON= true;
        HornTime = 0;
        digitalWrite(Hornpin,HIGH);
     
      }
    }
  }
}








void resetTime()
{
  tmrOn=false;
  perMLeft = EEPROM.read(_perLen);

  if (perMLeft > 60) perMLeft=5;
  EEPROM.write(_perLen,perMLeft);
  
  perMLeft--;
  perSLeft = 59;
  setDisplay();
}

//set all register pins to LOW
void clearRegisters(){
  for(int i = numOfRegisterPins - 1; i >=  0; i--){
    registers[i] = LOW;
  }
} 

//Set and display registers
//Only call AFTER all values are set how you would like (slow otherwise)    numOfRegisterPins -
void writeRegisters(){
  digitalWrite(RCLK_Pin, LOW);

  for(int i =  numOfRegisterPins-1; i >=  0; i--)
  {
    digitalWrite(SRCLK_Pin, LOW);
    int val = registers[i];
    digitalWrite(SER_Pin, val);
    digitalWrite(SRCLK_Pin, HIGH);
  }
  digitalWrite(RCLK_Pin, HIGH);

}

//set an individual pin HIGH or LOW
void setRegisterPin(int index, int value){
  registers[index] = value;
}

void setDisplay()
{
  clearRegisters();

  int pos =numOfRegisterPins -1;
  byte n1 = 0;
  byte n2 = 0;



  //push p2 score;
  if (P2score <10)
  {
    n1 = numbers[0];
    n2 = numbers[P2score];
  }
  else
  {
    n2 = numbers[ P2score % 10];
    n1 = numbers[ (P2score - (P2score %10)) /10];
  }

  
  for (int i=0; i<8; i++) // push N2
  {
    boolean val = (n2&(0x01<<i))>>i;
    registers[pos] = val;
    pos--;

  }
  for (int i=0; i<8; i++) // push N1
  {
    boolean val = (n1&(0x01<<i))>>i;
    registers[pos] = val;
    pos--;
  }

 // Serial.print("P2Score:");
 // Serial.println(P2score);





  //push seconds score;
  if (perSLeft <10)
  {
    n1= numbers[0];
    n2= numbers[perSLeft];
  }
  else
  {
    n2 = numbers[perSLeft % 10];
    n1 = numbers[(perSLeft - (perSLeft %10)) /10];
  }
  //n2
  for (int i=0; i<8; i++)
  {
    boolean val = (n2&(0x01<<i))>>i;
    registers[pos] = val;
    pos--;
  }
  //n1
  for (int i=0; i<8; i++)
  {
    boolean val = (n1&(0x01<<i))>>i;
    registers[pos] = val;
    pos--;
  }
//  Serial.print("Seconds:");
 // Serial.println(perSLeft);




  //push minutes score;
  if (perMLeft <10)
  {
    n1=numbers[0];
    n2=numbers[perMLeft];
  }
  else
  {
    n2 = numbers[perMLeft % 10];
    n1 = numbers[(perMLeft -  (perMLeft %10)) /10];
  }
  //n2
  for (int i=0; i<8; i++)
  {
    boolean val = (n2&(0x01<<i))>>i;
    registers[pos] = val;
    pos--;
  }
  //n1
  for (int i=0; i<8; i++)
  {
    boolean val = (n1&(0x01<<i))>>i;
    registers[pos] = val;
    pos--;
  }
//  Serial.print("Minutes:");
//  Serial.println(perMLeft);






  //push p1 score;
  if (P1score <10)
  {
    n1=numbers[0];
    n2=numbers[P1score];
  }
  else
  {
    n2 =numbers[P1score % 10];
    n1 =numbers[(P1score - (P1score %10)) /10];
  }
  //n2
  for (int i=0; i<8; i++)
  {
    boolean val = (n2&(0x01<<i))>>i;
    registers[pos] = val;
    pos--;
  }
  //n1
  for (int i=0; i<8; i++)
  {
    boolean val = (n1&(0x01<<i))>>i;
    registers[pos] = val;
    pos--;
  }
//  Serial.print("P1Score:");
//  Serial.println(P1score);


  writeRegisters();

}







