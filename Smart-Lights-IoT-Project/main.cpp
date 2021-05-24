#include <pistache/endpoint.h>
#include "SmartLightController.h"
#include "MqttSubscriber.h"
using namespace Pistache;
using namespace std;



int main(int argc, char *argv[]) {
    sigset_t signals;
    if (sigemptyset(&signals) != 0
        || sigaddset(&signals, SIGTERM) != 0
        || sigaddset(&signals, SIGINT) != 0
        || sigaddset(&signals, SIGHUP) != 0
        || pthread_sigmask(SIG_BLOCK, &signals, nullptr) != 0) {
        perror("install signal handler failed");
        return 1;
    }


    int thr = 2;

    cout << "Cores = " << hardware_concurrency() << endl;
    cout << "Using " << thr << " threads" << endl;

    Port port(9082);
    Address addr(Ipv4::any(), port);
    // Initialize and start the server
    SmartLightController ctrl(addr);
    ctrl.init(thr);
    ctrl.start();

    MqttSubscriber subscriber(&ctrl);
    subscriber.start();
    // Code that waits for the shutdown sinal for the server
    int signal = 0;
    int status = sigwait(&signals, &signal);
    if (status == 0)
    {
        std::cout << "received signal " << signal << std::endl;
    }
    else
    {
        std::cerr << "sigwait returns " << status << std::endl;
    }

    ctrl.stop();
}