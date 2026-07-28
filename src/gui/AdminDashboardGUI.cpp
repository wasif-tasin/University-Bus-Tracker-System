#include "AdminDashboardGUI.h"
#include "Button.h"
#include "TextBox.h"
#include "Admin.h"

#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <string>

using namespace std;

static std::string wrapRoute(const std::string& route, float maxWidth, const sf::Font& font, unsigned int characterSize, const std::string& prefix = "Route: ")
{
    std::string wrapped = prefix;
    std::string indent(prefix.length(), ' ');
    std::string currentLine = "";
    sf::Text tempText(font);
    tempText.setCharacterSize(characterSize);


    std::vector<std::string> stops;
    std::string stop = "";
    for (char c : route)
    {
        if (c == ',')
        {
            if (!stop.empty())
            {
                stops.push_back(stop);
                stop = "";
            }
        }
        else
        {
            stop += c;
        }
    }
    if (!stop.empty()) stops.push_back(stop);

    for (size_t i = 0; i < stops.size(); ++i)
    {
        std::string s = stops[i];
        while (!s.empty() && s[0] == ' ') s = s.substr(1);
        while (!s.empty() && s.back() == ' ') s.pop_back();

        std::string testStop = s + (i < stops.size() - 1 ? ", " : "");
        tempText.setString(currentLine + testStop);
        if (tempText.getLocalBounds().size.x > maxWidth)
        {
            if (!currentLine.empty())
            {
                wrapped += currentLine + "\n" + indent;
                currentLine = testStop;
            }
            else
            {
                wrapped += testStop + "\n" + indent;
                currentLine = "";
            }
        }
        else
        {
            currentLine += testStop;
        }
    }
    wrapped += currentLine;
    return wrapped;
}

static std::string formatBusInfo(Bus bus, float maxWidth, const sf::Font& font, unsigned int characterSize, bool includeUni = true)
{
    std::string header = bus.getBusID() + " | " + bus.getBusName();
    if (includeUni)
    {
        header += " (" + bus.getUniversityCode() + ")";
    }
    header += " | Seats: " + std::to_string(bus.getTotalSeats());
    
    std::string routeWrapped = wrapRoute(bus.getRoute(), maxWidth, font, characterSize, "Route: ");
    
    return header + "\n" + routeWrapped;
}

enum AdminState
{
    DASHBOARD,
    ADD_UNIVERSITY,
    VIEW_UNIVERSITIES,
    DELETE_UNIVERSITY,
    ADD_BUS,
    VIEW_BUSES,
    DELETE_BUS
};

