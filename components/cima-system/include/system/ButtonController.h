#pragma once

#include <list>
#include <mutex>
#include <queue>
#include <chrono>
#include <functional>

#include <driver/gpio.h>

#include <boost/signals2/signal.hpp>

#include <system/Log.h>

namespace cima::system {

    struct ClickType {
        /* Wehn the event occurred */
        std::time_t time;

        /* Up means button was released and is up, 
           as well as the pin state is up. */
        bool up;
    };

    class ButtonController {
        
        static const Log LOGGER;

        const gpio_num_t buttonGpioPin;

        std::list<std::function<void(void)>> buttonHandlers;

        std::queue<ClickType> clicks;
        std::mutex clicksMutex;

        boost::signals2::signal<void ()> buttonUpSignal;
        boost::signals2::signal<void ()> buttonDownSignal;

        boost::signals2::signal<void ()> longButtonSignal;

    public:
        ButtonController(gpio_num_t buttonGpioPin);
        void initButton();
        void handleClicks();

        /* Add routine called for burtton release */
        void addHandler(std::function<void(void)> func);

        void addUpHandler(std::function<void(void)> func);
        void addDownHandler(std::function<void(void)> func);

        void addLongPressHandler(std::function<void(void)> func);

        /* returns true for button UP(HIGH) or false for button DOWN(LOW) */
        bool isButtonUp();

    private:
        //deprecated
        void gpioButtonHandler();

        /* 
            Handles a button press (down). I.e. 
            Transition from HIGH state to LOW 
        */
        void gpioButtonDownHandler();

        /* 
            Handles a button release (up). I.e. 
            Transition from LOW state to HIGH 
        */
        void gpioButtonUpHandler();

        static void gpioButtonHandlerWrapper(void *);
    };
}