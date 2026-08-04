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
    if (!font.openFromFile("assets/Inter-Regular.ttf")) return;
    Theme::configureFont(font);

    Admin admin;

    std::string errorText;
    bool focusUser = true;

    // ── Layout (fixed window — compute once) ──────────────────────────
    const float ww = 720.f, wh = 500.f;
    const float cW = std::min(480.f, ww - 60.f);
    const float cH = 355.f;
    const float cX = (ww - cW) * 0.5f;
    const float cY = (wh - cH) * 0.5f;
    const float fX = cX + 28.f;
    const float fW = cW - 56.f;

    // ── TextBox objects live OUTSIDE the game loop so m_value persists ─
    TextBox userBox(font, {fW, 46.f}, {fX, cY + 118.f});
    TextBox passBox(font, {fW, 46.f}, {fX, cY + 210.f});
    userBox.setPlaceholder("Enter username");
    passBox.setPlaceholder("Enter password");
    passBox.setPasswordMode(true);
    userBox.setFocused(true);

    while (window.isOpen())
    {
        // Sync focus state to TextBox objects each frame
        userBox.setFocused(focusUser);
        passBox.setFocused(!focusUser);

        // Buttons are stateless — fine to construct each frame
        Button loginBtn(font, "Login",  {fW * 0.58f, 46.f}, {fX,               cY + 280.f});
        Button backBtn (font, "< Back", {fW * 0.36f, 46.f}, {fX + fW * 0.62f,  cY + 280.f},
                        ButtonStyle::SECONDARY);

        // ── Events ───────────────────────────────────────────────────
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>()) window.close();

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

                // Deliver to textboxes for eye-icon click detection
                userBox.handleEvent(*event);
                passBox.handleEvent(*event);
            }

            if (event->is<sf::Event::TextEntered>()) {
                if (focusUser) userBox.handleEvent(*event);
                else           passBox.handleEvent(*event);
            }
        }

        loginBtn.update(window);
        backBtn.update(window);

        // ── Draw ─────────────────────────────────────────────────────
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

        sf::Text title(font);
        title.setString("Admin Login");
        title.setCharacterSize(24);
        title.setFillColor(Theme::TEXT_PRIMARY);
        sf::FloatRect tb = title.getLocalBounds();
        title.setOrigin({tb.position.x + tb.size.x * 0.5f,
                         tb.position.y + tb.size.y * 0.5f});
        title.setPosition(Theme::px(cX + cW * 0.5f, cY + 76.f));
        window.draw(title);

        Theme::drawSeparator(window, cX + 28.f, cY + 97.f, cW - 56.f);

        auto drawLabel = [&](const std::string& s, float x, float y) {
            sf::Text lbl(font);
            lbl.setString(s);
            lbl.setCharacterSize(11);
            lbl.setFillColor(Theme::TEXT_SECONDARY);
            lbl.setPosition(Theme::px(x, y));
            window.draw(lbl);
        };
        drawLabel("USERNAME", fX, cY + 103.f);
        drawLabel("PASSWORD", fX, cY + 194.f);

        userBox.draw(window);
        passBox.draw(window);
        loginBtn.draw(window);
        backBtn.draw(window);

        if (!errorText.empty()) {
            sf::Text err(font);
            err.setString("  " + errorText);
            err.setCharacterSize(13);
            err.setFillColor(Theme::DANGER);
            err.setPosition({fX, cY + 336.f});
            window.draw(err);
        }

        window.display();
    }
}