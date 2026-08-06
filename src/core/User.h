#ifndef USER_H
#define USER_H

#include <string>
#include <vector>
#include <utility>
#include "Bus.h"

class User
{
public:
    void registerUser();
    bool registerUser(const std::string &email, const std::string &password, std::string &errorMsg);

    bool loginUser();
    bool loginUser(const std::string &email, const std::string &password, std::string &errorMsg);

    void dashboard();

    void viewUniversities();
    std::vector<std::pair<std::string, std::string>> getUniversities();

    void viewBuses();
    std::vector<Bus> getBuses();

    void selectUniversity();
    std::vector<Bus> getBusesForUniversity(const std::string &universityCode);

    void searchBus();
    std::vector<Bus> searchBus(const std::string &busID);

    void searchByStop();
    std::vector<Bus> searchByStop(const std::string &stopName);
};

#endif
