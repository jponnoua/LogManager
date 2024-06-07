#ifndef MY_LOG_MANAGER_H_
#define MY_LOG_MANAGER_H_

#include <string>
#include <fstream>
#include <mqueue.h>
#include <chrono>
#include <syslog.h>
#include <inttypes.h>

#include "../../Utils/myMsg.h"

class myLogManager {
public:
    myLogManager();
    ~myLogManager();
    
    void run();
    void stop();

private:
    mqd_t mq;
    bool isRunning;
};

#endif