#include <VBCoreG4_arduino_system.h>

uint8_t data[4] = { 90, 44, 42, 43};
unsigned long t;
FDCAN_HandleTypeDef* hfdcan1;
CanFD* canfd;
FDCAN_TxHeaderTypeDef TxHeader;

void setup() {
    Serial.begin(115200);
    SystemClock_Config();
    canfd = new CanFD();
    canfd->init();
    canfd->write_default_params();
    canfd->apply_config();
    hfdcan1 = canfd->get_hfdcan();
    canfd->default_start();
    TxHeader.Identifier = 0x12;
    TxHeader.DataLength = FDCAN_DLC_BYTES_4;
    TxHeader.IdType = FDCAN_EXTENDED_ID;
}
void loop() {
    if (HAL_FDCAN_GetTxFifoFreeLevel(hfdcan1) != 0){
        if (HAL_FDCAN_AddMessageToTxFifoQ(hfdcan1, &TxHeader, data) != HAL_OK){ Error_Handler(); }
    }
    while(HAL_FDCAN_GetRxFifoFillLevel(hfdcan1, FDCAN_RX_FIFO0) > 0 )
    {
    FDCAN_RxHeaderTypeDef Header;
    uint8_t RxData[4];
    if (HAL_FDCAN_GetRxMessage(hfdcan1, FDCAN_RX_FIFO0, &Header, RxData) != HAL_OK){ Error_Handler(); }
    else{
    Serial.println(RxData[0]);
    }
    }
    delay(100);
}