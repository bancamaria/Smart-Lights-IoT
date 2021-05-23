//
// Created by Catalin on 3/13/2021.
//

#include "SmartLamp.h"

namespace smartlamp {

    void to_json(json &j, const ParametrizedAction &p) {
        j = json{{"actionType",  p.actionType},
                 {"actionParam", p.actionParam}};
    }

    void from_json(const json &j, ParametrizedAction &p) {
        j.at("actionType").get_to(p.actionType);
        j.at("actionParam").get_to(p.actionParam);
    }

    const std::string EMPTY_PARAM = "";

    namespace light {

        void to_json(json &j, const BulbState &p) {
            j = json{{"intensity",    p.intensity},
                     {"color",        p.color},
                     {"colorPattern", p.colorPattern},
                     {"status",       p.isOn},
                     {"presence",     p.presence}};
        }

        void from_json(const json &j, BulbState &p) {
            j.at("intensity").get_to(p.intensity);
            j.at("colorPattern").get_to(p.colorPattern);
            j.at("color").get_to(p.color);
            j.at("status").get_to(p.isOn);
            j.at("presence").get_to(p.presence);
        }

        const std::string NONE_COLOR_PATTERN = "NONE";
        const std::string DEFAULT_COLOR = "WHITE";
        const int DEFAULT_INTENSITY = 5;
        const int DEFAULT_PRESENCE = 0;
        const int DEFAULT_ISON = 0;
        const int DEFAULT_BRIGHTNESS = 0;
    }

    namespace buzzer {
        void to_json(json &j, const buzzer::BuzzerState &p) {
            j = json{{"status",      p.status},
                     {"snooze_time", p.snooze_time}};
        }

        void from_json(const json &j, buzzer::BuzzerState &p) {
            j.at("status").get_to(p.status);
            j.at("snooze_time").get_to(p.snooze_time);
        }

        bool is_morning() {
            time_t rawtime = time(0);
            struct tm *timeinfo;
            char buffer[80];
            timeinfo = localtime(&rawtime);
            strftime(buffer, sizeof(buffer), "%H:%M", timeinfo);
            int current_hour = (buffer[0] - '0') * 10 + (buffer[1] = '0');

            if (current_hour >= 6 && current_hour <= 7) {
                return true;
            }
            return false;
        };
    }

};

using namespace smartlamp;


bool SmartLamp::addSoundPattern(const string &regexPattern, const string &action) {
    ParametrizedAction parametrizedAction;
    parametrizedAction.actionType = possibleActions[action];
    parametrizedAction.actionParam = EMPTY_PARAM;
    auto result = soundPatternsMapping.insert(std::make_pair(regexPattern, parametrizedAction));
    return result.second;
}

int SmartLamp::getMicSensitivity() const {
    return micSensitivity;
}

void SmartLamp::setMicSensitivity(const int &sensitivity) {
    micSensitivity = sensitivity;
}

bool SmartLamp::hasMapping(const string &mapping) {
    return possibleActions.find(mapping) != possibleActions.end();
}

/*
 * To be used whenever there is a newPattern mapping to CHANGE_COLOR or START_LIGHT_PATTERN
 * In the CHANGE_COLOR case, the optionValue parameter is the color's name
 * In the START_LIGHT_PATTERN case, the optionValue parameter is the light pattern */
bool SmartLamp::addSoundPattern(const string &regexPattern, const string &action, const string &optionValue) {
    ParametrizedAction actualAction;
    actualAction.actionType = possibleActions[action];
    actualAction.actionParam = optionValue;
    return soundPatternsMapping.insert(std::make_pair(regexPattern, actualAction)).second;
}

unordered_map<std::string, smartlamp::ParametrizedAction> SmartLamp::getSoundPatterns() {
    return soundPatternsMapping;
}

pair<light::BulbState, buzzer::BuzzerState> SmartLamp::onSoundRecorded(const string &soundPattern) {
    auto it = soundPatternsMapping.find(soundPattern);
    /*
     * If the soundPattern is not known, we must ignore it, since it is just 'noise' so no new action should be taken
     * */
    if (it == soundPatternsMapping.end())
        return make_pair(currentBulbState, currentBuzzerState);

    switch (it->second.actionType) {
        case ACTION::TURN_ON_LIGHT: {
            currentBulbState.isOn = 1;
            currentBulbState.color = light::DEFAULT_COLOR;
            currentBulbState.intensity = 5;
            currentBulbState.colorPattern = light::NONE_COLOR_PATTERN;
            break;
        }
        case ACTION::TURN_OFF_LIGHT: {
            currentBulbState.isOn = 0;
            break;
        }
        case ACTION::CHANGE_COLOR: {
            currentBulbState.isOn = 1;
            currentBulbState.colorPattern = light::NONE_COLOR_PATTERN;
            currentBulbState.color = it->second.actionParam;
            break;
        }

        case ACTION::START_COLOR_PATTERN: {
            currentBulbState.isOn = 1;
            currentBulbState.color = light::DEFAULT_COLOR;
            currentBulbState.colorPattern = it->second.actionParam;
            break;
        }

        case ACTION::TURN_ON_BUZZER: {
            currentBuzzerState.status = true;
            // the moment when the buzzer will snooze is the previous set timer (I just turned on the buzzer)
            break;
        }

        case ACTION::TURN_OFF_BUZZER: {
            currentBuzzerState.status = false;
            // the moment when the buzzer will snooze will remain the same (I just turned of the buzzer - I might turn it on again)
            break;
        }

    }
    return make_pair(currentBulbState, currentBuzzerState); // what we should do here ?
}

