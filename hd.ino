#include <VBCoreG4_arduino_system.h>
#include <NewPing.h>
#include <Servo.h>
Servo servo_1;

#define trig PC0
#define echo PC1
unsigned long t;
uint8_t data[4] = {1, 0, 0, 0};

FDCAN_HandleTypeDef*  hfdcan1; 
CanFD* canfd;
FDCAN_TxHeaderTypeDef TxHeader;

NewPing sonar(trig, echo, 255);

void setup() {
  /*system_settings*/
  Serial.begin(115200);
  SystemClock_Config();
  /*init canfd*/
  servo_1.attach(PB0);
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
  delay(2000);

}
uint8_t servo_n = 0;
void loop() {
  servo_1.write(servo_n);
  get_can();
  delay(50);
}


void send_can() {
  if (HAL_FDCAN_GetTxFifoFreeLevel(hfdcan1) != 0){
    if (HAL_FDCAN_AddMessageToTxFifoQ(hfdcan1, &TxHeader, data) != HAL_OK){Error_Handler();}
  }
}

void get_can(){
   while(HAL_FDCAN_GetRxFifoFillLevel(hfdcan1, FDCAN_RX_FIFO0) > 0 )
    {
      FDCAN_RxHeaderTypeDef Header; 
      uint8_t RxData[4];
      if (HAL_FDCAN_GetRxMessage(hfdcan1, FDCAN_RX_FIFO0, &Header, RxData) != HAL_OK){ Error_Handler(); }  
      else {
        uint8_t ident = Header.Identifier;
        Serial.println(ident);
        if (ident==66) {
          data[0] = sonar.ping_cm();
          delay(100);
          if (data[0]==0){
            data[0]=50;
          }
          send_can();
        } else if (ident==82){
          servo_n = RxData[0];
        }
      }
    }
}