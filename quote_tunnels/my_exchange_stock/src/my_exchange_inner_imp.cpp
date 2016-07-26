#include "my_exchange_inner_imp.h"
#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds

void* MYExchangeInnerImp::QryPosForInit()
{
    T_QryPosition qry_pos;
    std::this_thread::sleep_for (std::chrono::milliseconds(150));
    //usleep(150 * 1000); // wait for login state
    while (!have_send_qry_pos_flag)
    {
        p_tunnel->QueryPosition(&qry_pos);

        std::this_thread::sleep_for (std::chrono::milliseconds(1001));
        //usleep(1001 * 1000); // wait 1.001s for flow control
    }
    return NULL;
}
