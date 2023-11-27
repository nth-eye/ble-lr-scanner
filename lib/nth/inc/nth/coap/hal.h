#ifndef NTH_COAP_HAL_H
#define NTH_COAP_HAL_H

#include "nth/coap/base.h"
#include <source_location>

namespace nth::coap {

struct app_error {
    error num = error::ok;
    std::source_location loc = std::source_location::current();
};

enum class app_event_type {
    error,
    transmission,
    remove_observer,
    append_observer,
};

union app_event_data {
    app_error err;
};

struct app_event {
    app_event_type type;
    app_event_data data;
};

inline void app_handler(app_event_type type, app_event_data data = {})
{
    // TODO
}

inline timestamp app_get_time()
{
    return 0;
}

}

#endif