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
     *  RARESITO's EXAMPLE
     * Routes::Get(router, "/ready", Routes::bind(&Generic::handleReady));

    Routes::Post(router, "/settings/:settingName/:value", Routes::bind(&MicrowaveEndpoint::setSetting, this));
    Routes::Get(router, "/settings/:settingName/", Routes::bind(&MicrowaveEndpoint::getSetting, this));
     */
    Routes::Get(router, "/home", Routes::bind(&SmartLightController::doAuth, this));

    /*Here we will post messages that will simulate the sound "recorded" by the smart lamp*/
    Routes::Get(router, "/microphone/settings", Routes::bind(&SmartLightController::getMicrophoneSettings, this));
    Routes::Post(router, "/microphone/settings", Routes::bind(&SmartLightController::setMicrophoneSettings, this));

    Routes::Get(router, "/microphone/patterns", Routes::bind(&SmartLightController::getRegisteredPatterns, this));
    Routes::Post(router, "/microphone/patterns", Routes::bind(&SmartLightController::registerPattern, this));


    Routes::Get(router, "/bulb/settings", Routes::bind(&SmartLightController::getBulbSettings, this));
    Routes::Post(router, "/bulb/settings", Routes::bind(&SmartLightController::setBulbSettings, this));
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
    cout<<request.body();
    if(request.query().has("sensitivity")){
        int val = std::stoi(request.query().get("sensitivity").getOrElse("0"));
        smartLamp.setMicSensitivity(val);
    }


}


void SmartLightController::registerPattern(const Rest::Request &request, Http::ResponseWriter response) {

    string bad_request_message = "In order to register a new pattern please provide the pattern and the action that maps it";
    if(!request.query().has("newPattern")) {
        response.send(Http::Code::Bad_Request, "newPattern must not be null.");
        return;
    }
        string newPattern = request.query().get("newPattern").getOrElse("");
        if(newPattern.empty()){
            response.send(Http::Code::Bad_Request, "newPattern must not be null.");
            return;
        }
        if(!request.query().has("mapsTo")){
            response.send(Http::Code::Bad_Request, "newPattern must have a valid mapping action.");
            return;
        }
        string soundMapping = request.query().get("mapsTo").getOrElse("");
        if(soundMapping.empty() || !smartLamp.hasMapping(soundMapping)) {
            response.send(Http::Code::Bad_Request, "newPattern must have a valid mapping action.");
            return;
        }
        bool res = smartLamp.addSoundPattern(newPattern, soundMapping);
        if(res){
            json sendBack;
            sendBack["newPattern"] = smartLamp.getSoundPatterns()[newPattern];

            response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
            response.send(Http::Code::Ok, sendBack.dump());
            return;
        }else{
            response.send(Http::Code::Internal_Server_Error, "Failed to insert: " + newPattern + "as" + soundMapping);
            return;
        }

}

void SmartLightController::getRegisteredPatterns(const Rest::Request &request, Http::ResponseWriter response) {
    auto patterns = smartLamp.getSoundPatterns();
    json sendBack;
    sendBack["patterns"] = patterns;

    response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
    response.send(Http::Code::Ok, sendBack.dump(3));
}


void SmartLightController::getBulbSettings(const Rest::Request &request, Http::ResponseWriter response) {
    json result;
    result["intensity"] = smartLamp.getBulbIntensity();
    response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
    response.send(Http::Code::Ok,result.dump(3));
}

void SmartLightController::setBulbSettings(const Rest::Request &request, Http::ResponseWriter response) {
    cout<<request.body();
    if(request.query().has("status") && request.query().has("intensity")) {
        int valStatus = std::stoi(request.query().get("status").getOrElse("0"));
        smartLamp.setBulbStatus(valStatus);
        int valIntensity = std::stoi(request.query().get("intensity").getOrElse("0"));
        smartLamp.setBulbIntensity(valIntensity);
    }
    // something more to add
}



