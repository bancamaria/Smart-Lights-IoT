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

private:
    using Lock = std::mutex;
    using Guard = std::lock_guard<Lock>;
    Lock microwaveLock;

   SmartLamp smartLamp;

    // Defining the httpEndpoint and a router.
    std::shared_ptr<Http::Endpoint> httpEndpoint;
    Rest::Router router;

    void setupRoutes();
    void doAuth(const Rest::Request &request, Http::ResponseWriter response);

};


#endif //SMART_LIGHTS_IOT_PROJECT_SMARTLIGHTCONTROLLER_H
