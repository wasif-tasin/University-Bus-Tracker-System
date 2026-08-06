#include "Admin.h"
#include <cstdio>

#include <iostream>
#include <fstream>

using namespace std;

Admin::Admin()
{
    username = "";
    password = "";
}

bool Admin::login()
{
    cout << "\n========== ADMIN LOGIN ==========\n";

    cout << "Username : ";
    cin >> username;

    cout << "Password : ";
    cin >> password;

    ifstream file("data/admin.txt");

    if (!file)
    {
        cout << "\nError! admin.txt not found.\n";
        return false;
    }

    string fileUsername;
    string filePassword;

    getline(file, fileUsername, '|');
    getline(file, filePassword);

    file.close();

    if (username == fileUsername && password == filePassword)
    {
        cout << "\nLogin Successful!\n";
        return true;
    }

    cout << "\nInvalid Username or Password!\n";

    return false;
}

void Admin::dashboard()
{
    int choice;

    while (true)
    {
        cout << "\n==============================\n";
        cout << " UNIVERSITY BUS TRACKER\n";
        cout << "==============================\n";

        cout << "1. Add University\n";
        cout << "2. Add Bus\n";
        cout << "3. View Universities\n";
        cout << "4. View Buses\n";
        cout << "5. Delete University\n";
        cout << "6. Delete Bus\n";
        cout << "7. Logout\n";

        cout << "\nEnter Choice : ";
        cin >> choice;

        switch (choice)
        {
        case 1:
            addUniversity();
            break;

        case 2:
            addBus();
            break;

        case 3:
            viewUniversities();
            break;

        case 4:
            viewBuses();
            break;

        case 5:
            deleteUniversity();
            break;

        case 6:
            deleteBus();
            break;

        case 7:
            cout << "\nLogging Out...\n";
            return;

        default:
            cout << "\nInvalid Choice!\n";
        }
    }
}
void Admin::addUniversity()
{
    string code, name;

    cout << "\n========== ADD UNIVERSITY ==========\n";

    cin.ignore();

    cout << "University Code : ";
    getline(cin, code);

    cout << "University Name : ";
    getline(cin, name);
    ifstream checkFile("data/universities.txt");

    string existingCode, existingName;

    while (getline(checkFile, existingCode, '|') &&
           getline(checkFile, existingName))
    {
        if (existingCode == code)
        {
            cout << "\nUniversity Code already exists!\n";
            checkFile.close();
            return;
        }
    }

    checkFile.close();

    ofstream file("data/universities.txt", ios::app);

    if (!file)
    {
        cout << "\nError opening universities.txt\n";
        return;
    }

    file << code << "|" << name << endl;

    file.close();

    cout << "\nUniversity Added Successfully!\n";
}
void Admin::viewUniversities()
{
    ifstream file("data/universities.txt");

    if (!file)
    {
        cout << "\nUnable to open universities.txt\n";
        return;
    }

    string code, name;

    cout << "\n=============================================\n";
    cout << "          UNIVERSITY LIST\n";
    cout << "=============================================\n";

    int count = 0;

    while (getline(file, code, '|') && getline(file, name))
    {
        count++;

        cout << count << ". ";
        cout << "Code : " << code;
        cout << " | Name : " << name << endl;
    }

    if (count == 0)
    {
        cout << "\nNo University Found.\n";
    }

    file.close();
}
void Admin::addBus()
{
    string id, name, university, route;
    int seats;

    cout << "\n========== ADD BUS ==========\n";

    cin.ignore();

    cout << "Bus ID : ";
    getline(cin, id);

    ifstream checkFile("data/buses.txt");

    string busID;

    while (getline(checkFile, busID, '|'))
    {
        if (busID == id)
        {
            cout << "\nBus ID already exists!\n";
            checkFile.close();
            return;
        }

        string temp;
        getline(checkFile, temp);
    }

    checkFile.close();

    cout << "Bus Name : ";
    getline(cin, name);

    cout << "University Code : ";
    getline(cin, university);

    cout << "Total Seats : ";
    cin >> seats;

    cin.ignore();

    cout << "Route : ";
    getline(cin, route);

    ofstream file("data/buses.txt", ios::app);

    if (!file)
    {
        cout << "\nError opening buses.txt\n";
        return;
    }

    file << id << "|"
         << name << "|"
         << university << "|"
         << seats << "|"
         << route << endl;

    file.close();

    cout << "\nBus Added Successfully.\n";
}
void Admin::viewBuses()
{
    ifstream file("data/buses.txt");

    if (!file)
    {
        cout << "\nUnable to open buses.txt\n";
        return;
    }

    string id;
    string name;
    string university;
    string seats;
    string route;

    cout << "\n==============================================\n";
    cout << "              BUS LIST\n";
    cout << "==============================================\n";

    int count = 0;

    while (getline(file, id, '|') &&
           getline(file, name, '|') &&
           getline(file, university, '|') &&
           getline(file, seats, '|') &&
           getline(file, route))
    {
        count++;

        cout << "\nBus " << count << endl;
        cout << "ID         : " << id << endl;
        cout << "Name       : " << name << endl;
        cout << "University : " << university << endl;
        cout << "Seats      : " << seats << endl;
        cout << "Route      : " << route << endl;
    }

    if (count == 0)
    {
        cout << "\nNo Bus Found.\n";
    }

    file.close();
}
void Admin::deleteUniversity()
{
    string deleteCode;

    cout << "\n========== DELETE UNIVERSITY ==========\n";

    cin.ignore();

    cout << "Enter University Code : ";
    getline(cin, deleteCode);

    ifstream file("data/universities.txt");
    ofstream temp("data/temp.txt");

    string code, name;
    bool found = false;

    while (getline(file, code, '|') &&
           getline(file, name))
    {
        if (code == deleteCode)
        {
            found = true;
            continue;
        }

        temp << code << "|" << name << endl;
    }

    file.close();
    temp.close();

    remove("data/universities.txt");
    rename("data/temp.txt", "data/universities.txt");

    if (found)
        cout << "\nUniversity Deleted Successfully.\n";
    else
        cout << "\nUniversity Not Found.\n";
}
void Admin::deleteBus()
{
    string deleteID;

    cout << "\n========== DELETE BUS ==========\n";

    cin.ignore();

    cout << "Enter Bus ID : ";
    getline(cin, deleteID);

    ifstream file("data/buses.txt");
    ofstream temp("data/temp.txt");

    string id, name, university, seats, route;

    bool found = false;

    while (getline(file, id, '|') &&
           getline(file, name, '|') &&
           getline(file, university, '|') &&
           getline(file, seats, '|') &&
           getline(file, route))
    {
        if (id == deleteID)
        {
            found = true;
            continue;
        }

        temp << id << "|"
             << name << "|"
             << university << "|"
             << seats << "|"
             << route << endl;
    }

    file.close();
    temp.close();

    remove("data/buses.txt");
    rename("data/temp.txt", "data/buses.txt");

    if (found)
        cout << "\nBus Deleted Successfully.\n";
    else
        cout << "\nBus Not Found.\n";
}

