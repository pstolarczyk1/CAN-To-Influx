/* main.cpp */

#include <event2/event.h>
#include <event2/listener.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>


#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sstream>
#include <vector>
#include <signal.h>

#include "influx-sender.h"

#include "process-message.h"

#define BUFFER_SIZE 1024
#define INFLUX_URL "http://localhost:8086/api/v2/write?org=?ORG&bucket=data&precision=s"
#define INFLUX_TOKEN  "INFLUX-TOKEN=="

static const unsigned short PORT = 2137;
#define LISTEN_BACKLOG 3

void accept_error_cb(struct evconnlistener* listener, void* ctx);
void accept_conn_cb(struct evconnlistener* listener, evutil_socket_t fd, struct sockaddr* address, int socklen, void* ctx);
void event_cb(struct bufferevent* bev, short events, void* ctx);
void read_cb(struct bufferevent* bev, void* ctx);

int main() {
    struct event_base* base = event_base_new();
    if (!base) {
        std::cerr << "Could not initialize libevent!\n";
        return 1;
    }

    sockaddr_in sin {};
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(PORT);

    struct evconnlistener* listener = evconnlistener_new_bind(base, accept_conn_cb, base,
        LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, LISTEN_BACKLOG,
        (sockaddr*)&sin, sizeof(sin));

    if (!listener) {
        std::cerr << "Could not create a listener!\n";
        return 1;
    }

    evconnlistener_set_error_cb(listener, accept_error_cb);
    std::cout << "Server listening on port " << PORT << "...\n";
    event_base_dispatch(base);
    evconnlistener_free(listener);
    event_base_free(base);

    return 0;
}

void read_cb(struct bufferevent* bev, void* ctx) {
    std::vector<char> buffer(BUFFER_SIZE, 0);
    // Read data into buffer
    int bytes_received = bufferevent_read(bev, buffer.data(), buffer.size() - 1);
    if (bytes_received <= 0) {
        return;
    }
    buffer[bytes_received] = '\0';
    //std::cout << "Raw received: ";
    //std::cout.write(buffer.data(), bytes_received);
    //std::cout << std::endl;
    //Split into frames by ';'
    char* saveptr1;
    char* frame = strtok_r(buffer.data(), ";", &saveptr1);
    while (frame != nullptr) {
        // Each frame: "0xID,DATA1 DATA2 ... DATA8"
        char* saveptr2;
        char* addr = strtok_r(frame, ",", &saveptr2);  // Extract CAN ID
        char* data = strtok_r(nullptr, ",", &saveptr2); // Extract data bytes
        if (addr == nullptr || data == nullptr) {
            frame = strtok_r(nullptr, ";", &saveptr1);
            continue;  // Skip malformed frame
        }
        // Convert CAN ID from hex
        int f_addr = 0;
        try {
            f_addr = std::strtol(addr, nullptr, 16);
        } catch (...) {
            std::cerr << "Invalid CAN ID: " << addr << std::endl;
            frame = strtok_r(nullptr, ";", &saveptr1);
            continue;
        }
        // Parse hex data bytes
        std::vector<uint8_t> bytes;
        std::stringstream ss(data);
        std::string byteStr;
        while (std::getline(ss, byteStr, ' ')) {
            if (!byteStr.empty()) {
                try {
                    bytes.push_back(static_cast<uint8_t>(std::stoi(byteStr, nullptr, 16)));
                } catch (...) {
                    std::cerr << "Invalid byte: " << byteStr << std::endl;
                }
            }
        }
        if (!bytes.empty()) {
            // Pass frame to application logic
            processCANMessage(f_addr, bytes.size(), bytes.data());
        }
        // Get next frame
        frame = strtok_r(nullptr, ";", &saveptr1);
    }

    std::cout.flush();
}



void event_cb(struct bufferevent* bev, short events, void* ctx) {
    if (events & BEV_EVENT_EOF) {
        std::cout << "\nConnection closed.\n";
    } else if (events & BEV_EVENT_ERROR) {
        std::cerr << "Error on connection: " << strerror(errno) << "\n";
    }
    bufferevent_free(bev);
}

void accept_conn_cb(struct evconnlistener* listener, evutil_socket_t fd, struct sockaddr* address, int socklen, void* ctx) {
    struct event_base* base = (struct event_base*)ctx;

    struct bufferevent* bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);

    bufferevent_setcb(bev, read_cb, nullptr, event_cb, nullptr);
    bufferevent_enable(bev, EV_READ | EV_WRITE);

    std::cout << "New connection accepted.\n";
}

void accept_error_cb(struct evconnlistener* listener, void* ctx) {
    struct event_base* base = (struct event_base*)ctx;
    int err = EVUTIL_SOCKET_ERROR();
    std::cerr << "Listener error " << err << ": " << evutil_socket_error_to_string(err) << "\n";
    event_base_loopexit(base, nullptr);
}
