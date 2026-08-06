#include "User.h"

#include <iostream>
#include <fstream>
#include <string>

using namespace std;

void User::registerUser()
{
    string email, password;

    cout << "\n========== USER REGISTRATION ==========\n";

    cin.ignore();

    cout << "Enter Gmail : ";
    getline(cin, email);

    if (email.size() <= 10 ||
        email.substr(email.size() - 10) != "@gmail.com")
    {
        cout << "\nInvalid Gmail Address!\n";
        return;
    }

    cout << "Enter Password : ";
    getline(cin, password);
    ifstream checkFile("data/users.txt");

    string existingEmail, existingPassword;

    while (getline(checkFile, existingEmail, '|') &&
           getline(checkFile, existingPassword))
    {
        if (existingEmail == email)
        {
            cout << "\nThis Gmail is already registered!\n";
            checkFile.close();
            return;
        }
    }

    checkFile.close();
    ofstream file("data/users.txt", ios::app);

    file << email << "|" << password << endl;

    file.close();

    cout << "\nRegistration Successful!\n";
}

bool User::loginUser()
{
    string email, password;

    cout << "\n========== USER LOGIN ==========\n";

    cin.ignore();

    cout << "Enter Gmail : ";
    getline(cin, email);

    cout << "Enter Password : ";
    getline(cin, password);

    ifstream file("data/users.txt");

    string savedEmail, savedPassword;

    while (getline(file, savedEmail, '|') &&
           getline(file, savedPassword))
    {
        if (savedEmail == email && savedPassword == password)
        {
            cout << "\nLogin Successful!\n";
            file.close();
            return true;
        }
    }

    file.close();

    cout << "\nInvalid Gmail or Password!\n";

    return false;
}
void User::dashboard()
{
    int choice;

    while (true)
    {
        cout << "\n==================================\n";
        cout << "         USER DASHBOARD\n";
        cout << "==================================\n";

        cout << "1. View Universities\n";
        cout << "2. Select University\n";
        cout << "3. View All Buses\n";
        cout << "4. Search Bus\n";
        cout << "5. Search By Stop\n";
        cout << "6. Logout\n";

        cout << "\nEnter Choice : ";

        cin >> choice;

        switch (choice)
        {
        case 1:
            viewUniversities();
            break;

        case 2:
            selectUniversity();
            break;

        case 3:
            viewBuses();
            break;

        case 4:
            searchBus();
            break;

        case 5:
            searchByStop();
            break;

        case 6:
            cout << "\nLogged Out Successfully.\n";
            return;

        default:
            cout << "\nInvalid Choice!\n";
        }
    }
}
void User::viewUniversities()
{
    ifstream file("data/universities.txt");

    string code, name;

    cout << "\n========== UNIVERSITIES ==========\n";

    while (getline(file, code, '|') &&
           getline(file, name))
    {
        cout << code << " - " << name << endl;
    }

    file.close();
}
void User::viewBuses()
{
    ifstream file("data/buses.txt");

    string id, name, university, seats, route;

    cout << "\n============= BUS LIST =============\n";

    while (getline(file, id, '|') &&
           getline(file, name, '|') &&
           getline(file, university, '|') &&
           getline(file, seats, '|') &&
           getline(file, route))
    {
        cout << "\nBus ID      : " << id << endl;
        cout << "Bus Name    : " << name << endl;
        cout << "University  : " << university << endl;
        cout << "Seats       : " << seats << endl;
        cout << "Route       : " << route << endl;
    }

    file.close();
}
void User::selectUniversity()
{
    string code;

    cout << "\n========== SELECT UNIVERSITY ==========\n";

    cin.ignore();

    cout << "Enter University Code : ";
    getline(cin, code);

    ifstream file("data/buses.txt");

    string id;
    string name;
    string university;
    string seats;
    string route;

    bool found = false;

    cout << "\n========== AVAILABLE BUSES ==========\n";

    while (getline(file, id, '|') &&
           getline(file, name, '|') &&
           getline(file, university, '|') &&
           getline(file, seats, '|') &&
           getline(file, route))
    {
        if (university == code)
        {
            found = true;

            cout << "\nBus ID : " << id << endl;
            cout << "Bus Name : " << name << endl;
            cout << "Seats : " << seats << endl;
            cout << "Route : " << route << endl;
        }
    }

    if (!found)
    {
        cout << "\nNo Bus Found For This University.\n";
    }

    file.close();
}
void User::searchBus()
{
    string searchID;

    cout << "\n========== SEARCH BUS ==========\n";

    cin.ignore();

    cout << "Enter Bus ID : ";
    getline(cin, searchID);

    ifstream file("data/buses.txt");

    string id,name,university,seats,route;

    bool found = false;

    while(getline(file,id,'|') &&
          getline(file,name,'|') &&
          getline(file,university,'|') &&
          getline(file,seats,'|') &&
          getline(file,route))
    {
        if(id == searchID)
        {
            found = true;

            cout << "\nBus ID : " << id << endl;
            cout << "Bus Name : " << name << endl;
            cout << "University : " << university << endl;
            cout << "Seats : " << seats << endl;
            cout << "Route : " << route << endl;

            break;
        }
    }

    if(!found)
    {
        cout << "\nBus Not Found.\n";
    }

    file.close();
}
void User::searchByStop()
{
    string stop;

    cout << "\n========== SEARCH BY STOP ==========\n";

    cin.ignore();

    cout << "Enter Stop Name : ";
    getline(cin, stop);

    ifstream file("data/buses.txt");

    string id,name,university,seats,route;

    bool found = false;

    while(getline(file,id,'|') &&
          getline(file,name,'|') &&
          getline(file,university,'|') &&
          getline(file,seats,'|') &&
          getline(file,route))
    {
        if(route.find(stop) != string::npos)
        {
            found = true;

            cout << "\nBus ID : " << id << endl;
            cout << "Bus Name : " << name << endl;
            cout << "University : " << university << endl;
        }
    }

    if(!found)
    {
        cout << "\nNo Bus Found For This Stop.\n";
    }

    file.close();
}

