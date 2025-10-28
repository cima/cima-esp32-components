#include <system/ButtonController.h>

namespace cima::system {
    
    const Log ButtonController::LOGGER("ButtonController");

    ButtonController::ButtonController(gpio_num_t buttonGpioPin) : buttonGpioPin(buttonGpioPin) {}

    void ButtonController::initButton(){
        gpio_config_t io_conf;
        io_conf.intr_type = GPIO_INTR_ANYEDGE;
        io_conf.pin_bit_mask = 1ULL << buttonGpioPin;    
        io_conf.mode = GPIO_MODE_INPUT;
        io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;

        //TODO why the hell both resistor were enabled?
        //io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;

        esp_err_t gpioError = gpio_config(&io_conf);
        LOGGER.debug("Po globál dobrý 0x%x", gpioError);
        
        esp_err_t handlerError = gpio_isr_handler_add(buttonGpioPin, &ButtonController::gpioButtonHandlerWrapper, this);
        if (handlerError != ESP_OK) {
            LOGGER.error("Rising edge interrup installation failed: 0x%x", handlerError);
        } else {
            LOGGER.debug("Rising edge Button handler installed.");
        }


    }

    void ButtonController::handleClicks(){
        std::lock_guard<std::mutex> guard(clicksMutex);
        if(clicks.empty()){
            return;
        }
        auto event = clicks.front();
        LOGGER.info("Handling %s button event from: %s", event.up ? "UP" : "DOWN", std::ctime(&event.time));

        if(event.up){
            buttonUpSignal();
        } else {
            buttonDownSignal();
        }
        clicks.pop();
    }

    //Deprecated
    void ButtonController::addHandler(std::function<void(void)> func){
        addUpHandler(func);
    }

    void ButtonController::addUpHandler(std::function<void(void)> func){
        buttonUpSignal.connect(func);
    }
    
    void ButtonController::addDownHandler(std::function<void(void)> func){
        buttonDownSignal.connect(func);
    }

    void ButtonController::addLongPressHandler(std::function<void(void)> func){
        longButtonSignal.connect(func);
    }

    bool ButtonController::isButtonUp(){
        return gpio_get_level(buttonGpioPin) != 0;
    }

    void ButtonController::gpioButtonDownHandler(){
        // ATTENTION: This is IRS call. No monkey bussiness. Just forward over some parralel primitives.
        auto nowChrono = std::chrono::system_clock::now();
        std::time_t nowTime = std::chrono::system_clock::to_time_t(nowChrono);
    
        std::lock_guard<std::mutex> guard(clicksMutex);
        clicks.push({nowTime, false});
    }

    void ButtonController::gpioButtonUpHandler(){
        // ATTENTION: This is IRS call. No monkey bussiness. Just forward over some parralel primitives.
        auto nowChrono = std::chrono::system_clock::now();
        std::time_t nowTime = std::chrono::system_clock::to_time_t(nowChrono);
    
        std::lock_guard<std::mutex> guard(clicksMutex);
        clicks.push({nowTime, true});
    }

    void ButtonController::gpioButtonHandlerWrapper(void *arg){
        // ATTENTION: This is IRS call. No monkey bussiness. Just forward over some parralel primitives.
        if (gpio_get_level(((ButtonController *)arg)->buttonGpioPin)) { //pin is HIGH
            ((ButtonController *)arg)->gpioButtonUpHandler();
        } else {
            ((ButtonController *)arg)->gpioButtonDownHandler();
        }
    }
}