void SmartLamp::setBuzzerStatus(const int &status) {
    buzzerState.status = status;
}

void SmartLamp::setBuzzerSnoozeTime(const time_t &snooze_time) {
    buzzerState.snooze_time = snooze_time;
}


time_t SmartLamp::getBuzzerSnoozeTime() const {
    return buzzerState.snooze_time;
}


int SmartLamp::getBuzzerStatus() const {
    return buzzerState.status;
}

smartlamp::light::BulbState SmartLamp::getBulbState() {
    return currentBulbState;
}

void SmartLamp::setBulbColor(string color) {
    currentBulbState.color = color;
}

string SmartLamp::getBulbColor() const {
    return currentBulbState.color;
}

void SmartLamp::setBulbIntensity(int intensity) {
    currentBulbState.intensity = intensity;
}

int SmartLamp::getBulbIntensity() const {
    return currentBulbState.intensity;
}

int SmartLamp::getOnOffState() {
    return currentBulbState.isOn;
}

void SmartLamp::setOnOffState(bool isOn) {
    currentBulbState.isOn = isOn;
}

int SmartLamp::getPresence() {
    return currentBulbState.presence;
}

void SmartLamp::setPresence(bool presence) {
    currentBulbState.presence = presence;
}

string SmartLamp::getColorPattern() {
    return currentBulbState.colorPattern;
}

void SmartLamp::setColorPattern(string colorPattern) {
    currentBulbState.colorPattern = colorPattern;
}

int SmartLamp::getBrightness() {
    return currentBulbState.brightness;
}

void SmartLamp::setBulbBrightness(int brightness) {
    currentBulbState.brightness = brightness;
}


void SmartLamp::onBrightnessRecorded(const int &recordedBrightness, bool detectPresence) {
    auto it = brightPatternsMapping.find(std::make_pair(recordedBrightness, detectPresence));

    // hasn't met this specific pair
    if (it == brightPatternsMapping.end()) {

        // if there is no one present (is asleep)
        if (it->first == make_pair(10, false))
            if (smartlamp::buzzer::is_morning()) { // turn on the buzzer
                setBuzzerStatus(1);
                setBuzzerSnoozeTime(time(0));
            }

        // if there is morning
        if (it->first == make_pair(20, true)) {
            brightPatternsMapping.insert({make_pair(20, true), "BLUE"});
            // setBulbBrightness(20);
            // setBulbColor("BLUE");
            brightColorMapping.insert({make_pair("BLUE", 90)});
            currentBulbState.intensity = 90;
        }
            else if (it->first == make_pair(70, true)) {
                brightPatternsMapping.insert({make_pair(70, true), "YELLOW"});
                // setBulbBrightness(70);
                // setBulbColor("YELLOW");
                brightColorMapping.insert({make_pair("YELLOW", 70)});
                currentBulbState.intensity = 70;
            }
                else if (it->first == make_pair(40, true)) {
                    brightPatternsMapping.insert({make_pair(40, true), "RED"});
                    // setBulbBrightness(40);
                    // setBulbColor("RED");
                    brightColorMapping.insert({make_pair("RED", 30)});
                    currentBulbState.intensity = 30;
                }
                    else if (it->first == make_pair(3, true)) {
                        brightPatternsMapping.insert({make_pair(3, true), "GREEN"});
                        // setBulbBrightness(30);
                        // setBulbColor("GREEN");
                        brightColorMapping.insert({make_pair("GREEN", 100)});
                        currentBulbState.intensity = 100;
                    }
                        else currentBulbState.color = "WHITE";

    }
    // this type of pair has been encountered
    else {
        // takes second value (the color)
        currentBulbState.color = it->second;
        auto it2 = brightColorMapping.find(it->second);
        currentBulbState.intensity = it2->second;
    }

}

void SmartLamp::insertNewPair(const int &recordedBrightness, int detectPresence, string detectedColor) {
    brightPatternsMapping.insert(std::make_pair(std::make_pair(recordedBrightness, detectPresence), detectedColor));
}
