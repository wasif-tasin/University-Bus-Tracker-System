#ifndef BUS_H
#define BUS_H

#include <string>

class Bus
{
private:
    std::string busID;
    std::string busName;
    std::string universityCode;
    int totalSeats;
    std::string route;

public:
    Bus();

    void setBusID(std::string id);
    void setBusName(std::string name);
    void setUniversityCode(std::string code);
    void setTotalSeats(int seats);
    void setRoute(std::string route);

    std::string getBusID();
    std::string getBusName();
    std::string getUniversityCode();
    int getTotalSeats();
    std::string getRoute();
};

#endif