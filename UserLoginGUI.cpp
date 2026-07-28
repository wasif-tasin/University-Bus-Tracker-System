#include "UserLoginGUI.h"
#include "Button.h"
#include "TextBox.h"
#include "User.h"
#include "UserDashboardGUI.h"

#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>

using namespace std;

void UserLoginGUI::run()
{
    sf::RenderWindow window(
        sf::VideoMode({800, 500}),
        "User Login");

    sf::Font font;
    if (!font.openFromFile("assets/Roboto-Regular.ttf"))
    {
        cout << "Font Load Failed!\n";
        return;
    }

    sf::Text title(font);
    title.setString("USER LOGIN");
    title.setCharacterSize(32);
    title.setFillColor(sf::Color::White);
    sf::FloatRect titleBounds = title.getLocalBounds();
    title.setOrigin({titleBounds.position.x + titleBounds.size.x / 2.f, 0.f});
    title.setPosition({400.f, 40.f});

    sf::Text emailLabel(font);
    emailLabel.setString("Gmail Address");
    emailLabel.setCharacterSize(22);
    emailLabel.setFillColor(sf::Color::White);
    emailLabel.setPosition({150.f, 130.f});

    sf::Text passLabel(font);
    passLabel.setString("Password");
    passLabel.setCharacterSize(22);
    passLabel.setFillColor(sf::Color::White);
    passLabel.setPosition({150.f, 220.f});

    sf::Text errorMsg(font);
    errorMsg.setCharacterSize(18);
    errorMsg.setFillColor(sf::Color::Red);
    errorMsg.setPosition({150.f, 305.f});

    TextBox emailBox(font, {350.f, 45.f}, {150.f, 160.f});
    TextBox passwordBox(font, {350.f, 45.f}, {150.f, 250.f});

    Button loginBtn(font, "Login", {150.f, 50.f}, {150.f, 340.f});
    Button backBtn(font, "Back", {150.f, 50.f}, {350.f, 340.f});

    bool typingEmail = true;
    emailBox.setFocused(true);
    passwordBox.setFocused(false);

    User user;

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
                    typingEmail = true;
                    emailBox.setFocused(true);
                    passwordBox.setFocused(false);
                }

                if (mouse.x >= 150 && mouse.x <= 500 &&
                    mouse.y >= 250 && mouse.y <= 295)
                {
                    typingEmail = false;
                    emailBox.setFocused(false);
                    passwordBox.setFocused(true);
                }

                if (loginBtn.isClicked(window))
                {
                    string errMsg;
                    if (user.loginUser(emailBox.getText(), passwordBox.getText(), errMsg))
                    {
                        errorMsg.setString("");
                        UserDashboardGUI dashboard;
                        dashboard.run();
                    }
                    else
                    {
                        errorMsg.setString("Error: " + errMsg);
                    }
                }

                if (backBtn.isClicked(window))
                {
                    window.close();
                }
            }

            if (event->is<sf::Event::TextEntered>())
            {
                if (typingEmail)
                    emailBox.handleEvent(*event);
                else
                    passwordBox.handleEvent(*event);
            }
        }

        loginBtn.update(window);
        backBtn.update(window);

        window.clear(sf::Color(40, 60, 50)); // Slightly greenish dark background for user flows

        window.draw(title);
        window.draw(emailLabel);
        window.draw(passLabel);
        window.draw(errorMsg);

        emailBox.draw(window);
        passwordBox.draw(window);

        loginBtn.draw(window);
        backBtn.draw(window);

        window.display();
    }
}
