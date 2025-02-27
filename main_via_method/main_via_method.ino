#include <VBCoreG4_arduino_system.h>
#include <Servo.h>
#include <NewPing.h>

Servo servo_1;
Servo servo_2;
Servo servo_3;
Servo servo_4;
Servo servo_5;
Servo servo_6;

#define trig PC0
#define echo PC1
#define servo_1_pin PB0 
#define servo_2_pin PA7 
#define servo_3_pin PA6 
#define servo_4_pin PA4 
#define servo_5_pin PC3 
#define servo_6_pin PC2 


uint8_t data[4] = {1, 0, 0, 0};
unsigned long t;

FDCAN_HandleTypeDef*  hfdcan1; 
CanFD* canfd;
FDCAN_TxHeaderTypeDef TxHeader;

NewPing sonar(trig, echo, 255);

void setup() {
  Serial.begin(115200);

  /*system_settings*/
  SystemClock_Config();

  /*init pins*/
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  servo_1.attach(servo_1_pin);
  servo_2.attach(servo_2_pin);
  servo_3.attach(servo_3_pin);
  servo_4.attach(servo_4_pin);
  servo_5.attach(servo_5_pin);
  servo_6.attach(servo_6_pin);


  /*init canfd*/
  canfd = new CanFD();  
  canfd->init();
  canfd->write_default_params();  
  canfd->apply_config();  
  hfdcan1 = canfd->get_hfdcan();
  canfd->default_start();

  /*header the sender*/
  TxHeader.Identifier = 0x12;
  TxHeader.DataLength = FDCAN_DLC_BYTES_4;
  TxHeader.IdType = FDCAN_EXTENDED_ID;

}

uint8_t servo_n[4] = {0,0,0,0}; //changing

int left = 1000;
int stop = 1500;
int right = 2000;

bool flags[4] = {false, false, false, false}; //5_left, 5_right, 6_left, 6_right

void loop() {
  servo_1.write(servo_n[0]);
  servo_2.write(servo_n[1]);
  servo_3.write(servo_n[2]);
  servo_4.write(servo_n[3]);
  if (flags[0]){servo_5.writeMicroseconds(left);}
  else if (flags[1]){servo_5.writeMicroseconds(right);}
  else {servo_5.writeMicroseconds(stop);}
  if (flags[2]){servo_6.writeMicroseconds(left);}
  else if (flags[3]){servo_6.writeMicroseconds(right);}
  else {servo_6.writeMicroseconds(stop);}
  get_can();
  delay(20);
}


void get_can(){
  while(HAL_FDCAN_GetRxFifoFillLevel(hfdcan1, FDCAN_RX_FIFO0) > 0 ) {
    FDCAN_RxHeaderTypeDef Header; 
    uint8_t RxData[1]; 
    if (HAL_FDCAN_GetRxMessage(hfdcan1, FDCAN_RX_FIFO0, &Header, RxData) != HAL_OK){ Error_Handler(); }  
    else {
      uint8_t ident = Header.Identifier;
      if (ident == 0x52 || ident==0x68 || ident==0x60 || ident==0x20 || 0x42){
        switch(ident){
          case 0x52:
            servo_n[0]=RxData[0];
            break;
          case 0x68:
            servo_n[1]=RxData[0];
            break;
          case 0x60:
            servo_n[2]=RxData[0];
            break;
          case 0x20:
            servo_n[3]=RxData[0];
            break;
          case 0x42:
            data[0] = sonar.ping_cm();
            if (data[0]==0){
              data[0]=50;
            }
            delay(50);
            send_can();
            break;
        }
      } else if (ident==0x63 || ident==0x34){
        if (ident==0x63){
          if (RxData[0]==1){
            flags[0]=true;
          } else if (RxData[0]==2){
            flags[1]=true;
          } else {
            flags[0]=false;
            flags[1]=false;
          }
        } else {
          if (RxData[0]==1){
            flags[2]=true;
          } else if (RxData[0]==2){
            flags[3]=true;
          } else {
            flags[2]=false;
            flags[3]=false;
          }
        }
      } 
    }
  }
}

void send_can(){
  if (HAL_FDCAN_GetTxFifoFreeLevel(hfdcan1) != 0){
    if (HAL_FDCAN_AddMessageToTxFifoQ(hfdcan1, &TxHeader, data) != HAL_OK){Error_Handler();}
  }
}