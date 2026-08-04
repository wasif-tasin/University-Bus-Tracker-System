#include "AdminLoginGUI.h"
#include "Button.h"
#include "TextBox.h"
#include "Theme.h"
#include "AdminDashboardGUI.h"
#include "Admin.h"

#include <SFML/Graphics.hpp>
#include <algorithm>

void AdminLoginGUI::run()
{
    sf::RenderWindow window(sf::VideoMode({720, 500}),
                            "University Bus Tracker - Admin Login");
    window.setFramerateLimit(60);

    sf::Font font;
    if (!Theme::loadUIFont(font)) return;

    Admin admin;

    std::string errorText;
    bool focusUser = true;
    const float ww = 720.f, wh = 500.f;
    const float cW = std::min(480.f, ww - 60.f);
    const float cH = 376.f;
    const float cX = std::round((ww - cW) * 0.5f);
    const float cY = std::round((wh - cH) * 0.5f);
    const float fX = cX + 28.f;
    const float fW = cW - 56.f;
    TextBox userBox(font, {fW, 46.f}, {fX, cY + 140.f});
    TextBox passBox(font, {fW, 46.f}, {fX, cY + 220.f});
    userBox.setPlaceholder("Enter username");
    passBox.setPlaceholder("Enter password");
    passBox.setPasswordMode(true);
    userBox.setFocused(true);

    while (window.isOpen())
    {
        userBox.setFocused(focusUser);
        passBox.setFocused(!focusUser);

        Button loginBtn(font, "Login",  {fW * 0.58f, 46.f}, {fX,               cY + 288.f});
        Button backBtn (font, "< Back", {fW * 0.36f, 46.f}, {fX + fW * 0.62f,  cY + 288.f},
                        ButtonStyle::SECONDARY);

        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>()) window.close();
            Theme::syncViewToWindow(window, *event);
            if (event->is<sf::Event::MouseButtonPressed>())
            {
                auto mp = sf::Mouse::getPosition(window);
                float mx = static_cast<float>(mp.x);
                float my = static_cast<float>(mp.y);

                if (userBox.getBounds().contains({mx, my})) { focusUser = true;  }
                if (passBox.getBounds().contains({mx, my})) { focusUser = false; }

                if (loginBtn.isClicked(window)) {
                    if (admin.login(userBox.getText(), passBox.getText())) {
                        errorText = "";
                        AdminDashboardGUI dash;
                        dash.run();
                    } else {
                        errorText = "Invalid username or password.";
                    }
                }
                if (backBtn.isClicked(window)) window.close();
                userBox.handleEvent(*event);
                passBox.handleEvent(*event);
            }

            if (event->is<sf::Event::TextEntered>() ||
                event->is<sf::Event::KeyPressed>()) {
                if (focusUser) userBox.handleEvent(*event);
                else           passBox.handleEvent(*event);
            }
        }

        loginBtn.update(window);
        backBtn.update(window);
        window.clear(Theme::BG_DARK);
        Theme::drawGradientRect(window, {0.f, 0.f}, {ww, wh},
                                sf::Color(15, 22, 40), sf::Color(10, 15, 28));

        Theme::drawCard(window, {cX, cY}, {cW, cH}, Theme::BG_CARD, 14.f);

        sf::RectangleShape accent({cW, 4.f});
        accent.setPosition({cX, cY});
        accent.setFillColor(Theme::PURPLE);
        window.draw(accent);

        Theme::drawIconCircle(window, font, {cX + cW * 0.5f, cY + 42.f},
                              22.f, Theme::withAlpha(Theme::PURPLE, 55),
                              "A", Theme::PURPLE, 18);

        Theme::drawTextHCentered(window, font, "Admin Login", Theme::Type::TITLE,
                                 Theme::TEXT_PRIMARY, cX + cW * 0.5f, cY + 68.f,
                                 sf::Text::Bold);

        Theme::drawSeparator(window, cX + 28.f, cY + 106.f, cW - 56.f);

        auto drawLabel = [&](const std::string& s, float x, float y) {
            Theme::drawText(window, font, s, Theme::Type::LABEL,
                            Theme::TEXT_MUTED, {x, y}, sf::Text::Bold);
        };
        drawLabel("USERNAME", fX, cY + 120.f);
        drawLabel("PASSWORD", fX, cY + 200.f);

        userBox.draw(window);
        passBox.draw(window);
        loginBtn.draw(window);
        backBtn.draw(window);

        if (!errorText.empty()) {
            Theme::drawText(window, font, errorText, Theme::Type::META,
                            Theme::DANGER_HOVER, {fX + 2.f, cY + 346.f},
                            sf::Text::Bold);
        }

        window.display();
    }
}