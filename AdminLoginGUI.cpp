#include "AdminLoginGUI.h"
#include "Button.h"
#include "TextBox.h"
#include "AdminDashboardGUI.h"
#include "Admin.h"

#include <SFML/Graphics.hpp>
#include <iostream>

using namespace std;

void AdminLoginGUI::run()
{
    sf::RenderWindow window(
        sf::VideoMode({800, 500}),
        "Admin Login");

    sf::Font font;

    if (!font.openFromFile("assets/Roboto-Regular.ttf"))
    {
        cout << "Font Load Failed!\n";
        return;
    }

    sf::Text title(font);
    title.setString("ADMIN LOGIN");
    title.setCharacterSize(32);
    title.setFillColor(sf::Color::White);
    sf::FloatRect titleBounds = title.getLocalBounds();
    title.setOrigin({titleBounds.position.x + titleBounds.size.x / 2.f, 0.f});
    title.setPosition({400.f, 40.f});

    sf::Text userLabel(font);
    userLabel.setString("Username");
    userLabel.setCharacterSize(22);
    userLabel.setFillColor(sf::Color::White);
    userLabel.setPosition({150.f, 130.f});

    sf::Text passLabel(font);
    passLabel.setString("Password");
    passLabel.setCharacterSize(22);
    passLabel.setFillColor(sf::Color::White);
    passLabel.setPosition({150.f, 220.f});

    sf::Text errorMsg(font);
    errorMsg.setCharacterSize(18);
    errorMsg.setFillColor(sf::Color::Red);
    errorMsg.setPosition({150.f, 305.f});

    TextBox username(font, {350.f, 45.f}, {150.f, 160.f});
    TextBox password(font, {350.f, 45.f}, {150.f, 250.f});

    Button login(font, "Login", {150.f, 50.f}, {150.f, 340.f});
    Button back(font, "Back", {150.f, 50.f}, {350.f, 340.f});

    bool typingUsername = true;
    username.setFocused(true);
    password.setFocused(false);

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();

            if (event->is<sf::Event::MouseButtonPressed>())
            {
                sf::Vector2i mouse = sf::Mouse::getPosition(window);

                if (mouse.x >= 150 && mouse.x <= 500 &&
                    mouse.y >= 160 && mouse.y <= 205)
                {
                    typingUsername = true;
                    username.setFocused(true);
                    password.setFocused(false);
                }

                if (mouse.x >= 150 && mouse.x <= 500 &&
                    mouse.y >= 250 && mouse.y <= 295)
                {
                    typingUsername = false;
                    username.setFocused(false);
                    password.setFocused(true);
                }

                if (login.isClicked(window))
                {
                    Admin admin;
                    if (admin.login(username.getText(), password.getText()))
                    {
                        errorMsg.setString("");
                        AdminDashboardGUI dashboard;
                        dashboard.run();
                    }
                    else
                    {
                        errorMsg.setString("Invalid Username or Password!");
                    }
                }

                if (back.isClicked(window))
                {
                    window.close();
                }
            }

            if (event->is<sf::Event::TextEntered>())
            {
                if (const auto* txt = event->getIf<sf::Event::TextEntered>())
                {
                    std::cout << "[DEBUG] Text Entered: unicode = " << static_cast<uint32_t>(txt->unicode) << std::endl;
                }
                if (typingUsername)
                    username.handleEvent(*event);
                else
                    password.handleEvent(*event);
            }
        }

        login.update(window);
        back.update(window);

        window.clear(sf::Color(40, 40, 60));

        window.draw(title);
        window.draw(userLabel);
        window.draw(passLabel);
        window.draw(errorMsg);

        username.draw(window);
        password.draw(window);

        login.draw(window);
        back.draw(window);

        window.display();
    }
}