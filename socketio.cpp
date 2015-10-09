#include <iostream>

#include "socketio.hpp"
#include "profile.hpp"

SocketIO::SocketIO() {
}

void SocketIO::Initialize(bool chatEnabled) {
    using std::placeholders::_1;
    using std::placeholders::_2;
    using std::placeholders::_3;
    using std::placeholders::_4;

    using namespace sio;

    this->chatEnabled = chatEnabled;

    client.set_open_listener(std::bind(&SocketIO::OpenListener, this));
    client.set_fail_listener(std::bind(&SocketIO::FailListener, this));
    //client.set_close_listener();

    //client.socket()->on(std::string("pm"), std::bind(&SocketIO::OnPrivateMessage, this, _1, _2, _3));
    //client.socket()->on(std::string("msg"), std::bind(&SocketIO::OnMessage, this, _1, _2, _3, _4));
}

void SocketIO::Connect(Profile* profile) {
    this->profile = profile;
    client.connect("https://sockets.primedice.com");
}

void SocketIO::OpenListener() {
    // Initial authentication to the socket-io host
    client.socket()->emit("user", profile->accesstoken.toStdString());

    // Subscribe to both chat and private messages
    if(chatEnabled)
        client.socket()->emit("chat");

    // Subscribe to tip notifications
    client.socket()->emit("tip");
}

void SocketIO::FailListener() {
     //client.connect("https://sockets.primedice.com");

    std::cout << "fail" << std::endl;
}