bool Admin::login(const string &user, const string &pass)
{
    ifstream file("data/admin.txt");
    if (!file)
        return false;

    string fileUsername;
    string filePassword;

    getline(file, fileUsername, '|');
    getline(file, filePassword);
    file.close();

    while (!filePassword.empty() && (filePassword.back() == '\r' || filePassword.back() == '\n' || filePassword.back() == ' '))
        filePassword.pop_back();
    while (!fileUsername.empty() && (fileUsername.back() == '\r' || fileUsername.back() == '\n' || fileUsername.back() == ' '))
        fileUsername.pop_back();

    return (user == fileUsername && pass == filePassword);
}

bool Admin::addUniversity(const string &code, const string &name, string &errorMsg)
{
    if (code.empty() || name.empty())
    {
        errorMsg = "Fields cannot be empty!";
        return false;
    }

    ifstream checkFile("data/universities.txt");
    string existingCode, existingName;
    while (getline(checkFile, existingCode, '|') && getline(checkFile, existingName))
    {
        while (!existingName.empty() && (existingName.back() == '\r' || existingName.back() == '\n'))
            existingName.pop_back();

        if (existingCode == code)
        {
            errorMsg = "University Code already exists!";
            checkFile.close();
            return false;
        }
    }
    checkFile.close();

    ofstream file("data/universities.txt", ios::app);
    if (!file)
    {
        errorMsg = "Error opening universities.txt";
        return false;
    }

    file << code << "|" << name << endl;
    file.close();
    return true;
}

