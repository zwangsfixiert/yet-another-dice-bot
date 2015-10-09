#ifndef SOCKETIO_HPP
#define SOCKETIO_HPP

#include "sio_client.h"

#include "profile.hpp"

/*
#ifdef WIN32
#define BIND_EVENT(IO,EV,FN) \
    do{ \
        socket::event_listener_aux l = FN;\
        IO->on(EV,l);\
    } while(0)

#else
#define BIND_EVENT(IO,EV,FN) \
    IO->on(EV,FN)
#endif
*/
#define QT_NO_EMIT
#undef emit

class SocketIO {
public:
    SocketIO();
    void Initialize(bool chatEnabled);
    void Connect(Profile* profile);

    void OnPrivateMessage(std::string const& name, sio::message::ptr const& data, bool hasAck, sio::message::ptr &ack_resp);
    void OnMessage(std::string const& name, sio::message::ptr const& data, bool hasAck, sio::message::ptr &ack_resp);
    void OnTip(std::string const& name, sio::message::ptr const& data, bool hasAck, sio::message::ptr &ack_resp);

    void OpenListener();
    void FailListener();

    sio::client& GetSio() { return client; }

private:
    sio::client client;
    Profile* profile{nullptr};
    bool chatEnabled{false};
};

#endif // SOCKETIO_HPP
