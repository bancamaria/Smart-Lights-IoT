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
    Routes::Post(router, "/microphone/patterns", Routes::bind(&SmartLightController::registerSoundPattern, this));
    /*Input Buffer: Microphone*/
    Routes::Post(router, "/microphone", Routes::bind(&SmartLightController::onSoundRecorded, this));

    Routes::Get(router, "/buzzer/settings", Routes::bind(&SmartLightController::getBuzzerSettings, this));
    Routes::Post(router, "/buzzer/settings", Routes::bind(&SmartLightController::setBuzzerSettings, this));

    Routes::Get(router, "/bulb", Routes::bind(&SmartLightController::getBulbState, this));
    /*Photoresistor Settings*/
    Routes::Post(router, "/photoresistor/patterns", Routes::bind(&SmartLightController::registerBrightnessPattern, this));
    /*Input Buffer: Photoresistor*/
    Routes::Post(router, "/photoresistor", Routes::bind(&SmartLightController::onBrightnessRecorded, this));
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
        Pistache::Optional<string> requestedValue = request.query().get("sensitivity");
        if (requestedValue.isEmpty()) {
            smartLamp.setMicSensitivity(0);
            return;
        }
        int val = std::stoi(requestedValue.get());
        smartLamp.setMicSensitivity(val);
        response.send(Http::Code::Ok);
    }
    response.send(Http::Code::Bad_Request, "No setting to update sent.");
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
    auto validStatus = isValidRequestParam("status", request, response);
    if(!validStatus.first)
        return;
    int statusValue = std::stoi(validStatus.second);
    smartLamp.setBuzzerStatus(statusValue);

    auto validSnooze = isValidRequestParam("snooze_timer", request, response);
    if(!validSnooze.first)
        return;
    std::string value = validSnooze.second;
    value.replace(0, 3, "");
    value.replace(value.length() - 3, 3, "");

    const char *time_details = value.c_str();
    struct tm tm{};
    strptime(time_details, "%H:%M", &tm);
    time_t t = mktime(&tm);
    smartLamp.setBuzzerSnoozeTime(t);
    response.send(Http::Code::Ok);
}

void SmartLightController::getBulbState(const Rest::Request &request, Http::ResponseWriter response) {
    json result;
    result["brightness"] = smartLamp.getBrightness();
    result["color"] = smartLamp.getBulbColor();
    result["presence"] = smartLamp.getPresence();
    result["isOn"] = smartLamp.getOnOffState();
    result["intensity"] = smartLamp.getBulbIntensity();

    response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
    response.send(Http::Code::Ok, result.dump(3));
}

void SmartLightController::registerBrightnessPattern(const Rest::Request &request, Http::ResponseWriter response) {

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
    bool detectPresence = std::stoi(isValid2.second) == 1;
    string detectedColor = isValid3.second;
    bool success = smartLamp.addBrightnessPresenceMapping(recordedBrightness, detectPresence, detectedColor);
//    Could end it right here and the intensity will be the default of 50.
    if(!success){
        response.send(Http::Code::Internal_Server_Error,
                      "Could not save color " + detectedColor + " for brightness: " + std::to_string(recordedBrightness));
        return;
    }
//    BONUS : If the user sends intensity as well, save it
    if(request.query().has("intensity")){
        Pistache::Optional<string> paramValue = request.query().get("intensity");
        if (paramValue.isEmpty() || paramValue.get().empty()) {
            response.send(Http::Code::Bad_Request, "Missing 'intensity' request parameter value.");
            return;
        }
        int intensity = std::stoi(paramValue.get());
        bool success = smartLamp.addColorIntensityMapping(detectedColor, intensity);
//        If it is not successful, add default Intensity
        if(!success){
            response.send(Http::Code::Internal_Server_Error,
                          "Could not save custom intensity for brightness: " + std::to_string(recordedBrightness));
            return;
        }
        response.send(Http::Code::Ok);
        return;
    }
    response.send(Http::Code::Ok);
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
    smartLamp.onBrightnessRecorded(recordedBrightness, detectPresence == 1);
    response.send(Http::Code::Ok);
}

void SmartLightController::registerSoundPattern(const Rest::Request &request, Http::ResponseWriter response) {

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
            sendBack["soundPattern"] = newPattern;
            sendBack["action"] = smartLamp.getActionForSoundPattern(newPattern);
            response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
            response.send(Http::Code::Created, sendBack.dump());
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
                sendBack["soundPattern"] = newPattern;
                sendBack["action"] = smartLamp.getActionForSoundPattern(newPattern);
                response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
                response.send(Http::Code::Created, sendBack.dump());
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
                sendBack["soundPattern"] = newPattern;
                sendBack["action"] = smartLamp.getActionForSoundPattern(newPattern);
                response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
                response.send(Http::Code::Created, sendBack.dump());
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
    sendBack["patterns"] = patterns;
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
    Pistache::Optional<string> paramValue = request.query().get(paramName);
    if (paramValue.isEmpty()) {
        response.send(Http::Code::Bad_Request, "Missing " + paramName + " request parameter value.");
        return {false, nullptr};
    }
    return {true, paramValue.get()};
}
