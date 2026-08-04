#include "UserPanelGUI.h"
#include "Button.h"
#include "TextBox.h"
#include "Theme.h"
#include "UserLoginGUI.h"
#include "UserRegistrationGUI.h"

#include <SFML/Graphics.hpp>
#include <algorithm>

void UserPanelGUI::run()
{
    sf::RenderWindow window(sf::VideoMode({720, 460}),
                            "University Bus Tracker - User Panel");
    window.setFramerateLimit(60);

    sf::Font font;
    if (!Theme::loadUIFont(font)) return;

    while (window.isOpen())
    {
        auto sz  = window.getSize();
        float ww = static_cast<float>(sz.x);
        float wh = static_cast<float>(sz.y);

        float cardW  = std::min(420.f, ww - 60.f);
        float cardH  = 260.f;
        float cardX  = (ww - cardW) * 0.5f;
        float cardY  = (wh - cardH) * 0.5f - 10.f;

        float btnW   = cardW - 60.f;
        float btnH   = 50.f;
        float btnX   = cardX + 30.f;
        float loginY = cardY + 110.f;
        float regY   = cardY + 175.f;

        Button loginBtn(font, "User Login",   {btnW, btnH}, {btnX, loginY});
        Button regBtn  (font, "Register",     {btnW, btnH}, {btnX, regY},   ButtonStyle::SECONDARY);
        Button backBtn (font, "< Back",       {90.f, 34.f}, {20.f, 16.f},  ButtonStyle::GHOST);

        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>()) window.close();
            Theme::syncViewToWindow(window, *event);
            if (event->is<sf::Event::MouseButtonPressed>())
            {
                if (loginBtn.isClicked(window)) {
                    UserLoginGUI ui;
                    ui.run();
                }
                if (regBtn.isClicked(window)) {
                    UserRegistrationGUI ur;
                    ur.run();
                }
                if (backBtn.isClicked(window)) {
                    window.close();
                }
            }
        }

        loginBtn.update(window);
        regBtn.update(window);
        backBtn.update(window);
        window.clear(Theme::BG_DARK);

        Theme::drawGradientRect(window, {0.f, 0.f}, {ww, wh},
                                sf::Color(15, 22, 40), sf::Color(10, 16, 30));

        
        Theme::drawCard(window, {cardX, cardY}, {cardW, cardH}, Theme::BG_CARD, 14.f);

        sf::RectangleShape accent({cardW, 4.f});
        accent.setPosition({cardX, cardY});
        accent.setFillColor(Theme::ACCENT);
        window.draw(accent);

        Theme::drawIconCircle(window, font,
                              {cardX + cardW * 0.5f, cardY + 45.f},
                              22.f,
                              Theme::withAlpha(Theme::ACCENT, 55), "U",
                              Theme::ACCENT, 18);

        Theme::drawTextHCentered(window, font, "User Panel", Theme::Type::TITLE,
                                 Theme::TEXT_PRIMARY, cardX + cardW * 0.5f,
                                 cardY + 68.f, sf::Text::Bold);

        Theme::drawSeparator(window, cardX + 30.f, cardY + 100.f, cardW - 60.f);

        loginBtn.draw(window);
        regBtn.draw(window);
        backBtn.draw(window);

        window.display();
    }
}
