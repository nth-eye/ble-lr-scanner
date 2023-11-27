#ifndef NTH_COAP_CLIENT_H
#define NTH_COAP_CLIENT_H

#include "nth/coap/session.h"

namespace nth::coap {

struct Client : Session<Client> {
    void cancel();
    void request();
private:
    ClientState state = client_st_idle;  
};

}

#endif