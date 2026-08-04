#include "UserRegistrationGUI.h"
#include "Button.h"
#include "TextBox.h"
#include "Theme.h"
#include "User.h"

#include <SFML/Graphics.hpp>
#include <algorithm>

void UserRegistrationGUI::run()
{
    sf::RenderWindow window(sf::VideoMode({720, 520}),
                            "University Bus Tracker - Register");
    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.openFromFile("assets/Inter-Regular.ttf")) return;
    Theme::configureFont(font);

    User user;
    std::string infoText;
    bool infoIsError = false;
    bool focusEmail  = true;

    // ── Layout ────────────────────────────────────────────────────────
    const float ww = 720.f, wh = 520.f;
    const float cW = std::min(480.f, ww - 60.f);
    const float cH = 390.f;
    const float cX = (ww - cW) * 0.5f;
    const float cY = (wh - cH) * 0.5f;
    const float fX = cX + 28.f;
    const float fW = cW - 56.f;

    // ── TextBox objects outside the game loop ─────────────────────────
    TextBox emailBox(font, {fW, 46.f}, {fX, cY + 130.f});
    TextBox passBox (font, {fW, 46.f}, {fX, cY + 222.f});
    emailBox.setPlaceholder("yourname@gmail.com");
    passBox.setPlaceholder("Choose a password");
    passBox.setPasswordMode(true);
    emailBox.setFocused(true);

    while (window.isOpen())
    {
        emailBox.setFocused(focusEmail);
        passBox.setFocused(!focusEmail);

        Button regBtn  (font, "Create Account", {fW * 0.58f, 46.f}, {fX,               cY + 305.f},
                        ButtonStyle::SUCCESS);
        Button backBtn (font, "< Back",         {fW * 0.36f, 46.f}, {fX + fW * 0.62f,  cY + 305.f},
                        ButtonStyle::SECONDARY);

        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>()) window.close();

            if (event->is<sf::Event::MouseButtonPressed>())
            {
                auto mp = sf::Mouse::getPosition(window);
                float mx = static_cast<float>(mp.x);
                float my = static_cast<float>(mp.y);

                if (emailBox.getBounds().contains({mx, my})) { focusEmail = true;  }
                if (passBox.getBounds().contains({mx, my}))  { focusEmail = false; }

                if (regBtn.isClicked(window)) {
                    std::string errMsg;
                    if (user.registerUser(emailBox.getText(), passBox.getText(), errMsg)) {
                        infoText    = "Account created! You can now login.";
                        infoIsError = false;
                        emailBox.clear();
                        passBox.clear();
                        focusEmail = true;
                    } else {
                        infoText    = errMsg;
                        infoIsError = true;
                    }
                }
                if (backBtn.isClicked(window)) window.close();

                // Deliver to textboxes for eye-icon click detection
                emailBox.handleEvent(*event);
                passBox.handleEvent(*event);
            }

            if (event->is<sf::Event::TextEntered>()) {
                if (focusEmail) emailBox.handleEvent(*event);
                else            passBox.handleEvent(*event);
            }
        }

        regBtn.update(window);
        backBtn.update(window);

        // ── Draw ─────────────────────────────────────────────────────
        window.clear(Theme::BG_DARK);
        Theme::drawGradientRect(window, {0.f, 0.f}, {ww, wh},
                                sf::Color(15, 22, 40), sf::Color(10, 15, 28));

        Theme::drawCard(window, {cX, cY}, {cW, cH}, Theme::BG_CARD, 14.f);

        sf::RectangleShape accent({cW, 4.f});
        accent.setPosition({cX, cY});
        accent.setFillColor(Theme::SUCCESS_DARK);
        window.draw(accent);

        Theme::drawIconCircle(window, font, {cX + cW * 0.5f, cY + 42.f},
                              22.f, Theme::withAlpha(Theme::SUCCESS, 45),
                              "+", Theme::SUCCESS, 22);

        sf::Text title(font);
        title.setString("Create Account");
        title.setCharacterSize(24);
        title.setFillColor(Theme::TEXT_PRIMARY);
        sf::FloatRect tb = title.getLocalBounds();
        title.setOrigin({tb.position.x + tb.size.x * 0.5f,
                         tb.position.y + tb.size.y * 0.5f});
        title.setPosition({cX + cW * 0.5f, cY + 78.f});
        window.draw(title);

        Theme::drawSeparator(window, cX + 28.f, cY + 100.f, cW - 56.f);

        auto drawLabel = [&](const std::string& s, float x, float y) {
            sf::Text lbl(font);
            lbl.setString(s);
            lbl.setCharacterSize(11);
            lbl.setFillColor(Theme::TEXT_SECONDARY);
            lbl.setPosition({x, y});
            window.draw(lbl);
        };
        drawLabel("GMAIL ADDRESS (must end in @gmail.com)", fX, cY + 115.f);
        drawLabel("PASSWORD",                               fX, cY + 207.f);

        emailBox.draw(window);
        passBox.draw(window);
        regBtn.draw(window);
        backBtn.draw(window);

        if (!infoText.empty()) {
            sf::Text info(font);
            info.setString("  " + infoText);
            info.setCharacterSize(13);
            info.setFillColor(infoIsError ? Theme::DANGER : Theme::SUCCESS);
            info.setPosition({fX, cY + 362.f});
            window.draw(info);
        }

        window.display();
    }
}