bool User::registerUser(const string& email, const string& password, string& errorMsg)
{
    if (email.size() <= 10 || email.substr(email.size() - 10) != "@gmail.com")
    {
        errorMsg = "Invalid Gmail Address! Must end with @gmail.com";
        return false;
    }
    if (password.empty())
    {
        errorMsg = "Password cannot be empty!";
        return false;
    }

    ifstream checkFile("data/users.txt");
    string existingEmail, existingPassword;
    while (getline(checkFile, existingEmail, '|') && getline(checkFile, existingPassword))
    {
        while (!existingPassword.empty() && (existingPassword.back() == '\r' || existingPassword.back() == '\n'))
            existingPassword.pop_back();

        if (existingEmail == email)
        {
            errorMsg = "This Gmail is already registered!";
            checkFile.close();
            return false;
        }
    }
    checkFile.close();

    ofstream file("data/users.txt", ios::app);
    if (!file)
    {
        errorMsg = "Error opening users.txt";
        return false;
    }
    file << email << "|" << password << endl;
    file.close();
    return true;
}

bool User::loginUser(const string& email, const string& password, string& errorMsg)
{
    ifstream file("data/users.txt");
    if (!file)
    {
        errorMsg = "No users registered yet!";
        return false;
    }
    string savedEmail, savedPassword;
    while (getline(file, savedEmail, '|') && getline(file, savedPassword))
    {
        while (!savedPassword.empty() && (savedPassword.back() == '\r' || savedPassword.back() == '\n'))
            savedPassword.pop_back();

        if (savedEmail == email && savedPassword == password)
        {
            file.close();
            return true;
        }
    }
    file.close();
    errorMsg = "Invalid Gmail or Password!";
    return false;
}

