//
// Created by Catalin on 3/13/2021.
//

#include "SmartLightController.h"

void SmartLightController::init(size_t thr) {

}

void setupRoutes() {
    using namespace Rest;
    // Defining various endpoints
    // Generally say that when http://localhost:9080/ready is called, the handleReady function should be called.
    Routes::Get(router, "/ready", Routes::bind(&Generic::handleReady));
    Routes::Get(router, "/auth", Routes::bind(&MicrowaveEndpoint::doAuth, this));
    Routes::Post(router, "/settings/:settingName/:value", Routes::bind(&MicrowaveEndpoint::setSetting, this));
    Routes::Get(router, "/settings/:settingName/", Routes::bind(&MicrowaveEndpoint::getSetting, this));
}



void doAuth(const Rest::Request& request, Http::ResponseWriter response) {
    // Function that prints cookies
    printCookies(request);
    // In the response object, it adds a cookie regarding the communications language.
    response.cookies()
            .add(Http::Cookie("lang", "en-US"));
    // Send the response
    response.send(Http::Code::Ok);
}

// Endpoint to configure one of the Microwave's settings.
void setSetting(const Rest::Request& request, Http::ResponseWriter response){
    // You don't know what the parameter content that you receive is, but you should
    // try to cast it to some data structure. Here, I cast the settingName to string.
    auto settingName = request.param(":settingName").as<std::string>();

    // This is a guard that prevents editing the same value by two concurent threads.
    Guard guard(microwaveLock);


    string val = "";
    if (request.hasParam(":value")) {
        auto value = request.param(":value");
        val = value.as<string>();
    }

    // Setting the microwave's setting to value
    int setResponse = mwv.set(settingName, val);

    // Sending some confirmation or error response.
    if (setResponse == 1) {
        response.send(Http::Code::Ok, settingName + " was set to " + val);
    }
    else {
        response.send(Http::Code::Not_Found, settingName + " was not found and or '" + val + "' was not a valid value ");
    }

}

// Setting to get the settings value of one of the configurations of the Microwave
void getSetting(const Rest::Request& request, Http::ResponseWriter response){
    auto settingName = request.param(":settingName").as<std::string>();

    Guard guard(microwaveLock);

    string valueSetting = mwv.get(settingName);

    if (valueSetting != "") {

        // In this response I also add a couple of headers, describing the server that sent this response, and the way the content is formatted.
        using namespace Http;
        response.headers()
                .add<Header::Server>("pistache/0.1")
                .add<Header::ContentType>(MIME(Text, Plain));

        response.send(Http::Code::Ok, settingName + " is " + valueSetting);
    }
    else {
        response.send(Http::Code::Not_Found, settingName + " was not found");
    }
}

