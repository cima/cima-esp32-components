#pragma once

#include <system/Log.h>

namespace cima::system {

    class InterruptController {
        static const system::Log LOGGER;

        bool interruptsEnabled = false;

    public:
        InterruptController() = default;
        ~InterruptController() = default;

        bool enableInterrupts();
        void disableInterrupts();
    };
}