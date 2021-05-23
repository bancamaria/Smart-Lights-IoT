//
// Created by Catalin on 3/13/2021.
//

#include "SmartLightController.h"

// Get the Controller started on the given endpoint
void SmartLightController::init(size_t thr) {
    auto opts = Http::Endpoint::options()
            .threads(static_cast<int>(thr));
    httpEndpoint->init(opts);
    // Server routes are loaded up
    setupRoutes();
}

void SmartLightController::start() {
    httpEndpoint->setHandler(router.handler());
    httpEndpoint->serveThreaded();
}

void SmartLightController::stop() {
    httpEndpoint->shutdown();
}

void SmartLightController::setupRoutes() {
    using namespace Rest;

    /*
     * Rares's note: we will not need and endpoint to publish the actual JSON specifications.
     * */
    Routes::Get(router, "/home", Routes::bind(&SmartLightController::doAuth, this));

    /*Here we will post messages that will simulate the sound "recorded" by the smart lamp*/
    Routes::Get(router, "/microphone/settings", Routes::bind(&SmartLightController::getMicrophoneSettings, this));
    Routes::Post(router, "/microphone/settings", Routes::bind(&SmartLightController::setMicrophoneSettings, this));
    Routes::Get(router, "/microphone/patterns", Routes::bind(&SmartLightController::getRegisteredPatterns, this));
    Routes::Post(router, "/microphone/patterns", Routes::bind(&SmartLightController::registerPattern, this));
    Routes::Get(router, "/microphone", Routes::bind(&SmartLightController::onSoundRecorded, this));

    Routes::Get(router, "/buzzer/settings", Routes::bind(&SmartLightController::getBuzzerSettings, this));
    Routes::Post(router, "/buzzer/settings", Routes::bind(&SmartLightController::setBuzzerSettings, this));

    Routes::Get(router, "/bulb/settings", Routes::bind(&SmartLightController::getBulbSettings, this));
    Routes::Post(router, "/bulb/settings", Routes::bind(&SmartLightController::setBulbSettings, this));

    Routes::Post(router, "/photorezistor", Routes::bind(&SmartLightController::onBrightnessRecorded, this));
}

/*
void SmartLightController::(){
}*/
void SmartLightController::doAuth(const Rest::Request &request, Http::ResponseWriter response) {
//    RARESITO' example:
//     Function that prints cookies
//    printCookies(request);

    // In the response object, it adds a cookie regarding the communications language.
    response.cookies()
            .add(Http::Cookie("lang", "en-US"));
    // Send the response
    response.send(Http::Code::Ok, "The Light Controller Works.");
}

void SmartLightController::getMicrophoneSettings(const Rest::Request &request, Http::ResponseWriter response) {
    json result;
    result["sensitivity"] = smartLamp.getMicSensitivity();
    response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
    response.send(Http::Code::Ok, result.dump(3));
}

void SmartLightController::setMicrophoneSettings(const Rest::Request &request, Http::ResponseWriter response) {
    /*
     * Parse the parameters from the url
     * */

    if (request.query().has("sensitivity")) {
        optional<string> requestedValue = request.query().get("sensitivity");
        if (requestedValue->empty()) {
            requestedValue.value() = "0";
        }
        int val = std::stoi(requestedValue.value());
        smartLamp.setMicSensitivity(val);
    }
}

void SmartLightController::getBuzzerSettings(const Rest::Request &request, Http::ResponseWriter response) {
    json result;
    result["status"] = smartLamp.getBuzzerStatus();


    time_t rawtime = smartLamp.getBuzzerSnoozeTime();
    struct tm *timeinfo;
    char buffer[80];
    timeinfo = localtime(&rawtime);
    strftime(buffer, sizeof(buffer), "%H:%M", timeinfo);
    std::string str(buffer);
    result["snooze_timer"] = str;

    response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
    response.send(Http::Code::Ok, result.dump(3));
}

void SmartLightController::setBuzzerSettings(const Rest::Request &request, Http::ResponseWriter response) {
    if (request.query().has("status")) {
        int val = std::stoi(request.query().get("status").value());
        smartLamp.setBuzzerStatus(val);
    }
    if (request.query().has("snooze_timer")) {
        optional<string> val = request.query().get("snooze_timer");
        string value = val.value();
        value.replace(0, 3, "");
        value.replace(value.length() - 3, 3, "");

        const char *time_details = value.c_str();
        struct tm tm{};
        strptime(time_details, "%H:%M", &tm);
        time_t t = mktime(&tm);

        smartLamp.setBuzzerSnoozeTime(t);
    }
}

void SmartLightController::getBulbSettings(const Rest::Request &request, Http::ResponseWriter response) {
    json result;
    result["brightness"] = smartLamp.getBrightness();
    result["color"] = smartLamp.getBulbColor();
    result["presence"] = smartLamp.getPresence();
    result["isOn"] = smartLamp.getOnOffState();
    result["intensity"] = smartLamp.getBulbIntensity();

    response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
    response.send(Http::Code::Ok, result.dump(3));
}

