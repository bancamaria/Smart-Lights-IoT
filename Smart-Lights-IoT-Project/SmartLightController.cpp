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
    Routes::Get(router, "/buzzer/settings", Routes::bind(&SmartLightController::getBuzzerSettings, this));
    Routes::Post(router, "/buzzer/settings", Routes::bind(&SmartLightController::setBuzzerSettings, this));
    Routes::Get(router, "/bulb/settings", Routes::bind(&SmartLightController::getBulbSettings, this));
    Routes::Post(router, "/bulb/settings", Routes::bind(&SmartLightController::setBulbSettings, this));

    Routes::Get(router, "/color/settings", Routes::bind(&SmartLightController::getColorSettings, this));
    Routes::Post(router, "/color/settings", Routes::bind(&SmartLightController::setColorSettings, this));

    Routes::Get(router, "/microphone/patterns", Routes::bind(&SmartLightController::getRegisteredPatterns, this));
    Routes::Post(router, "/microphone/patterns", Routes::bind(&SmartLightController::registerPattern, this));
    Routes::Get(router, "/microphone", Routes::bind(&SmartLightController::onSoundRecorded, this));


}

/*
void SmartLightController::(){
}*/
void SmartLightController::doAuth(const Rest::Request& request, Http::ResponseWriter response) {
//    RARESITO' example:
//     Function that prints cookies
//    printCookies(request);

    // In the response object, it adds a cookie regarding the communications language.
    response.cookies()
            .add(Http::Cookie("lang", "en-US"));
    // Send the response
    response.send(Http::Code::Ok,"The Light Controller Works.");
}

void SmartLightController::getMicrophoneSettings(const Rest::Request& request, Http::ResponseWriter response) {
    json result;
    result["sensitivity"] = smartLamp.getMicSensitivity();
    response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
    response.send(Http::Code::Ok,result.dump(3));
}

void SmartLightController::setMicrophoneSettings(const Rest::Request &request, Http::ResponseWriter response) {
    /*
     * Parse the parameters from the url
     * */

    if(request.query().has("sensitivity")){
        int val = std::stoi(request.query().get("sensitivity").getOrElse("0"));
        smartLamp.setMicSensitivity(val);
    }
}

void SmartLightController::getBuzzerSettings(const Rest::Request& request, Http::ResponseWriter response) {
    json result;
    result["status"] = smartLamp.getBuzzerStatus();
    result["snooze_timer"] = smartLamp.getBuzzerSnoozeTime();
    response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
    response.send(Http::Code::Ok,result.dump(3));
}

void SmartLightController::setBuzzerSettings(const Rest::Request &request, Http::ResponseWriter response) {
    /*
     * Parse the parameters from the url
     * */

    if(request.query().has("status")){
        int val = std::stoi(request.query().get("status").getOrElse("0"));
        smartLamp.setBuzzerStatus(val);
    }
    if(request.query().has("snooze_timer")){
        string val = request.query().get("snooze_timer").getOrElse("0");

        time_t snooze_time;
        int  hh, mm, ss;
        struct tm whenStart;
        const char *zStart = val.c_str();

        sscanf(zStart, "%d:%d:%d", &hh, &mm, &ss);
        whenStart.tm_hour = hh;
        whenStart.tm_min = mm;
        whenStart.tm_sec = ss;
        whenStart.tm_isdst = -1;

        snooze_time = mktime(&whenStart);

        smartLamp.setBuzzerSnoozeTime(snooze_time);
    }
}

void SmartLightController::getBulbSettings(const Rest::Request &request, Http::ResponseWriter response) {
    json result;
    result["intensity"] = smartLamp.getBulbIntensity();
    response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
    response.send(Http::Code::Ok,result.dump(3));
}

void SmartLightController::setBulbSettings(const Rest::Request &request, Http::ResponseWriter response) {
    cout << request.body();
    if (request.query().has("status") && request.query().has("intensity")) {
        int valStatus = std::stoi(request.query().get("status").getOrElse("0"));
        smartLamp.setBulbStatus(valStatus);
        int valIntensity = std::stoi(request.query().get("intensity").getOrElse("0"));
        smartLamp.setBulbIntensity(valIntensity);
    }
}

