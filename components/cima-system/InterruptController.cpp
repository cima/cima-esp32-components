#include <system/InterruptController.h>
#include <driver/gpio.h>
#include <esp_event.h>

namespace cima::system {

const system::Log InterruptController::LOGGER("InterruptController");

bool InterruptController::enableInterrupts(){

    if(interruptsEnabled){
        return true;
    }

    int isrFlags = ESP_INTR_FLAG_LEVEL1 | ESP_INTR_FLAG_SHARED;
    esp_err_t isrError = gpio_install_isr_service(isrFlags);
    if (isrError == ESP_OK) {
        interruptsEnabled = true;
    } else {
        LOGGER.error("GPIO interrupt handling service installation failed: 0x%x", isrError);
        interruptsEnabled = false;
    }

    return interruptsEnabled;
}

void InterruptController::disableInterrupts(){
    if(interruptsEnabled){
        gpio_uninstall_isr_service();
        interruptsEnabled = false;
    }
}

void InterruptController::createDefaultEventLoop(){
    ESP_ERROR_CHECK(esp_event_loop_create_default());
}

}