void SmartLightController::setBulbSettings(const Rest::Request &request, Http::ResponseWriter response) {

   if (request.query().has("presence")) {
       optional<string> requestedValue = request.query().get("presence");
       if (requestedValue->empty()) {
           requestedValue.value() = "0";
       }
       int val = std::stoi(requestedValue.value());
       smartLamp.setPresence(val);

       // check if the bulb is already on
       if (smartLamp.getPresence() == 1)
           smartLamp.setOnOffState(1);
   }

    auto isValid = isValidRequestParam("brightness", request, response);
    if (!isValid.first)
        return;

    auto isValid2 = isValidRequestParam("presence", request, response);
    if(!isValid2.first)
        return;

    auto isValid3 = isValidRequestParam("color", request, response);
    if(!isValid3.first)
        return;

    int recordedBrightness = std::stoi(isValid.second);
    int detectPresence = std::stoi(isValid2.second);
    string detectedColor = std::string(isValid3.second);
    smartLamp.insertNewPair(recordedBrightness, detectPresence, detectedColor);
}

void SmartLightController::onBrightnessRecorded(const Rest::Request &request, Http::ResponseWriter response) {
    auto isValid = isValidRequestParam("brightness", request, response);
    if (!isValid.first)
        return;

    auto isValid2 = isValidRequestParam("presence", request, response);
    if(!isValid2.first)
        return;

    int recordedBrightness = std::stoi(isValid.second);
    int detectPresence = std::stoi(isValid2.second);
    smartLamp.onBrightnessRecorded(recordedBrightness, detectPresence);
}

void SmartLightController::registerPattern(const Rest::Request &request, Http::ResponseWriter response) {

    string bad_request_message = "In order to register a new pattern please provide the pattern and the action that maps it";
    auto isValid = isValidRequestParam("newPattern", request, response);
    if (!isValid.first)
        return;
    string newPattern = isValid.second;

    isValid = isValidRequestParam("mapsTo", request, response);
    if (!isValid.first)
        return;
    string soundMapping = isValid.second;
    if (!smartLamp.hasMapping(soundMapping)) {
        response.send(Http::Code::Bad_Request, "Invalid possible action.");
        return;
    }
    /*Up to this point, the endpoint is  valid /patterns/newPatterns=X&mapsTo=Z.
    * Will need to check further options depending on action specified in mapsTo */
    if (soundMapping == "TURN_ON_LIGHT" || soundMapping == "TURN_OFF_LIGHT") {
        auto succ = smartLamp.addSoundPattern(newPattern, soundMapping);
        if (succ) {
            json sendBack;
            sendBack["patterns"] = smartLamp.getSoundPatterns();
            response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
            response.send(Http::Code::Ok, sendBack.dump());
            return;
        }
    }

    if (soundMapping == "TURN_ON_BUZZER" || soundMapping == "TURN_OFF_BUZZER") {
        auto succ = smartLamp.addSoundPattern(newPattern, soundMapping);
        if (succ) {
            json sendBack;
            sendBack["patterns"] = smartLamp.getSoundPatterns();
            response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
            response.send(Http::Code::Ok, sendBack.dump());
            return;
        }
    }

    if (soundMapping == "CHANGE_COLOR") {
        /*Check if the requestParam 'color' is present*/
        auto isValid = isValidRequestParam("color", request, response);
        if (isValid.first) {
            string color = isValid.second;
            auto succss = smartLamp.addSoundPattern(newPattern, soundMapping, color);
            if (succss) {
                json sendBack;
                sendBack["patterns"] = smartLamp.getSoundPatterns();
                response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
                response.send(Http::Code::Ok, sendBack.dump());
                return;
            }
        }
    }

    if (soundMapping == "START_COLOR_PATTERN") {
        /*
        * Will need to check further for validity of 'colorPattern'
        *
        * */
        auto isValid = isValidRequestParam("colorPattern", request, response);
        if (isValid.first) {
            string colorPattern = isValid.second;
            auto succss = smartLamp.addSoundPattern(newPattern, soundMapping, colorPattern);
            if (succss) {
                json sendBack;
                sendBack["patterns"] = smartLamp.getSoundPatterns();
                response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
                response.send(Http::Code::Ok, sendBack.dump());
                return;
            }
        }
    }

    /*If none of the above is true, then it must be a 500.*/
    response.send(Http::Code::Internal_Server_Error, "Could not map " + newPattern + " to " + soundMapping);
}


void SmartLightController::getRegisteredPatterns(const Rest::Request &request, Http::ResponseWriter response) {
    auto patterns = smartLamp.getSoundPatterns();
    json sendBack;
//    sendBack["patterns"] = patterns;

    response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
    response.send(Http::Code::Ok, sendBack.dump(3));
}

void SmartLightController::onSoundRecorded(const Rest::Request &request, Http::ResponseWriter response) {
    auto isValid = isValidRequestParam("record", request, response);
    if (!isValid.first)
        return;
    string recordedSound = isValid.second;

    pair<smartlamp::light::BulbState, smartlamp::buzzer::BuzzerState> lampState = smartLamp.onSoundRecorded(
            recordedSound);

    smartlamp::light::BulbState lightState = lampState.first;
    smartlamp::buzzer::BuzzerState buzzerState = lampState.second;

    json j;
    j["lightState"] = lightState;
    j["buzzerState"] = buzzerState;
    response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
    response.send(Http::Code::Ok, j.dump());
}

std::pair<bool, std::string>
SmartLightController::isValidRequestParam(const std::string &paramName, const Rest::Request &request,
                                          Http::ResponseWriter &response) {

    if (!request.query().has(paramName)) {
        response.send(Http::Code::Bad_Request, "Missing " + paramName + " request parameter.");
        return {false, nullptr};
    }
    optional<string> paramValue = request.query().get(paramName);
    if (paramValue->empty()) {
        response.send(Http::Code::Bad_Request, "Missing " + paramName + " request parameter value.");
        return {false, nullptr};
    }
    return {true, *paramValue};
}