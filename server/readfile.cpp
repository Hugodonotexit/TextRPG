#include "readfile.h"

std::vector<std::string> readfile::split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::istringstream ss(str);
    std::string token;

    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens;
}


bool readfile::check_username(const std::string& filename, const std::string& username) {
    std::ifstream file(filename);
    std::string line;

    while (std::getline(file, line)) {
        std::vector<std::string> tokens = split(line, ',');
        if (!tokens.empty() && tokens[0] == username) {
            return true;
        }
    }

    return false;
}

// Function to write user details to a CSV file
void readfile::write_user_to_csv(const std::string& filename, const std::string& username, const std::string& uuid) {
    std::ofstream file(filename, std::ios::app);
    file << username << "," << uuid << "\n";
}

// Function to read user details from a CSV file
std::map<std::string, std::string> readfile::read_users_from_csv(const std::string& filename) {
    std::map<std::string, std::string> user_data;
    std::ifstream file(filename);
    std::string line;

    while (std::getline(file, line)) {
        std::istringstream line_stream(line);
        std::string username, uuid;

        std::getline(line_stream, username, ',');
        std::getline(line_stream, uuid, ',');

        user_data[username] = uuid;
    }

    return user_data;
}