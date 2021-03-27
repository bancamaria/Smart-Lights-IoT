//
// Created by Catalin on 3/13/2021.
//

#ifndef SMART_LIGHTS_IOT_PROJECT_SMARTLIGHTCONTROLLER_H
#define SMART_LIGHTS_IOT_PROJECT_SMARTLIGHTCONTROLLER_H

#include <algorithm>

#include <pistache/net.h>
#include <pistache/http.h>
#include <pistache/peer.h>
#include <pistache/http_headers.h>
#include <pistache/cookie.h>
#include <pistache/router.h>
#include <pistache/endpoint.h>
#include <pistache/common.h>

#include <signal.h>
#include "SmartLamp.h"
using namespace Pistache;
using namespace std;

class SmartLightController {
public:
    explicit SmartLightController(Address addr)
            : httpEndpoint(std::make_shared<Http::Endpoint>(addr)) {}

    // Initialization of the server. Additional options can be provided here
    void init(size_t thr = 2);

    // Server is started threaded.
    void start();

    // When signaled server shuts down
    void stop();

    /*
     * Note: required to add a refference to ResponseWriter because passing by value is not allowed
     * (copy constructor is private for some reasone).*/
    void getMicrophoneSettings(const Rest::Request& request, Http::ResponseWriter response);
    void setMicrophoneSettings(const Rest::Request& request, Http::ResponseWriter response);

    /*
     * POST: http://localhost:port/microphone/patterns?newPattern=val&mapsTo=TURN_ON/OFF_LIGHT
     * or
     * POST: http://localhost:port/microphone/patterns?newPattern=val&mapsTo=CHANGE_COLOR&color=COLOR
     * or
     * POST: http://localhost:port/microphone/patterns?newPattern=val&mapsTo=START_LIGHT_PATTERN&ligthPattern=PATTERN_CONFIG
     *
     * */
    void registerPattern(const Rest::Request& request, Http::ResponseWriter response);
    void getRegisteredPatterns(const Rest::Request& request, Http::ResponseWriter response);

    /*GET: http://localhost:port/microphone?recorded=X*/
    void onSoundRecorded(const Rest::Request& request, Http::ResponseWriter response);

private:
    using Lock = std::mutex;
    using Guard = std::lock_guard<Lock>;
    Lock lock;

    /*SmartLamp will be our high-level service. This will in fac resolve all the requests intercepted by the controller.*/
    SmartLamp smartLamp;

    // Defining the httpEndpoint and a router.
    std::shared_ptr<Http::Endpoint> httpEndpoint;
    Rest::Router router;

    void setupRoutes();
    void doAuth(const Rest::Request &request, Http::ResponseWriter response);
    static std::pair<bool, std::string> isValidRequestParam(const std::string& paramName, const Rest::Request& request,
                                                            Http::ResponseWriter& response);

};


#endif //SMART_LIGHTS_IOT_PROJECT_SMARTLIGHTCONTROLLER_H