std::vector<std::pair<std::string, std::string>> Admin::getUniversities()
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

bool Admin::addBus(const string &id, const string &name, const string &university, int seats, const string &route, string &errorMsg)
{
    if (id.empty() || name.empty() || university.empty() || route.empty())
    {
        errorMsg = "Fields cannot be empty!";
        return false;
    }
    if (seats <= 0)
    {
        errorMsg = "Seats must be greater than 0!";
        return false;
    }

    ifstream checkUni("data/universities.txt");
    bool uniExists = false;
    string uCode, uName;
    while (getline(checkUni, uCode, '|') && getline(checkUni, uName))
    {
        if (uCode == university)
        {
            uniExists = true;
            break;
        }
    }
    checkUni.close();

    if (!uniExists)
    {
        errorMsg = "University Code does not exist!";
        return false;
    }

    ifstream checkFile("data/buses.txt");
    string busID;
    while (getline(checkFile, busID, '|'))
    {
        if (busID == id)
        {
            errorMsg = "Bus ID already exists!";
            checkFile.close();
            return false;
        }
        string temp;
        getline(checkFile, temp);
    }
    checkFile.close();

    ofstream file("data/buses.txt", ios::app);
    if (!file)
    {
        errorMsg = "Error opening buses.txt";
        return false;
    }

    file << id << "|"
         << name << "|"
         << university << "|"
         << seats << "|"
         << route << endl;

    file.close();
    return true;
}

std::vector<Bus> Admin::getBuses()
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
        try
        {
            b.setTotalSeats(std::stoi(seatsStr));
        }
        catch (...)
        {
            b.setTotalSeats(0);
        }
        b.setRoute(route);
        list.push_back(b);
    }
    file.close();
    return list;
}

bool Admin::deleteUniversity(const string &code, string &errorMsg)
{
    if (code.empty())
    {
        errorMsg = "University code cannot be empty!";
        return false;
    }

    ifstream file("data/universities.txt");
    if (!file)
    {
        errorMsg = "Error opening universities.txt";
        return false;
    }
    ofstream temp("data/temp.txt");

    string existingCode, name;
    bool found = false;

    while (getline(file, existingCode, '|') && getline(file, name))
    {
        while (!name.empty() && (name.back() == '\r' || name.back() == '\n'))
            name.pop_back();

        if (existingCode == code)
        {
            found = true;
            continue;
        }

        temp << existingCode << "|" << name << endl;
    }

    file.close();
    temp.close();

    remove("data/universities.txt");
    rename("data/temp.txt", "data/universities.txt");

    if (found)
    {
        return true;
    }
    else
    {
        errorMsg = "University not found!";
        return false;
    }
}

bool Admin::deleteBus(const string &id, string &errorMsg)
{
    if (id.empty())
    {
        errorMsg = "Bus ID cannot be empty!";
        return false;
    }

    ifstream file("data/buses.txt");
    if (!file)
    {
        errorMsg = "Error opening buses.txt";
        return false;
    }
    ofstream temp("data/temp.txt");

    string busID, name, university, seats, route;
    bool found = false;

    while (getline(file, busID, '|') &&
           getline(file, name, '|') &&
           getline(file, university, '|') &&
           getline(file, seats, '|') &&
           getline(file, route))
    {
        while (!route.empty() && (route.back() == '\r' || route.back() == '\n'))
            route.pop_back();

        if (busID == id)
        {
            found = true;
            continue;
        }

        temp << busID << "|"
             << name << "|"
             << university << "|"
             << seats << "|"
             << route << endl;
    }

    file.close();
    temp.close();

    remove("data/buses.txt");
    rename("data/temp.txt", "data/buses.txt");

    if (found)
    {
        return true;
    }
    else
    {
        errorMsg = "Bus not found!";
        return false;
    }
}