void SmartLightController::getColorSettings(const Rest::Request &request, Http::ResponseWriter response) {
    json result;
    result["color"] = smartLamp.getColor();
    response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
    response.send(Http::Code::Ok, result.dump(3));
}

void SmartLightController::setColorSettings(const Rest::Request &request, Http::ResponseWriter response ) {
    /*
     * Parse the parameters from the url
     * */
    cout<<request.body();
    if (request.query().has("status") && request.query().has("color")) {
        int valStatus = std::stoi(request.query().get("status").getOrElse("0"));
        smartLamp.setBulbStatus(valStatus);
        int valColor = std::stoi(request.query().get("color").getOrElse("0"));
        smartLamp.setColor(valColor);
    }
}

void SmartLightController::registerPattern(const Rest::Request &request, Http::ResponseWriter response) {

    string bad_request_message = "In order to register a new pattern please provide the pattern and the action that maps it";
    auto isValid = isValidRequestParam("newPattern", request,response);
    if(!isValid.first)
        return;
    string newPattern = isValid.second;

    isValid = isValidRequestParam("mapsTo", request,response);
    if(!isValid.first)
        return;
    string soundMapping = isValid.second;
    if(!smartLamp.hasMapping(soundMapping)) {
        response.send(Http::Code::Bad_Request, "Invalid possible action.");
        return;
    }
    /*Up to this point, the endpoint is  valid /patterns/newPatterns=X&mapsTo=Z.
    * Will need to check further options depending on action specified in mapsTo */
    if(soundMapping == "TURN_ON_LIGHT" || soundMapping == "TURN_OFF_LIGHT"){
        auto succ = smartLamp.addSoundPattern(newPattern, soundMapping);
        if(succ) {
            json sendBack;
            sendBack["patterns"] = smartLamp.getSoundPatterns();
            response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
            response.send(Http::Code::Ok, sendBack.dump());
            return;
        }

    }

    if(soundMapping == "TURN_ON_BUZZER" || soundMapping == "TURN_OFF_BUZZER"){
        auto succ = smartLamp.addSoundPattern(newPattern, soundMapping);
        if(succ) {
            json sendBack;
            sendBack["patterns"] = smartLamp.getSoundPatterns();
            response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
            response.send(Http::Code::Ok, sendBack.dump());
            return;
        }

    }

    if(soundMapping == "CHANGE_COLOR"){
        /*Check if the requestParam 'color' is present*/
        auto isValid = isValidRequestParam("color",request, response);
        if(isValid.first){
            string color = isValid.second;
            auto succss = smartLamp.addSoundPattern(newPattern, soundMapping, color);
            if(succss) {
                json sendBack;
                sendBack["patterns"] = smartLamp.getSoundPatterns();
                response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
                response.send(Http::Code::Ok, sendBack.dump());
                return;
            }
        }
    }

    if(soundMapping == "START_COLOR_PATTERN"){
        /*
        * Will need to check further for validity of 'colorPattern'
        *
        * */
        auto isValid = isValidRequestParam("colorPattern",request, response);
        if(isValid.first){
            string colorPattern = isValid.second;
            auto succss = smartLamp.addSoundPattern(newPattern, soundMapping, colorPattern);
            if(succss){
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
    auto isValid = isValidRequestParam("record",request, response);
    if(!isValid.first)
        return;
    string recordedSound = isValid.second;

    pair<smartlamp::light::LightState, smartlamp::buzzer::BuzzerState> lampState = smartLamp.onSoundRecorded(recordedSound);

    smartlamp::light::LightState lightState = lampState.first;
    smartlamp::buzzer::BuzzerState buzzerState = lampState.second;

    json j;
    j["lightState"] = lightState;
    j["buzzerState"] = buzzerState;
    response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
    response.send(Http::Code::Ok, j.dump());
}

std::pair<bool, std::string> SmartLightController::isValidRequestParam(const std::string& paramName, const Rest::Request &request,
                                                                       Http::ResponseWriter& response ) {

    if (!request.query().has(paramName)) {
        response.send(Http::Code::Bad_Request, "Missing " + paramName + " request parameter.");
        return {false, nullptr};
    }
    std::string paramValue = request.query().get(paramName).getOrElse("");
    if (paramValue.empty()) {
        response.send(Http::Code::Bad_Request, "Missing " + paramName + " request parameter value.");
        return {false, nullptr};
    }
    return {true, paramValue};
}

