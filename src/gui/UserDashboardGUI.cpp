#include "UserDashboardGUI.h"
#include "Button.h"
#include "TextBox.h"
#include "User.h"
#include "Bus.h"

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

enum UserState
{
    USER_DASHBOARD,
    USER_VIEW_UNIVERSITIES,
    USER_SELECT_UNIVERSITY,
    USER_VIEW_BUSES,
    USER_SEARCH_BUS,
    USER_SEARCH_BY_STOP
};

void UserDashboardGUI::run()
{
    sf::RenderWindow window(
        sf::VideoMode({900, 700}),
        "User Dashboard");

    sf::Font font;
    if (!font.openFromFile("assets/Roboto-Regular.ttf"))
        return;

    User user;

    sf::Text title(font);
    title.setString("USER DASHBOARD");
    title.setCharacterSize(34);
    title.setFillColor(sf::Color::White);

    sf::Text infoMsg(font);
    infoMsg.setCharacterSize(20);
    infoMsg.setFillColor(sf::Color::Yellow);

    Button viewUniBtn(font, "View Universities", {300.f, 50.f}, {300.f, 120.f});
    Button selectUniBtn(font, "Select University", {300.f, 50.f}, {300.f, 190.f});
    Button viewAllBusesBtn(font, "View All Buses", {300.f, 50.f}, {300.f, 260.f});
    Button searchBusBtn(font, "Search Bus by ID", {300.f, 50.f}, {300.f, 330.f});
    Button searchStopBtn(font, "Search by Stop Name", {300.f, 50.f}, {300.f, 400.f});
    Button logoutBtn(font, "Logout", {300.f, 50.f}, {300.f, 470.f});

    Button backBtn(font, "Back", {150.f, 50.f}, {375.f, 600.f});
    Button prevPageBtn(font, "Prev", {100.f, 45.f}, {250.f, 600.f});
    Button nextPageBtn(font, "Next", {100.f, 45.f}, {550.f, 600.f});

    sf::Text selectUniLabel(font);
    selectUniLabel.setString("Enter University Code:");
    selectUniLabel.setCharacterSize(18);
    selectUniLabel.setFillColor(sf::Color::White);
    selectUniLabel.setPosition({150.f, 120.f});
    TextBox selectUniCodeBox(font, {300.f, 45.f}, {150.f, 150.f});
    Button selectUniSubmit(font, "Search", {150.f, 45.f}, {480.f, 150.f});

    sf::Text searchBusLabel(font);
    searchBusLabel.setString("Enter Bus ID:");
    searchBusLabel.setCharacterSize(18);
    searchBusLabel.setFillColor(sf::Color::White);
    searchBusLabel.setPosition({150.f, 120.f});
    TextBox searchBusIdBox(font, {300.f, 45.f}, {150.f, 150.f});
    Button searchBusSubmit(font, "Search", {150.f, 45.f}, {480.f, 150.f});

    sf::Text searchStopLabel(font);
    searchStopLabel.setString("Enter Stop Name:");
    searchStopLabel.setCharacterSize(18);
    searchStopLabel.setFillColor(sf::Color::White);
    searchStopLabel.setPosition({150.f, 120.f});
    TextBox searchStopBox(font, {300.f, 45.f}, {150.f, 150.f});
    Button searchStopSubmit(font, "Search", {150.f, 45.f}, {480.f, 150.f});

    UserState state = USER_DASHBOARD;
    int currentPage = 0;
    const int itemsPerPage = 8; 
    const int busesPerPage = 5;

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

                if (state == USER_DASHBOARD)
                {
                    if (viewUniBtn.isClicked(window))
                    {
                        state = USER_VIEW_UNIVERSITIES;
                        title.setString("UNIVERSITY LIST");
                        universitiesList = user.getUniversities();
                        currentPage = 0;
                    }
                    else if (selectUniBtn.isClicked(window))
                    {
                        state = USER_SELECT_UNIVERSITY;
                        title.setString("UNIVERSITY BUSES");
                        selectUniCodeBox.setFocused(true);
                        busesList.clear();
                        infoMsg.setString("");
                    }
                    else if (viewAllBusesBtn.isClicked(window))
                    {
                        state = USER_VIEW_BUSES;
                        title.setString("ALL BUSES LIST");
                        busesList = user.getBuses();
                        currentPage = 0;
                    }
                    else if (searchBusBtn.isClicked(window))
                    {
                        state = USER_SEARCH_BUS;
                        title.setString("SEARCH BUS");
                        searchBusIdBox.setFocused(true);
                        busesList.clear();
                        infoMsg.setString("");
                    }
                    else if (searchStopBtn.isClicked(window))
                    {
                        state = USER_SEARCH_BY_STOP;
                        title.setString("SEARCH BY STOP");
                        searchStopBox.setFocused(true);
                        busesList.clear();
                        infoMsg.setString("");
                    }
                    else if (logoutBtn.isClicked(window))
                    {
                        window.close();
                    }
                }
                else if (state == USER_VIEW_UNIVERSITIES)
                {
                    if (backBtn.isClicked(window))
                    {
                        state = USER_DASHBOARD;
                        title.setString("USER DASHBOARD");
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
                else if (state == USER_SELECT_UNIVERSITY)
                {
                    if (mouse.x >= 150 && mouse.x <= 450 && mouse.y >= 150 && mouse.y <= 195)
                        selectUniCodeBox.setFocused(true);

                    if (selectUniSubmit.isClicked(window))
                    {
                        busesList = user.getBusesForUniversity(selectUniCodeBox.getText());
                        currentPage = 0;
                        if (busesList.empty())
                            infoMsg.setString("No Buses Found for University: " + selectUniCodeBox.getText());
                        else
                            infoMsg.setString("");
                    }
                    else if (backBtn.isClicked(window))
                    {
                        state = USER_DASHBOARD;
                        title.setString("USER DASHBOARD");
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
                else if (state == USER_VIEW_BUSES)
                {
                    if (backBtn.isClicked(window))
                    {
                        state = USER_DASHBOARD;
                        title.setString("USER DASHBOARD");
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
                else if (state == USER_SEARCH_BUS)
                {
                    if (mouse.x >= 150 && mouse.x <= 450 && mouse.y >= 150 && mouse.y <= 195)
                        searchBusIdBox.setFocused(true);

                    if (searchBusSubmit.isClicked(window))
                    {
                        busesList = user.searchBus(searchBusIdBox.getText());
                        currentPage = 0;
                        if (busesList.empty())
                            infoMsg.setString("Bus Not Found: " + searchBusIdBox.getText());
                        else
                            infoMsg.setString("");
                    }
                    else if (backBtn.isClicked(window))
                    {
                        state = USER_DASHBOARD;
                        title.setString("USER DASHBOARD");
                    }
                }
                else if (state == USER_SEARCH_BY_STOP)
                {
                    if (mouse.x >= 150 && mouse.x <= 450 && mouse.y >= 150 && mouse.y <= 195)
                        searchStopBox.setFocused(true);

                    if (searchStopSubmit.isClicked(window))
                    {
                        busesList = user.searchByStop(searchStopBox.getText());
                        currentPage = 0;
                        if (busesList.empty())
                            infoMsg.setString("No Buses Found containing stop: " + searchStopBox.getText());
                        else
                            infoMsg.setString("");
                    }
                    else if (backBtn.isClicked(window))
                    {
                        state = USER_DASHBOARD;
                        title.setString("USER DASHBOARD");
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
                if (state == USER_SELECT_UNIVERSITY)
                    selectUniCodeBox.handleEvent(*event);
                else if (state == USER_SEARCH_BUS)
                    searchBusIdBox.handleEvent(*event);
                else if (state == USER_SEARCH_BY_STOP)
                    searchStopBox.handleEvent(*event);
            }
        }

        if (state == USER_DASHBOARD)
        {
            viewUniBtn.update(window);
            selectUniBtn.update(window);
            viewAllBusesBtn.update(window);
            searchBusBtn.update(window);
            searchStopBtn.update(window);
            logoutBtn.update(window);
        }
        else
        {
            backBtn.update(window);
            prevPageBtn.update(window);
            nextPageBtn.update(window);

            if (state == USER_SELECT_UNIVERSITY) selectUniSubmit.update(window);
            else if (state == USER_SEARCH_BUS) searchBusSubmit.update(window);
            else if (state == USER_SEARCH_BY_STOP) searchStopSubmit.update(window);
        }

        window.clear(sf::Color(30, 45, 55)); 

        sf::FloatRect titleBounds = title.getLocalBounds();
        title.setOrigin({titleBounds.position.x + titleBounds.size.x / 2.f, 0.f});
        title.setPosition({450.f, 40.f}); 

        window.draw(title);

        if (state == USER_DASHBOARD)
        {
            viewUniBtn.draw(window);
            selectUniBtn.draw(window);
            viewAllBusesBtn.draw(window);
            searchBusBtn.draw(window);
            searchStopBtn.draw(window);
            logoutBtn.draw(window);
        }
        else
        {
            backBtn.draw(window);

            if (state == USER_VIEW_UNIVERSITIES)
            {
                if (universitiesList.empty())
                {
                    sf::Text row(font, "No Universities Registered.", 22);
                    row.setFillColor(sf::Color::Yellow);
                    row.setPosition({300.f, 200.f});
                    window.draw(row);
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
                        currentY += 45.f;
                    }

                    if (currentPage > 0) prevPageBtn.draw(window);
                    if (endIdx < (int)universitiesList.size()) nextPageBtn.draw(window);
                }
            }
            else if (state == USER_SELECT_UNIVERSITY)
            {
                window.draw(selectUniLabel);
                selectUniCodeBox.draw(window);
                selectUniSubmit.draw(window);

                if (!infoMsg.getString().isEmpty())
                {
                    infoMsg.setPosition({150.f, 210.f});
                    window.draw(infoMsg);
                }
                else
                {
                    int startIdx = currentPage * busesPerPage;
                    int endIdx = min(startIdx + busesPerPage, (int)busesList.size());
                    float currentY = 220.f;

                    for (int i = startIdx; i < endIdx; ++i)
                    {
                        string info = formatBusInfo(busesList[i], 780.f, font, 18, false);
                        sf::Text row(font, info, 18);
                        row.setFillColor(sf::Color::White);
                        row.setPosition({80.f, currentY});
                        window.draw(row);
                        currentY += 65.f;
                    }

                    if (currentPage > 0) prevPageBtn.draw(window);
                    if (endIdx < (int)busesList.size()) nextPageBtn.draw(window);
                }
            }
            else if (state == USER_VIEW_BUSES)
            {
                if (busesList.empty())
                {
                    sf::Text row(font, "No Buses Registered.", 22);
                    row.setFillColor(sf::Color::Yellow);
                    row.setPosition({300.f, 200.f});
                    window.draw(row);
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

                    if (currentPage > 0) prevPageBtn.draw(window);
                    if (endIdx < (int)busesList.size()) nextPageBtn.draw(window);
                }
            }
            else if (state == USER_SEARCH_BUS)
            {
                window.draw(searchBusLabel);
                searchBusIdBox.draw(window);
                searchBusSubmit.draw(window);

                if (!infoMsg.getString().isEmpty())
                {
                    infoMsg.setPosition({150.f, 210.f});
                    window.draw(infoMsg);
                }
                else if (!busesList.empty())
                {
                    float currentY = 220.f;
                    string info1 = "Bus ID      : " + busesList[0].getBusID();
                    string info2 = "Bus Name    : " + busesList[0].getBusName();
                    string info3 = "University  : " + busesList[0].getUniversityCode();
                    string info4 = "Total Seats : " + to_string(busesList[0].getTotalSeats());
                    string info5 = wrapRoute(busesList[0].getRoute(), 710.f, font, 20, "Route       : ");

                    string fullInfo = info1 + "\n" + info2 + "\n" + info3 + "\n" + info4 + "\n" + info5;

                    sf::Text row(font, fullInfo, 20);
                    row.setFillColor(sf::Color::White);
                    row.setPosition({150.f, currentY});
                    window.draw(row);
                }
            }
            else if (state == USER_SEARCH_BY_STOP)
            {
                window.draw(searchStopLabel);
                searchStopBox.draw(window);
                searchStopSubmit.draw(window);

                if (!infoMsg.getString().isEmpty())
                {
                    infoMsg.setPosition({150.f, 210.f});
                    window.draw(infoMsg);
                }
                else
                {
                    int startIdx = currentPage * busesPerPage;
                    int endIdx = min(startIdx + busesPerPage, (int)busesList.size());
                    float currentY = 220.f;

                    for (int i = startIdx; i < endIdx; ++i)
                    {
                        string header = busesList[i].getBusID() + " | " + busesList[i].getBusName() + " (" + busesList[i].getUniversityCode() + ")";
                        string routeWrapped = wrapRoute(busesList[i].getRoute(), 780.f, font, 18, "Route: ");
                        string info = header + "\n" + routeWrapped;
                        sf::Text row(font, info, 18);
                        row.setFillColor(sf::Color::White);
                        row.setPosition({80.f, currentY});
                        window.draw(row);
                        currentY += 65.f;
                    }

                    if (currentPage > 0) prevPageBtn.draw(window);
                    if (endIdx < (int)busesList.size()) nextPageBtn.draw(window);
                }
            }
        }

        window.display();
    }
}
