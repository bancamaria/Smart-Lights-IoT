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
        * TODO: create a function that checks and sends response for valid param
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
            response.send(Http::Code::Internal_Server_Error, "Could not insert" + newPattern);
        }
    }
}

void SmartLightController::getRegisteredPatterns(const Rest::Request &request, Http::ResponseWriter response) {
    auto patterns = smartLamp.getSoundPatterns();
    json sendBack;
//    sendBack["patterns"] = patterns;

    response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
    response.send(Http::Code::Ok, sendBack.dump(3));
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
