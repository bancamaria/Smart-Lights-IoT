//
// Created by Catalin on 3/13/2021.
//

#include "SmartLamp.h"

void SmartLamp::on_sound_record() {

}

std::unordered_map<std::string,ACTION> SmartLamp::getSoundPatterns() {
    return soundPatterns;
}

bool SmartLamp::addSoundPattern(const string &regexPattern, ACTION action) {
    auto result = soundPatterns.insert({regexPattern, action});
    return result.second;
}

bool SmartLamp::addSoundPattern(const string &regexPattern, const string &action) {
    ACTION a  = possibleActions[action];
    auto result = soundPatterns.insert({regexPattern, a});
    return result.second;
}


int SmartLamp::getMicSensitivity() {
    return micSensitivity;
}

void SmartLamp::setMicSensitivity(const int &sensitivity) {
    micSensitivity = sensitivity;
}

bool SmartLamp::hasMapping(const string &mapping) {
    return possibleActions.find(mapping) != possibleActions.end();
}






