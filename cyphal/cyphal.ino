#include <memory>

#include <VBCoreG4_arduino_system.h>   
#include <stm32g4xx_hal_fdcan.h>       
#include <Libcanard2.h>                
#include <uavcan/primitive/String_1_0.h>

#include <cyphal.h>

TYPE_ALIAS(cyph_str, uavcan_node_String_1_0)

CanFD canfd;
FDCAN_HandleTypeDef* hfdcan1;

HardwareTimer *timer = new HardwareTimer(TIM3);

std::shared_ptr<CyphalInterface> interface;

// Объявим функцию, которую libcyphal будем вызывать для обработки ошибок
void error_handler() {Serial.println("error"); while (1) {};}
uint64_t micros_64() {return micros();}
UtilityConfig utilities(micros_64, error_handler);

class str_subs: public AbstractSubscription<cyph_str> {
public:
    str_subs(InterfacePtr interface): AbstractSubscription<cyph_str>(interface,
        uavcan_node_String_1_0_FIXED_PORT_ID_
    ) {};
    void handler(const uavcan_node_String_1_0& data, CanardRxTransfer* transfer) override {
        Serial.print(+transfer->metadata.remote_node_id);
        Serial.print(": ");
        Serial.println(data);
    }
};

HBeatReader* reader;

void setup() {
    canfd.can_init();
    hfdcan1 = canfd.get_hfdcan();

    // memory location, node_id, fdcan handler, messages memory pool, utils ref
    interface = CyphalInterface::create_heap<G4CAN, O1Allocator>(97, hfdcan1, 200, utilities);

    // Создаем обработчик сообщений, который объявили выше
    reader = new str_subs(interface);

    // Настраиваем таймер на прерывания раз в секунду
    timer->pause();
    timer->setPrescaleFactor(7999);
    timer->setOverflow(19999);
    timer->attachInterrupt(hbeat_func);
    timer->refresh();
    timer->resume();
}

void loop() {
    interface->loop();
}

const CanardPortID SD_PORT = 176;
void send_diagnostic(char* string) { 
    static uint8_t sd_buf[CyphalString::buffer_size];
    static CanardTransferID sd_transfer_id = 0;
    uavcan_primitive_String_1_0 sd = {};
    sprintf((char*)sd.value.elements, "%s", string);
    sd.value.count = strlen((char*)sd.value.elements);

    interface->send_msg<CyphalString>(&sd, sd_buf, SD_PORT, &sd_transfer_id);
}

// Функция таймера
void hbeat_func() {
    digitalWrite(LED2, !digitalRead(LED2));
    send_diagnostic("hello");
    uptime += 1;
}