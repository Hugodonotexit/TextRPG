#include <iostream>
#include <omp.h>
#include <string>
#include <set>
#include <map>
#include <mutex>
#include <boost/asio.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <functional>
#include <uuid/uuid.h>
#include "single_include/nlohmann/json.hpp"
#include "readfile.h"
#include "var.h"

// Typedefs for convenience
typedef websocketpp::server<websocketpp::config::asio> server;
typedef websocketpp::connection_hdl connection_hdl;

// Server state
std::set<connection_hdl, std::owner_less<connection_hdl>> connections;
std::map<connection_hdl, Player, std::owner_less<connection_hdl>> players;
std::mutex connection_mutex;

// Utility function to generate a new UUID
std::string generate_uuid() {
    uuid_t uuid;
    uuid_generate_random(uuid);
    char uuid_str[37];
    uuid_unparse(uuid, uuid_str);
    return std::string(uuid_str);
}


// Function to handle incoming messages
void on_message(server* s, connection_hdl hdl, server::message_ptr msg) {
    readfile rf;
    std::string payload = msg->get_payload();
    std::cout << "Received message: " << payload << std::endl;

    // Parse the incoming JSON message
    auto json_msg = nlohmann::json::parse(payload);
    uint_fast8_t type = json_msg["type"];

    std::lock_guard<std::mutex> guard(connection_mutex);

    if (type == -1) {
        std::string username = json_msg["username"];     
        if (!rf.check_username("users.csv", username))
        {
            // Generate a new UUID for the new player
            std::string uuid = generate_uuid();

            // Write the new player to the CSV file
            rf.write_user_to_csv("users.csv", username, uuid);
            
            // Store the new player information
            players[hdl] = Player{username, uuid};

            // Send back the registration success message with the UUID
            nlohmann::json response;
            response["type"] = -1;
            response["success"] = true;
            response["username"] = username;
            response["uuid"] = uuid;
            s->send(hdl, response.dump(), msg->get_opcode());
            std::cout << "Registered new player: " << username << " with UUID: " << uuid << std::endl;
        } else {
            nlohmann::json response;
            response["type"] = -1;
            response["success"] = false;
            s->send(hdl, response.dump(), msg->get_opcode());
            std::cout << "Registered new player fail: " << username << std::endl;
        }
        
    } else if (type == 0) {
        std::string username = json_msg["username"];
        std::string uuid = json_msg["uuid"];

        // Check if the provided username and UUID match an existing player
        auto user_data = rf.read_users_from_csv("users.csv");
        bool valid = user_data.find(username) != user_data.end() && user_data[username] == uuid;

        if (valid) {
            players[hdl] = Player{username, uuid};
            std::cout << "Player " << username << " logged in with UUID: " << uuid << std::endl;

            // Send back the login success message
            nlohmann::json response;
            response["type"] = 0;
            response["success"] = true;
            s->send(hdl, response.dump(), msg->get_opcode());
        } else {
            std::cout << "Invalid login attempt for player: " << username << std::endl;
            // Send back the login failure message
            nlohmann::json response;
            response["type"] = 0;
            response["success"] = false;
            s->send(hdl, response.dump(), msg->get_opcode());
            }
    } else if (type == 1) {
        std::string command = json_msg["data"];
        std::cout << "Received command from " << players[hdl].uuid << "|" << players[hdl].username << ": " << command << std::endl;
        
    } else if (type == 2) {
            std::string command = json_msg["data"];
            int i = 0;
            bool isfound = false;
            while (!std::isspace(command[i]) && i < command.size())
            {
                i++;
            }
            std::string usr = command.substr(0,i);
            #pragma omp for
            for (const auto& pair : players)
            {
                if (pair.second.username == usr)
                {
                    isfound = true;
                    if (isfound)
                    {
                        nlohmann::json response;
                        response["type"] = 99;
                        response["data"] = "Message from " + players[hdl].username + ": " + command.substr(i+1);
                        s->send(pair.first, response.dump(), msg->get_opcode());
                    }
                    break;
                }
            }
            if (!isfound)
            {
                nlohmann::json response;
                response["type"] = 99;
                response["data"] = "System: " + usr + " not found!";
                s->send(hdl, response.dump(), msg->get_opcode());
            }
    }
}

// Function to handle new connections
void on_open(server* s, connection_hdl hdl) {
    std::lock_guard<std::mutex> guard(connection_mutex);
    connections.insert(hdl);
    std::cout << "New connection established" << std::endl;
}

// Function to handle closed connections
void on_close(server* s, connection_hdl hdl) {
    std::lock_guard<std::mutex> guard(connection_mutex);
    connections.erase(hdl);
    players.erase(hdl);
    std::cout << "Connection closed" << std::endl;
}

int main() {
    // Create a server endpoint
    server ws_server;

    // Set logging settings
    ws_server.set_access_channels(websocketpp::log::alevel::all);
    ws_server.clear_access_channels(websocketpp::log::alevel::frame_payload);

    // Initialize Asio
    ws_server.init_asio();

    // Register our message handler
    ws_server.set_message_handler(std::bind(&on_message, &ws_server, std::placeholders::_1, std::placeholders::_2));
    ws_server.set_open_handler(std::bind(&on_open, &ws_server, std::placeholders::_1));
    ws_server.set_close_handler(std::bind(&on_close, &ws_server, std::placeholders::_1));

    // Listen on port 9002
    ws_server.listen(9002);

    // Start the server accept loop
    ws_server.start_accept();

    // Start the ASIO io_service run loop
    try {
        ws_server.run();
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    } catch (websocketpp::lib::error_code e) {
        std::cerr << "WebSocket++ Error: " << e.message() << std::endl;
    } catch (...) {
        std::cerr << "Unknown exception." << std::endl;
    }

    return 0;
}