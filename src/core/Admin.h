#ifndef ADMIN_H
#define ADMIN_H

#include <string>
#include <vector>
#include <utility>
#include "Bus.h"

class Admin
{
private:
    std::string username;
    std::string password;

public:
    Admin();

    bool login();
    bool login(const std::string& user, const std::string& pass);
    void dashboard();

    void addUniversity();
    bool addUniversity(const std::string& code, const std::string& name, std::string& errorMsg);

    void viewUniversities();
    std::vector<std::pair<std::string, std::string>> getUniversities();

    void addBus();
    bool addBus(const std::string& id, const std::string& name, const std::string& university, int seats, const std::string& route, std::string& errorMsg);

    void viewBuses();
    std::vector<Bus> getBuses();

    void deleteUniversity();
    bool deleteUniversity(const std::string& code, std::string& errorMsg);

    void deleteBus();
    bool deleteBus(const std::string& id, std::string& errorMsg);
};

#endif