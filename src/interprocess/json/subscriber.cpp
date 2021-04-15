#include <goby/middleware/marshalling/json.h>

#include <goby/zeromq/application/single_thread.h>

#include "messages/groups.h"

#include "config.pb.h"
#include "nav.h"

using Base = goby::zeromq::SingleThreadApplication<JsonSubscriberConfig>;

class JsonSubscriber : public Base
{
  public:
    JsonSubscriber()
    {
        auto nav_callback = [this](const NavigationReport& nav) { this->incoming_nav(nav); };
        // subscribe directly to this type (using JSON since goby_json_type is defined in the struct)
        interprocess().subscribe<groups::nav>(nav_callback);

        // subscribe with regex to all json messages on this group
        interprocess().subscribe_type_regex<groups::nav, nlohmann::json>(
            [this](std::shared_ptr<const nlohmann::json> json, const std::string& type) {
                std::cout << "As json (type: " << type << "): " << json->dump() << std::endl;
            });
    }

    void incoming_nav(const NavigationReport& nav) { std::cout << "Rx: " << nav << std::endl; }
};

int main(int argc, char* argv[]) { return goby::run<JsonSubscriber>(argc, argv); }
