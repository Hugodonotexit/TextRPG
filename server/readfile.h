#ifndef READFILE_H
#define READFILE_H
#include <fstream>
#include <map>
#include <string>
#include <set>
#include <mutex>
#include <sstream>
#include <vector>

class readfile
{
private:
    std::vector<std::string> split(const std::string& str, char delimiter);

public:
    bool check_username(const std::string& filename, const std::string& username);
    void write_user_to_csv(const std::string& filename, const std::string& username, const std::string& uuid);
    std::map<std::string, std::string> read_users_from_csv(const std::string& filename);
};


#endif