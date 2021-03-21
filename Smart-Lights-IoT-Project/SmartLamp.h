//
// Created by Catalin on 3/13/2021.
//

#ifndef SMART_LIGHTS_IOT_PROJECT_SMARTLAMP_H
#define SMART_LIGHTS_IOT_PROJECT_SMARTLAMP_H

#include "pistache/http.h"

using namespace Pistache;
using namespace std;

enum class ACTION{
    TURN_ON_LIGHT,
    TURN_OFF_LIGHT,
    CHANGE_COLOR,
    START_COLOR_PATTERN
};
enum class MIC_CONFIG{
    SENSITIVITY,
    PATTERNS
};
typedef std::pair<std::string,int> color_pattern_member;

class SmartLamp {

public:
    explicit SmartLamp(){
        possibleActions.insert(std::make_pair("TURN_ON_LIGHT",ACTION::TURN_ON_LIGHT));
        possibleActions.insert(std::make_pair("TURN_OFF_LIGHT",ACTION::TURN_OFF_LIGHT));
        possibleActions.insert(std::make_pair("CHANGE_COLOR",ACTION::CHANGE_COLOR));
        possibleActions.insert(std::make_pair("START_COLOR_PATTERN",ACTION::START_COLOR_PATTERN));

    };

    bool hasMapping(const std::string &mapping);
    void setMicSensitivity(const int &sensititvity);
    int getMicSensitivity();

    std::unordered_map<std::string,ACTION> getSoundPatterns();
    bool addSoundPattern(const std::string &regexPattern, ACTION action);
    bool addSoundPattern(const std::string &regexPattern, const string& action);

    void on_sound_record();

private:
    /*Members that can adjust the microphone */
    std::unordered_map<std::string,ACTION> soundPatterns;
    std::unordered_map<std::string,std::vector<color_pattern_member>> colorPatterns;
    std::unordered_map<std::string,ACTION> possibleActions;
    int micSensitivity;

};


#endif //SMART_LIGHTS_IOT_PROJECT_SMARTLAMP_H