void AdminDashboardGUI::run()
{
    sf::RenderWindow window(
        sf::VideoMode({900, 700}),
        "Admin Dashboard");

    sf::Font font;
    if (!font.openFromFile("assets/Roboto-Regular.ttf"))
        return;

    Admin admin;

    sf::Text title(font);
    title.setString("ADMIN DASHBOARD");
    title.setCharacterSize(34);
    title.setFillColor(sf::Color::White);

    sf::Text infoMsg(font);
    infoMsg.setCharacterSize(20);
    infoMsg.setFillColor(sf::Color::Yellow);

    Button addUniBtn(font, "Add University", {300.f, 50.f}, {300.f, 120.f});
    Button viewUniBtn(font, "View University", {300.f, 50.f}, {300.f, 190.f});
    Button addBusBtn(font, "Add Bus", {300.f, 50.f}, {300.f, 260.f});
    Button viewBusBtn(font, "View Bus", {300.f, 50.f}, {300.f, 330.f});
    Button deleteBusBtn(font, "Delete Bus", {300.f, 50.f}, {300.f, 400.f});
    Button deleteUniBtn(font, "Delete University", {300.f, 50.f}, {300.f, 470.f});
    Button logoutBtn(font, "Logout", {300.f, 50.f}, {300.f, 540.f});

    sf::Text codeLabel(font);
    codeLabel.setString("University Code:");
    codeLabel.setCharacterSize(18);
    codeLabel.setFillColor(sf::Color::White);
    codeLabel.setPosition({200.f, 140.f});

    TextBox uniCodeBox(font, {500.f, 45.f}, {200.f, 170.f});

    sf::Text nameLabel(font);
    nameLabel.setString("University Name:");
    nameLabel.setCharacterSize(18);
    nameLabel.setFillColor(sf::Color::White);
    nameLabel.setPosition({200.f, 240.f});

    TextBox uniNameBox(font, {500.f, 45.f}, {200.f, 270.f});

    Button uniSubmit(font, "Add", {150.f, 50.f}, {200.f, 380.f});
    Button uniBack(font, "Back", {150.f, 50.f}, {400.f, 380.f});

    sf::Text delUniLabel(font);
    delUniLabel.setString("Enter University Code to Delete:");
    delUniLabel.setCharacterSize(18);
    delUniLabel.setFillColor(sf::Color::White);
    delUniLabel.setPosition({200.f, 160.f});

    TextBox delUniCodeBox(font, {500.f, 45.f}, {200.f, 190.f});

    Button delUniSubmit(font, "Delete", {150.f, 50.f}, {200.f, 300.f});
    Button delUniBack(font, "Back", {150.f, 50.f}, {400.f, 300.f});

    sf::Text bIdLabel(font); bIdLabel.setString("Bus ID:"); bIdLabel.setCharacterSize(18); bIdLabel.setFillColor(sf::Color::White); bIdLabel.setPosition({100.f, 120.f});
    TextBox busIdBox(font, {300.f, 45.f}, {100.f, 150.f});

    sf::Text bNameLabel(font); bNameLabel.setString("Bus Name:"); bNameLabel.setCharacterSize(18); bNameLabel.setFillColor(sf::Color::White); bNameLabel.setPosition({100.f, 210.f});
    TextBox busNameBox(font, {300.f, 45.f}, {100.f, 240.f});

    sf::Text bUniLabel(font); bUniLabel.setString("University Code:"); bUniLabel.setCharacterSize(18); bUniLabel.setFillColor(sf::Color::White); bUniLabel.setPosition({100.f, 300.f});
    TextBox busUniCodeBox(font, {300.f, 45.f}, {100.f, 330.f});

    sf::Text bSeatsLabel(font); bSeatsLabel.setString("Total Seats:"); bSeatsLabel.setCharacterSize(18); bSeatsLabel.setFillColor(sf::Color::White); bSeatsLabel.setPosition({500.f, 120.f});
    TextBox busSeatsBox(font, {300.f, 45.f}, {500.f, 150.f});

    sf::Text bRouteLabel(font); bRouteLabel.setString("Route Stops (comma-separated):"); bRouteLabel.setCharacterSize(18); bRouteLabel.setFillColor(sf::Color::White); bRouteLabel.setPosition({500.f, 210.f});
    TextBox busRouteBox(font, {300.f, 45.f}, {500.f, 240.f});

    Button busSubmit(font, "Add Bus", {150.f, 50.f}, {100.f, 440.f});
    Button busBack(font, "Back", {150.f, 50.f}, {280.f, 440.f});

    sf::Text delBusLabel(font);
    delBusLabel.setString("Enter Bus ID to Delete:");
    delBusLabel.setCharacterSize(18);
    delBusLabel.setFillColor(sf::Color::White);
    delBusLabel.setPosition({200.f, 160.f});

    TextBox delBusIdBox(font, {500.f, 45.f}, {200.f, 190.f});

    Button delBusSubmit(font, "Delete", {150.f, 50.f}, {200.f, 300.f});
    Button delBusBack(font, "Back", {150.f, 50.f}, {400.f, 300.f});

    Button prevPageBtn(font, "Prev", {100.f, 45.f}, {250.f, 600.f});
    Button nextPageBtn(font, "Next", {100.f, 45.f}, {550.f, 600.f});
    Button listBackBtn(font, "Back", {150.f, 50.f}, {375.f, 600.f});

    AdminState state = DASHBOARD;
    int focusedTextBox = 0; 
    int currentPage = 0;
    const int itemsPerPage = 10;
    const int busesPerPage = 6;

    vector<pair<string, string>> universitiesList;
    vector<Bus> busesList;

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();

            if (event->is<sf::Event::MouseButtonPressed>())
            {
                sf::Vector2i mouse = sf::Mouse::getPosition(window);

                if (state == DASHBOARD)
                {
                    if (addUniBtn.isClicked(window))
                    {
                        state = ADD_UNIVERSITY;
                        title.setString("ADD UNIVERSITY");
                        infoMsg.setString("");
                        focusedTextBox = 0;
                        uniCodeBox.setFocused(true);
                        uniNameBox.setFocused(false);
                    }
                    else if (viewUniBtn.isClicked(window))
                    {
                        state = VIEW_UNIVERSITIES;
                        title.setString("UNIVERSITY LIST");
                        universitiesList = admin.getUniversities();
                        currentPage = 0;
                    }
                    else if (addBusBtn.isClicked(window))
                    {
                        state = ADD_BUS;
                        title.setString("ADD BUS");
                        infoMsg.setString("");
                        focusedTextBox = 0;
                        busIdBox.setFocused(true);
                        busNameBox.setFocused(false);
                        busUniCodeBox.setFocused(false);
                        busSeatsBox.setFocused(false);
                        busRouteBox.setFocused(false);
                    }
                    else if (viewBusBtn.isClicked(window))
                    {
                        state = VIEW_BUSES;
                        title.setString("BUS LIST");
                        busesList = admin.getBuses();
                        currentPage = 0;
                    }
                    else if (deleteBusBtn.isClicked(window))
                    {
                        state = DELETE_BUS;
                        title.setString("DELETE BUS");
                        infoMsg.setString("");
                        delBusIdBox.setFocused(true);
                    }
                    else if (deleteUniBtn.isClicked(window))
                    {
                        state = DELETE_UNIVERSITY;
                        title.setString("DELETE UNIVERSITY");
                        infoMsg.setString("");
                        delUniCodeBox.setFocused(true);
                    }
                    else if (logoutBtn.isClicked(window))
                    {
                        window.close();
                    }
                }
                else if (state == ADD_UNIVERSITY)
                {
                    // Focus checks
                    if (mouse.x >= 200 && mouse.x <= 700)
                    {
                        if (mouse.y >= 170 && mouse.y <= 215)
                        {
                            focusedTextBox = 0;
                            uniCodeBox.setFocused(true);
                            uniNameBox.setFocused(false);
                        }
                        else if (mouse.y >= 270 && mouse.y <= 315)
                        {
                            focusedTextBox = 1;
                            uniCodeBox.setFocused(false);
                            uniNameBox.setFocused(true);
                        }
                    }

                    if (uniSubmit.isClicked(window))
                    {
                        string errMsg;
                        if (admin.addUniversity(uniCodeBox.getText(), uniNameBox.getText(), errMsg))
                        {
                            infoMsg.setFillColor(sf::Color::Green);
                            infoMsg.setString("University added successfully!");
                        }
                        else
                        {
                            infoMsg.setFillColor(sf::Color::Red);
                            infoMsg.setString("Error: " + errMsg);
                        }
                    }
                    else if (uniBack.isClicked(window))
                    {
                        state = DASHBOARD;
                        title.setString("ADMIN DASHBOARD");
                    }
                }
                else if (state == DELETE_UNIVERSITY)
                {
                    if (mouse.x >= 200 && mouse.x <= 700 && mouse.y >= 190 && mouse.y <= 235)
                        delUniCodeBox.setFocused(true);

                    if (delUniSubmit.isClicked(window))
                    {
                        string errMsg;
                        if (admin.deleteUniversity(delUniCodeBox.getText(), errMsg))
                        {
                            infoMsg.setFillColor(sf::Color::Green);
                            infoMsg.setString("University deleted successfully!");
                        }
                        else
                        {
                            infoMsg.setFillColor(sf::Color::Red);
                            infoMsg.setString("Error: " + errMsg);
                        }
                    }
                    else if (delUniBack.isClicked(window))
                    {
                        state = DASHBOARD;
                        title.setString("ADMIN DASHBOARD");
                    }
                }
                else if (state == ADD_BUS)
                {
                    if (mouse.x >= 100 && mouse.x <= 400)
                    {
                        if (mouse.y >= 150 && mouse.y <= 195) { focusedTextBox = 0; busIdBox.setFocused(true); busNameBox.setFocused(false); busUniCodeBox.setFocused(false); busSeatsBox.setFocused(false); busRouteBox.setFocused(false); }
                        else if (mouse.y >= 240 && mouse.y <= 285) { focusedTextBox = 1; busIdBox.setFocused(false); busNameBox.setFocused(true); busUniCodeBox.setFocused(false); busSeatsBox.setFocused(false); busRouteBox.setFocused(false); }
                        else if (mouse.y >= 330 && mouse.y <= 375) { focusedTextBox = 2; busIdBox.setFocused(false); busNameBox.setFocused(false); busUniCodeBox.setFocused(true); busSeatsBox.setFocused(false); busRouteBox.setFocused(false); }
                    }
                    else if (mouse.x >= 500 && mouse.x <= 800)
                    {
                        if (mouse.y >= 150 && mouse.y <= 195) { focusedTextBox = 3; busIdBox.setFocused(false); busNameBox.setFocused(false); busUniCodeBox.setFocused(false); busSeatsBox.setFocused(true); busRouteBox.setFocused(false); }
                        else if (mouse.y >= 240 && mouse.y <= 285) { focusedTextBox = 4; busIdBox.setFocused(false); busNameBox.setFocused(false); busUniCodeBox.setFocused(false); busSeatsBox.setFocused(false); busRouteBox.setFocused(true); }
                    }

                    if (busSubmit.isClicked(window))
                    {
                        string errMsg;
                        int seats = 0;
                        try {
                            seats = stoi(busSeatsBox.getText());
                        } catch(...) {
                            seats = -1;
                        }

                        if (admin.addBus(busIdBox.getText(), busNameBox.getText(), busUniCodeBox.getText(), seats, busRouteBox.getText(), errMsg))
                        {
                            infoMsg.setFillColor(sf::Color::Green);
                            infoMsg.setString("Bus added successfully!");
                        }
                        else
                        {
                            infoMsg.setFillColor(sf::Color::Red);
                            infoMsg.setString("Error: " + errMsg);
                        }
                    }
                    else if (busBack.isClicked(window))
                    {
                        state = DASHBOARD;
                        title.setString("ADMIN DASHBOARD");
                    }
                }
                else if (state == DELETE_BUS)
                {
                    if (mouse.x >= 200 && mouse.x <= 700 && mouse.y >= 190 && mouse.y <= 235)
                        delBusIdBox.setFocused(true);

                    if (delBusSubmit.isClicked(window))
                    {
                        string errMsg;
                        if (admin.deleteBus(delBusIdBox.getText(), errMsg))
                        {
                            infoMsg.setFillColor(sf::Color::Green);
                            infoMsg.setString("Bus deleted successfully!");
                        }
                        else
                        {
                            infoMsg.setFillColor(sf::Color::Red);
                            infoMsg.setString("Error: " + errMsg);
                        }
                    }
                    else if (delBusBack.isClicked(window))
                    {
                        state = DASHBOARD;
                        title.setString("ADMIN DASHBOARD");
                    }
                }
                else if (state == VIEW_UNIVERSITIES)
                {
                    if (listBackBtn.isClicked(window))
                    {
                        state = DASHBOARD;
                        title.setString("ADMIN DASHBOARD");
                    }
                    else if (currentPage > 0 && prevPageBtn.isClicked(window))
                    {
                        currentPage--;
                    }
                    else if ((currentPage + 1) * itemsPerPage < universitiesList.size() && nextPageBtn.isClicked(window))
                    {
                        currentPage++;
                    }
                }
                else if (state == VIEW_BUSES)
                {
                    if (listBackBtn.isClicked(window))
                    {
                        state = DASHBOARD;
                        title.setString("ADMIN DASHBOARD");
                    }
                    else if (currentPage > 0 && prevPageBtn.isClicked(window))
                    {
                        currentPage--;
                    }
                    else if ((currentPage + 1) * busesPerPage < busesList.size() && nextPageBtn.isClicked(window))
                    {
                        currentPage++;
                    }
                }
            }

            if (event->is<sf::Event::TextEntered>())
            {
                if (state == ADD_UNIVERSITY)
                {
                    if (focusedTextBox == 0) uniCodeBox.handleEvent(*event);
                    else if (focusedTextBox == 1) uniNameBox.handleEvent(*event);
                }
                else if (state == DELETE_UNIVERSITY)
                {
                    delUniCodeBox.handleEvent(*event);
                }
                else if (state == ADD_BUS)
                {
                    if (focusedTextBox == 0) busIdBox.handleEvent(*event);
                    else if (focusedTextBox == 1) busNameBox.handleEvent(*event);
                    else if (focusedTextBox == 2) busUniCodeBox.handleEvent(*event);
                    else if (focusedTextBox == 3) busSeatsBox.handleEvent(*event);
                    else if (focusedTextBox == 4) busRouteBox.handleEvent(*event);
                }
                else if (state == DELETE_BUS)
                {
                    delBusIdBox.handleEvent(*event);
                }
            }
        }

        if (state == DASHBOARD)
        {
            addUniBtn.update(window);
            viewUniBtn.update(window);
            addBusBtn.update(window);
            viewBusBtn.update(window);
            deleteBusBtn.update(window);
            deleteUniBtn.update(window);
            logoutBtn.update(window);
        }
        else if (state == ADD_UNIVERSITY)
        {
            uniSubmit.update(window);
            uniBack.update(window);
        }
        else if (state == DELETE_UNIVERSITY)
        {
            delUniSubmit.update(window);
            delUniBack.update(window);
        }
        else if (state == ADD_BUS)
        {
            busSubmit.update(window);
            busBack.update(window);
        }
        else if (state == DELETE_BUS)
        {
            delBusSubmit.update(window);
            delBusBack.update(window);
        }
        else if (state == VIEW_UNIVERSITIES || state == VIEW_BUSES)
        {
            listBackBtn.update(window);
            prevPageBtn.update(window);
            nextPageBtn.update(window);
        }

        window.clear(sf::Color(35, 45, 70));

        sf::FloatRect titleBounds = title.getLocalBounds();
        title.setOrigin({titleBounds.position.x + titleBounds.size.x / 2.f, 0.f});
        title.setPosition({450.f, 40.f}); // 900.f / 2 = 450.f
        window.draw(title);

        if (state == DASHBOARD)
        {
            addUniBtn.draw(window);
            viewUniBtn.draw(window);
            addBusBtn.draw(window);
            viewBusBtn.draw(window);
            deleteBusBtn.draw(window);
            deleteUniBtn.draw(window);
            logoutBtn.draw(window);
        }
        else if (state == ADD_UNIVERSITY)
        {
            window.draw(codeLabel);
            uniCodeBox.draw(window);

            window.draw(nameLabel);
            uniNameBox.draw(window);

            uniSubmit.draw(window);
            uniBack.draw(window);

            if (!infoMsg.getString().isEmpty())
            {
                infoMsg.setPosition({200.f, 340.f});
                window.draw(infoMsg);
            }
        }
        else if (state == DELETE_UNIVERSITY)
        {
            window.draw(delUniLabel);
            delUniCodeBox.draw(window);

            delUniSubmit.draw(window);
            delUniBack.draw(window);

            if (!infoMsg.getString().isEmpty())
            {
                infoMsg.setPosition({200.f, 260.f});
                window.draw(infoMsg);
            }
        }
        else if (state == ADD_BUS)
        {
            window.draw(bIdLabel); busIdBox.draw(window);
            window.draw(bNameLabel); busNameBox.draw(window);
            window.draw(bUniLabel); busUniCodeBox.draw(window);
            window.draw(bSeatsLabel); busSeatsBox.draw(window);
            window.draw(bRouteLabel); busRouteBox.draw(window);

            busSubmit.draw(window);
            busBack.draw(window);

            if (!infoMsg.getString().isEmpty())
            {
                infoMsg.setPosition({100.f, 400.f});
                window.draw(infoMsg);
            }
        }
        else if (state == DELETE_BUS)
        {
            window.draw(delBusLabel);
            delBusIdBox.draw(window);

            delBusSubmit.draw(window);
            delBusBack.draw(window);

            if (!infoMsg.getString().isEmpty())
            {
                infoMsg.setPosition({200.f, 260.f});
                window.draw(infoMsg);
            }
        }
        else if (state == VIEW_UNIVERSITIES)
        {
            listBackBtn.draw(window);

            if (universitiesList.empty())
            {
                sf::Text emptyTxt(font, "No Universities Registered.", 24);
                emptyTxt.setFillColor(sf::Color::Yellow);
                emptyTxt.setPosition({300.f, 200.f});
                window.draw(emptyTxt);
            }
            else
            {
                int startIdx = currentPage * itemsPerPage;
                int endIdx = min(startIdx + itemsPerPage, (int)universitiesList.size());
                float currentY = 120.f;

                for (int i = startIdx; i < endIdx; ++i)
                {
                    sf::Text row(font, universitiesList[i].first + "  -  " + universitiesList[i].second, 22);
                    row.setFillColor(sf::Color::White);
                    row.setPosition({150.f, currentY});
                    window.draw(row);
                    currentY += 40.f;
                }

                if (currentPage > 0)
                    prevPageBtn.draw(window);
                if (endIdx < (int)universitiesList.size())
                    nextPageBtn.draw(window);
            }
        }
        else if (state == VIEW_BUSES)
        {
            listBackBtn.draw(window);

            if (busesList.empty())
            {
                sf::Text emptyTxt(font, "No Buses Registered.", 24);
                emptyTxt.setFillColor(sf::Color::Yellow);
                emptyTxt.setPosition({300.f, 200.f});
                window.draw(emptyTxt);
            }
            else
            {
                int startIdx = currentPage * busesPerPage;
                int endIdx = min(startIdx + busesPerPage, (int)busesList.size());
                float currentY = 120.f;

                for (int i = startIdx; i < endIdx; ++i)
                {
                    string info = formatBusInfo(busesList[i], 780.f, font, 18, true);
                    sf::Text row(font, info, 18);
                    row.setFillColor(sf::Color::White);
                    row.setPosition({80.f, currentY});
                    window.draw(row);
                    currentY += 65.f;
                }

                if (currentPage > 0)
                    prevPageBtn.draw(window);
                if (endIdx < (int)busesList.size())
                    nextPageBtn.draw(window);
            }
        }

        window.display();
    }
}