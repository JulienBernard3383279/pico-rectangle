#ifndef __UI
#define __UI

namespace UI {
    class State {
        public:
            const char * oledMessage;
            bool ledMode;
    };

    enum class UIOutput : int {
        LED = 0,
        SSD1306 = 1
    };

    void updateState(State *state);

    void initUI();
}

#endif