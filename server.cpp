#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

constexpr std::uint16_t kDefaultPort = 8080;
constexpr int kBacklog = 16;

class Client {
public:
    int fd = -1;
    std::string nickname;
    std::string room;
};

class Room {
public:
    std::string name;
    std::vector<int> members;
};

class ServerState {
public:
    std::mutex mutex;
    std::unordered_map<int, Client> clients_by_fd;
    std::unordered_map<std::string, Room> rooms_by_name;
    bool running = true;
};

ServerState& state() {
    static ServerState instance;
    return instance;
}

std::uint16_t parse_port(int argc, char* argv[]) {
    if (argc >= 2) {
        return static_cast<std::uint16_t>(std::stoi(argv[1]));
    }
    return kDefaultPort;
}

bool send_all(int fd, const std::string& message) {
    (void)fd;
    (void)message;
    return false;
}

bool recv_line(int fd, std::string& out) {
    (void)fd;
    out.clear();
    return false;
}

int create_listener(std::uint16_t port) {
    int listener = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0) {
        return -1;
    }

    int reuse = 1;
    if (::setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        ::close(listener);
        return -1;
    }

    sockaddr_in address {};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(port);

    if (::bind(listener, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
        ::close(listener);
        return -1;
    }

    if (::listen(listener, kBacklog) < 0) {
        ::close(listener);
        return -1;
    }

    return listener;
}

void register_client(int fd) {
    std::lock_guard<std::mutex> lock(state().mutex);
    state().clients_by_fd.emplace(fd, Client{fd, "", ""});
}

void unregister_client(int fd) {
    std::lock_guard<std::mutex> lock(state().mutex);
    state().clients_by_fd.erase(fd);
}

void broadcast_room(const std::string& room_name, const std::string& message, int exclude_fd = -1) {
    (void)room_name;
    (void)message;
    (void)exclude_fd;
}

void join_room(int fd, const std::string& room_name) {
    std::lock_guard<std::mutex> lock(state().mutex);
    Client& client = state().clients_by_fd.at(fd);
    client.room = room_name;

    Room& room = state().rooms_by_name[room_name];
    room.name = room_name;
    room.members.push_back(fd);
}

void leave_room(int fd) {
    std::lock_guard<std::mutex> lock(state().mutex);
    auto client_it = state().clients_by_fd.find(fd);
    if (client_it == state().clients_by_fd.end()) {
        return;
    }

    const std::string room_name = client_it->second.room;
    if (room_name.empty()) {
        return;
    }

    auto room_it = state().rooms_by_name.find(room_name);
    if (room_it != state().rooms_by_name.end()) {
        auto& members = room_it->second.members;
        members.erase(std::remove(members.begin(), members.end(), fd), members.end());
        if (members.empty()) {
            state().rooms_by_name.erase(room_it);
        }
    }

    client_it->second.room.clear();
}

void print_admin_state() {
    std::lock_guard<std::mutex> lock(state().mutex);
    std::cout << "[admin] clients: " << state().clients_by_fd.size() << "\n";
    std::cout << "[admin] rooms: " << state().rooms_by_name.size() << "\n";
}

void admin_console() {
    while (state().running) {
        std::string command;
        if (!std::getline(std::cin, command)) {
            break;
        }

        if (command == "help") {
            std::cout << "help, rooms, clients, shutdown\n";
        } else if (command == "rooms" || command == "clients") {
            print_admin_state();
        } else if (command == "shutdown") {
            state().running = false;
            break;
        } else if (!command.empty()) {
            std::cout << "Unknown command: " << command << "\n";
        }
    }
}

void handle_client(int fd) {
    register_client(fd);
    send_all(fd, "Welcome to the learning server\n");

    std::string line;
    while (state().running && recv_line(fd, line)) {
        if (line == "quit") {
            break;
        }

        if (line == "help") {
            send_all(fd, "Commands: help, quit, join <room>, leave, msg <text>\n");
            continue;
        }

        send_all(fd, "TODO: command not implemented yet\n");
    }

    leave_room(fd);
    unregister_client(fd);
    ::close(fd);
}

void run_server(std::uint16_t port) {
    int listener = create_listener(port);
    if (listener < 0) {
        std::cerr << "Failed to start listener on port " << port << "\n";
        return;
    }

    std::thread admin_thread(admin_console);

    while (state().running) {
        sockaddr_in client_address {};
        socklen_t client_length = sizeof(client_address);
        int client_fd = ::accept(listener, reinterpret_cast<sockaddr*>(&client_address), &client_length);
        if (client_fd < 0) {
            continue;
        }

        std::thread(handle_client, client_fd).detach();
    }

    ::close(listener);
    if (admin_thread.joinable()) {
        admin_thread.join();
    }
}

int main(int argc, char* argv[]) {
    const std::uint16_t port = parse_port(argc, argv);
    std::cout << "Starting learning server on port " << port << "\n";
    run_server(port);
    return 0;
}

