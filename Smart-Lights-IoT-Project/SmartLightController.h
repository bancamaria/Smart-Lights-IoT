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
#include "nlohmann/json.hpp"
using namespace Pistache;
using namespace std;
using namespace nlohmann;


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

    void getMicrophoneSettings(const Rest::Request& request, Http::ResponseWriter response);
    void setMicrophoneSettings(const Rest::Request& request, Http::ResponseWriter response);
    void registerPattern(const Rest::Request& request, Http::ResponseWriter response);
    void getRegisteredPatterns(const Rest::Request& request, Http::ResponseWriter response);

    void getBulbSettings(const Rest::Request& request, Http::ResponseWriter response);
    void setBulbSettings(const Rest::Request& request, Http::ResponseWriter response);

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

};


#endif //SMART_LIGHTS_IOT_PROJECT_SMARTLIGHTCONTROLLER_H
