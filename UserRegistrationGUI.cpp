#include "UserRegistrationGUI.h"
#include "Button.h"
#include "TextBox.h"
#include "User.h"

#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>

using namespace std;

void UserRegistrationGUI::run()
{
    sf::RenderWindow window(
        sf::VideoMode({800, 500}),
        "User Registration");

    sf::Font font;
    if (!font.openFromFile("assets/Roboto-Regular.ttf"))
    {
        cout << "Font Load Failed!\n";
        return;
    }

    sf::Text title(font);
    title.setString("USER REGISTRATION");
    title.setCharacterSize(32);
    title.setFillColor(sf::Color::White);
    sf::FloatRect titleBounds = title.getLocalBounds();
    title.setOrigin({titleBounds.position.x + titleBounds.size.x / 2.f, 0.f});
    title.setPosition({400.f, 40.f});

    sf::Text emailLabel(font);
    emailLabel.setString("Gmail Address (ends in @gmail.com)");
    emailLabel.setCharacterSize(20);
    emailLabel.setFillColor(sf::Color::White);
    emailLabel.setPosition({150.f, 130.f});

    sf::Text passLabel(font);
    passLabel.setString("Password");
    passLabel.setCharacterSize(20);
    passLabel.setFillColor(sf::Color::White);
    passLabel.setPosition({150.f, 220.f});

    sf::Text infoMsg(font);
    infoMsg.setCharacterSize(18);
    infoMsg.setPosition({150.f, 305.f});

    TextBox emailBox(font, {350.f, 45.f}, {150.f, 160.f});
    TextBox passwordBox(font, {350.f, 45.f}, {150.f, 250.f});

    Button registerBtn(font, "Register", {150.f, 50.f}, {150.f, 340.f});
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

                if (registerBtn.isClicked(window))
                {
                    string errMsg;
                    if (user.registerUser(emailBox.getText(), passwordBox.getText(), errMsg))
                    {
                        infoMsg.setFillColor(sf::Color::Green);
                        infoMsg.setString("Registration Successful! Click Back to Login.");
                    }
                    else
                    {
                        infoMsg.setFillColor(sf::Color::Red);
                        infoMsg.setString("Error: " + errMsg);
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

        registerBtn.update(window);
        backBtn.update(window);

        window.clear(sf::Color(40, 60, 50)); // Slightly greenish dark background for user flows

        window.draw(title);
        window.draw(emailLabel);
        window.draw(passLabel);

        emailBox.draw(window);
        passwordBox.draw(window);

        registerBtn.draw(window);
        backBtn.draw(window);

        if (!infoMsg.getString().isEmpty())
        {
            window.draw(infoMsg);
        }

        window.display();
    }
}
