#ifndef __UI
#define __UI

namespace UI {
    class State {
        public:
            State(const char *_oledMessage, bool _ledMode) {
                oledMessage = _oledMessage;
                ledMode = _ledMode;
            };
            const char * oledMessage;
            bool ledMode;
    };

    class States {
        public:
            UI::State meleeAdapter = UI::State("Melee Adapter", false);
            UI::State countdownThree = UI::State("Three", false);
            UI::State countdownTwo = UI::State("Two", true);
            UI::State countdownOne = UI::State("One", false);
            UI::State pleaseRestart = UI::State("Please restart", false);
    };

    enum class UIOutput : int {
        LED = 0,
        SSD1306 = 1
    };

    extern States states;

    void updateState(UI::State *state);

    void initUI();
}

#endif