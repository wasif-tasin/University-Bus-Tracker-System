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
    if (!Theme::loadUIFont(font)) return;

    User user;
    std::string infoText;
    bool infoIsError = false;
    bool focusEmail  = true;

    // ── Layout ────────────────────────────────────────────────────────
    // Card height is even so the centred card lands on integer pixels; a
    // half-pixel origin here would soften every glyph inside it.
    const float ww = 720.f, wh = 520.f;
    const float cW = std::min(480.f, ww - 60.f);
    const float cH = 396.f;
    const float cX = std::round((ww - cW) * 0.5f);
    const float cY = std::round((wh - cH) * 0.5f);
    const float fX = cX + 28.f;
    const float fW = cW - 56.f;

    // ── TextBox objects outside the game loop ─────────────────────────
    TextBox emailBox(font, {fW, 46.f}, {fX, cY + 148.f});
    TextBox passBox (font, {fW, 46.f}, {fX, cY + 228.f});
    emailBox.setPlaceholder("yourname@gmail.com");
    passBox.setPlaceholder("Choose a password");
    passBox.setPasswordMode(true);
    emailBox.setFocused(true);

    while (window.isOpen())
    {
        emailBox.setFocused(focusEmail);
        passBox.setFocused(!focusEmail);

        Button regBtn  (font, "Create Account", {fW * 0.58f, 46.f}, {fX,               cY + 296.f},
                        ButtonStyle::SUCCESS);
        Button backBtn (font, "< Back",         {fW * 0.36f, 46.f}, {fX + fW * 0.62f,  cY + 296.f},
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

            if (event->is<sf::Event::TextEntered>() ||
                event->is<sf::Event::KeyPressed>()) {
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

        Theme::drawTextHCentered(window, font, "Create Account", Theme::Type::TITLE,
                                 Theme::TEXT_PRIMARY, cX + cW * 0.5f, cY + 68.f,
                                 sf::Text::Bold);

        Theme::drawSeparator(window, cX + 28.f, cY + 108.f, cW - 56.f);

        auto drawLabel = [&](const std::string& s, float x, float y) {
            Theme::drawText(window, font, s, Theme::Type::LABEL,
                            Theme::TEXT_MUTED, {x, y}, sf::Text::Bold);
        };
        drawLabel("GMAIL ADDRESS (must end in @gmail.com)", fX, cY + 126.f);
        drawLabel("PASSWORD",                               fX, cY + 208.f);

        emailBox.draw(window);
        passBox.draw(window);
        regBtn.draw(window);
        backBtn.draw(window);

        if (!infoText.empty()) {
            Theme::drawText(window, font, infoText, Theme::Type::META,
                            infoIsError ? Theme::DANGER_HOVER : Theme::SUCCESS,
                            {fX + 2.f, cY + 356.f}, sf::Text::Bold);
        }

        window.display();
    }
}
