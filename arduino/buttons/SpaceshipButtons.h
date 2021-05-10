#include "ButtonSet.h"

class SpaceshipButtons {

public:

    static const uint8_t AUDIO_MODE = 1;
    static const uint8_t AUDIO_CH_PLUS = 2;
    static const uint8_t AUDIO_CH_MINUS = 3;
    static const uint8_t AUDIO_VOL_PLUS = 4;
    static const uint8_t AUDIO_VOL_MINUS = 5;

    static const uint8_t HFT_TALK = 1;
    static const uint8_t HFT_BACK = 2;
    static const uint8_t HFT_HOOK_ON = 3;
    static const uint8_t HFT_HOOK_OFF = 4;

    /**
     * Audio remote control switch connected with 1K pull-up to VCC.
     */
    static ButtonSet audioSwitchMaster(uint8_t pin);

    /**
     * Audio remote control switch connected to stock audio system (pull-up to 3.3V).
     */
    static ButtonSet audioSwitchSlave(uint8_t pin);

    /**
     * Hands-free switch connected with 1K pull-up to VCC.
     */
    static ButtonSet hft(uint8_t pin);

};
