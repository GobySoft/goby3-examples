#include <goby/middleware/marshalling/protobuf.h> 
//
#include <goby/zeromq/application/single_thread.h>
#include <goby/middleware/protobuf/logger.pb.h>
#include <goby/middleware/log/groups.h>

#include "messages/groups.h" 
#include "messages/nav.pb.h"


#include "config.pb.h"

using Base = goby::zeromq::SingleThreadApplication<LogControlPublisherConfig>;
using protobuf::NavigationReport;

class LogControlPublisher : public Base
{
  public:
    LogControlPublisher() : Base(1 /*hertz*/)
    {
    }

    void loop() override
    {
        NavigationReport nav;
        nav.set_x(95 + std::rand() % 20);
        nav.set_y(195 + std::rand() % 20);
        nav.set_z(-305 + std::rand() % 10);

        std::cout << "Tx: " << nav.DebugString() << std::flush;
        interprocess().publish<groups::nav>(nav);

        static int i = 0;
        ++i;
        if(i == 10)
        {
            // stop logging
            std::cout << "STOP LOGGING" << std::endl;
            goby::middleware::protobuf::LoggerRequest request;
            request.set_requested_state(goby::middleware::protobuf::LoggerRequest::STOP_LOGGING);
            interprocess().publish<goby::middleware::groups::logger_request>(request);
        }
        else if(i == 20)
        {
            // start logging
            std::cout << "START LOGGING" << std::endl;
            goby::middleware::protobuf::LoggerRequest request;
            request.set_requested_state(goby::middleware::protobuf::LoggerRequest::START_LOGGING);
            interprocess().publish<goby::middleware::groups::logger_request>(request);
        }
        else if(i == 30)
        {
            // rotate the log and keep logging
            std::cout << "ROTATE LOG" << std::endl;
            goby::middleware::protobuf::LoggerRequest request;
            request.set_requested_state(goby::middleware::protobuf::LoggerRequest::ROTATE_LOG);
            interprocess().publish<goby::middleware::groups::logger_request>(request);
        }
        
    }
};

int main(int argc, char* argv[]) { return goby::run<LogControlPublisher>(argc, argv); }
