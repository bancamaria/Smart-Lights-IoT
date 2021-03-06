//
// Created by Catalin on 3/13/2021.
//

#ifndef SMART_LIGHTS_IOT_PROJECT_SMARTLAMP_H
#define SMART_LIGHTS_IOT_PROJECT_SMARTLAMP_H

#include "pistache/http.h"
#include "nlohmann/json.hpp"

using namespace Pistache;
using namespace std;
using namespace nlohmann;

namespace smartlamp {

    enum ACTION {
        TURN_ON_LIGHT,
        TURN_OFF_LIGHT,
        CHANGE_COLOR,
        START_COLOR_PATTERN,
        TURN_ON_BUZZER,
        TURN_OFF_BUZZER,
        CHANGE_INTENSITY
    };

    enum COLORS {
        WHITE, YELLOW, RED, BLUE
    };

    struct ParametrizedAction {
        ACTION actionType;
        std::string actionParam;
    };

    void to_json(json &j, const ParametrizedAction &p);
    void from_json(const json &j, ParametrizedAction &p);

    extern const std::string EMPTY_PARAM;

    namespace light {
        extern const std::string NONE_COLOR_PATTERN;
        extern const std::string DEFAULT_COLOR;
        extern const int DEFAULT_INTENSITY;
        extern const int DEFAULT_ISON;
        extern const int DEFAULT_PRESENCE;
        extern const int DEFAULT_BRIGHTNESS;

        /*
         * This namespace describes the bulb
         * */
        struct BulbState {
            int intensity = DEFAULT_INTENSITY;
            std::string colorPattern = NONE_COLOR_PATTERN;
            std::string color = DEFAULT_COLOR;
            int isOn = DEFAULT_ISON;
            int presence = DEFAULT_PRESENCE;
            int brightness = DEFAULT_BRIGHTNESS;
        };

        void to_json(json &j, const BulbState &p);
        void from_json(const json &j, BulbState &p);
    }

    namespace buzzer {

        /*
         * This namespace describes the buzzer
         * */
        struct BuzzerState {
            bool status = false;
            time_t snooze_time = time(0); // current time
        };

        void to_json(json &j, const BuzzerState &p);
        void from_json(const json &j, BuzzerState &p);
    }

};


class SmartLamp {

public:

    explicit SmartLamp() {
        possibleActions.insert(std::make_pair("TURN_ON_LIGHT", smartlamp::ACTION::TURN_ON_LIGHT));
        possibleActions.insert(std::make_pair("TURN_OFF_LIGHT", smartlamp::ACTION::TURN_OFF_LIGHT));
        possibleActions.insert(std::make_pair("CHANGE_COLOR", smartlamp::ACTION::CHANGE_COLOR));
        possibleActions.insert(std::make_pair("START_COLOR_PATTERN", smartlamp::ACTION::START_COLOR_PATTERN));
        possibleActions.insert(std::make_pair("CHANGE_INTENSITY", smartlamp::ACTION::CHANGE_INTENSITY));

        possibleActions.insert(std::make_pair("TURN_ON_BUZZER", smartlamp::ACTION::TURN_ON_LIGHT));
        possibleActions.insert(std::make_pair("TURN_OFF_BUZZER", smartlamp::ACTION::TURN_OFF_LIGHT));

    };

    bool hasMapping(const std::string &mapping);

    void setMicSensitivity(const int &sensitivity);
    int getMicSensitivity() const;

    std::unordered_map<std::string, smartlamp::ParametrizedAction> getSoundPatterns();
    bool addSoundPattern(const std::string &regexPattern, const string &action);
    bool addSoundPattern(const std::string &regexPattern, const string &action, const string &optionValue);

    void setBuzzerStatus(const int &status);
    int getBuzzerStatus() const;
    void setBuzzerSnoozeTime(const time_t &snooze_time);
    time_t getBuzzerSnoozeTime() const;

    string getBulbColor() const;
    void setBulbColor(string color);
    int getBulbIntensity() const;
    void setBulbIntensity(int intensity);
    int getOnOffState();
    void setOnOffState(bool isOn);
    int getPresence();
    void setPresence(bool presence);
    string getColorPattern();
    void setColorPattern(string colorPattern);
    int getBrightness();

    smartlamp::light::BulbState getBulbState();

    pair<smartlamp::light::BulbState, smartlamp::buzzer::BuzzerState> onSoundRecorded(const std::string &soundPattern);
    void onBrightnessRecorded(const int &recordedBrightness, bool detectPresence);

private:
    /*Members that can adjust the microphone */
    /*
     * All the recorded sound patterns that, when detected, will result in a state change of the lamp
     * where the key is the pattern, e.g. '1000101011' and the value is the possible ACTION. */
    std::unordered_map<std::string, smartlamp::ParametrizedAction> soundPatternsMapping;

    std::unordered_map<std::string, smartlamp::ACTION> possibleActions;
    /*Change fields of this whenever working with the bulb
     * Contains :
     * - intensity
     * - color
     * - color_pattern (if set)
     * - isOn
     * */
    smartlamp::light::BulbState currentBulbState;
    smartlamp::buzzer::BuzzerState currentBuzzerState;

    int micSensitivity;
    smartlamp::buzzer::BuzzerState buzzerState;
    int lightIntensity;
    int lightValue;
};


#endif //SMART_LIGHTS_IOT_PROJECT_SMARTLAMP_H