#include "Bus.h"

Bus::Bus()
{
    busID = "";
    busName = "";
    universityCode = "";
    totalSeats = 0;
    route = "";
}

void Bus::setBusID(std::string id)
{
    busID = id;
}

void Bus::setBusName(std::string name)
{
    busName = name;
}

void Bus::setUniversityCode(std::string code)
{
    universityCode = code;
}

void Bus::setTotalSeats(int seats)
{
    totalSeats = seats;
}

void Bus::setRoute(std::string r)
{
    route = r;
}

std::string Bus::getBusID() const
{
    return busID;
}

std::string Bus::getBusName() const
{
    return busName;
}

std::string Bus::getUniversityCode() const
{
    return universityCode;
}

int Bus::getTotalSeats() const
{
    return totalSeats;
}

std::string Bus::getRoute() const
{
    return route;
}