std::vector<std::pair<std::string, std::string>> User::getUniversities()
{
    std::vector<std::pair<std::string, std::string>> list;
    ifstream file("data/universities.txt");
    if (!file)
        return list;

    string code, name;
    while (getline(file, code, '|') && getline(file, name))
    {
        while (!name.empty() && (name.back() == '\r' || name.back() == '\n'))
            name.pop_back();
        list.push_back({code, name});
    }
    file.close();
    return list;
}

std::vector<Bus> User::getBuses()
{
    std::vector<Bus> list;
    ifstream file("data/buses.txt");
    if (!file)
        return list;

    string id, name, university, seatsStr, route;
    while (getline(file, id, '|') &&
           getline(file, name, '|') &&
           getline(file, university, '|') &&
           getline(file, seatsStr, '|') &&
           getline(file, route))
    {
        while (!route.empty() && (route.back() == '\r' || route.back() == '\n'))
            route.pop_back();

        Bus b;
        b.setBusID(id);
        b.setBusName(name);
        b.setUniversityCode(university);
        try {
            b.setTotalSeats(std::stoi(seatsStr));
        } catch(...) {
            b.setTotalSeats(0);
        }
        b.setRoute(route);
        list.push_back(b);
    }
    file.close();
    return list;
}

std::vector<Bus> User::getBusesForUniversity(const string& universityCode)
{
    std::vector<Bus> list;
    ifstream file("data/buses.txt");
    if (!file)
        return list;

    string id, name, university, seatsStr, route;
    while (getline(file, id, '|') &&
           getline(file, name, '|') &&
           getline(file, university, '|') &&
           getline(file, seatsStr, '|') &&
           getline(file, route))
    {
        while (!route.empty() && (route.back() == '\r' || route.back() == '\n'))
            route.pop_back();

        if (university == universityCode)
        {
            Bus b;
            b.setBusID(id);
            b.setBusName(name);
            b.setUniversityCode(university);
            try {
                b.setTotalSeats(std::stoi(seatsStr));
            } catch(...) {
                b.setTotalSeats(0);
            }
            b.setRoute(route);
            list.push_back(b);
        }
    }
    file.close();
    return list;
}

std::vector<Bus> User::searchBus(const string& busID)
{
    std::vector<Bus> list;
    ifstream file("data/buses.txt");
    if (!file)
        return list;

    string id, name, university, seatsStr, route;
    while (getline(file, id, '|') &&
           getline(file, name, '|') &&
           getline(file, university, '|') &&
           getline(file, seatsStr, '|') &&
           getline(file, route))
    {
        while (!route.empty() && (route.back() == '\r' || route.back() == '\n'))
            route.pop_back();

        if (id == busID)
        {
            Bus b;
            b.setBusID(id);
            b.setBusName(name);
            b.setUniversityCode(university);
            try {
                b.setTotalSeats(std::stoi(seatsStr));
            } catch(...) {
                b.setTotalSeats(0);
            }
            b.setRoute(route);
            list.push_back(b);
            break;
        }
    }
    file.close();
    return list;
}

std::vector<Bus> User::searchByStop(const string& stopName)
{
    std::vector<Bus> list;
    ifstream file("data/buses.txt");
    if (!file)
        return list;

    string id, name, university, seatsStr, route;
    while (getline(file, id, '|') &&
           getline(file, name, '|') &&
           getline(file, university, '|') &&
           getline(file, seatsStr, '|') &&
           getline(file, route))
    {
        while (!route.empty() && (route.back() == '\r' || route.back() == '\n'))
            route.pop_back();

        if (route.find(stopName) != string::npos)
        {
            Bus b;
            b.setBusID(id);
            b.setBusName(name);
            b.setUniversityCode(university);
            try {
                b.setTotalSeats(std::stoi(seatsStr));
            } catch(...) {
                b.setTotalSeats(0);
            }
            b.setRoute(route);
            list.push_back(b);
        }
    }
    file.close();
    return list;
}
