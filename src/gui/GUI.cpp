#include "GUI.h"
#include "Button.h"
#include "AdminLoginGUI.h"
#include "UserLoginGUI.h"
#include "UserRegistrationGUI.h"

#include <SFML/Graphics.hpp>
#include <iostream>

using namespace std;

void GUI::run()
{
    sf::RenderWindow window(
        sf::VideoMode({1200, 700}),
        "University Bus Tracker");

    sf::Font font;

    if (!font.openFromFile("assets/Roboto-Regular.ttf"))
    {
        cout << "Font Load Failed!\n";
        return;
    }

    // ================= TITLE =================

    sf::Text title(font);
    title.setString("UNIVERSITY BUS TRACKER");
    title.setCharacterSize(40);
    title.setFillColor(sf::Color::White);
    title.setPosition({355.f, 40.f});

    // ================= BUTTONS =================

    Button admin(font, "Admin Login", {300.f, 60.f}, {450.f, 180.f});
    Button user(font, "User Login", {300.f, 60.f}, {450.f, 270.f});
    Button reg(font, "Registration", {300.f, 60.f}, {450.f, 360.f});
    Button exit(font, "Exit", {300.f, 60.f}, {450.f, 450.f});

    // ================= BUTTON TEXT =================

    // ================= MAIN LOOP =================

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();

            if (event->is<sf::Event::MouseButtonPressed>())
            {
                auto mouse =
                    window.mapPixelToCoords(
                        sf::Mouse::getPosition(window));

                if (admin.isClicked(window))
                {
                    AdminLoginGUI adminLogin;
                    adminLogin.run();
                }

                if (user.isClicked(window))
                {
                    UserLoginGUI userLogin;
                    userLogin.run();
                }

                if (reg.isClicked(window))
                {
                    UserRegistrationGUI registration;
                    registration.run();
                }

                if (exit.isClicked(window))
                {
                    window.close();
                }
            }
        }

        window.clear(sf::Color(30, 50, 80));

        admin.update(window);
        user.update(window);
        reg.update(window);
        exit.update(window);

        window.draw(title);

        admin.draw(window);
        user.draw(window);
        reg.draw(window);
        exit.draw(window);

        window.display();
    }
}