#include "GUI.h"
#include "Button.h"
#include "Theme.h"
#include "AdminLoginGUI.h"
#include "UserPanelGUI.h"

#include <SFML/Graphics.hpp>
#include <algorithm>

void GUI::run()
{
    sf::RenderWindow window(sf::VideoMode({980, 620}),
                            "University Bus Tracker");
    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.openFromFile("assets/Inter-Regular.ttf")) {
        return;
    }
    Theme::configureFont(font);

    float card1HoverT = 0.f;
    float card2HoverT = 0.f;

    while (window.isOpen())
    {
        auto sz  = window.getSize();
        float ww = static_cast<float>(sz.x);
        float wh = static_cast<float>(sz.y);

        // ── Responsive layout ────────────────────────────────────────────
        float leftW   = ww * 0.38f;
        float rightX  = leftW + 2.f;
        float rightW  = ww - rightX;

        const float cardW  = std::min(rightW - 50.f, 500.f);
        const float cardH  = 120.f;
        const float cardGap= 22.f;
        const float totalH = 2.f * cardH + cardGap;
        float cardX        = rightX + (rightW - cardW) * 0.5f;
        float card1Y       = (wh - totalH) * 0.5f;
        float card2Y       = card1Y + cardH + cardGap;

        // Exit button (bottom right)
        Button exitBtn(font, "Exit", {90.f, 34.f},
                       {ww - 110.f, wh - 52.f}, ButtonStyle::GHOST);

        // ── Mouse for card hover ─────────────────────────────────────────
        auto mp = sf::Mouse::getPosition(window);
        bool c1h = mp.x >= (int)cardX && mp.x < (int)(cardX + cardW) &&
                   mp.y >= (int)card1Y && mp.y < (int)(card1Y + cardH);
        bool c2h = mp.x >= (int)cardX && mp.x < (int)(cardX + cardW) &&
                   mp.y >= (int)card2Y && mp.y < (int)(card2Y + cardH);

        card1HoverT = std::clamp(card1HoverT + (c1h ? 0.1f : -0.1f), 0.f, 1.f);
        card2HoverT = std::clamp(card2HoverT + (c2h ? 0.1f : -0.1f), 0.f, 1.f);

        // ── Events ───────────────────────────────────────────────────────
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>()) window.close();

            if (event->is<sf::Event::MouseButtonPressed>())
            {
                if (c1h) {
                    UserPanelGUI up;
                    up.run();
                }
                if (c2h) {
                    AdminLoginGUI al;
                    al.run();
                }
                if (exitBtn.isClicked(window)) window.close();
            }
        }

        exitBtn.update(window);

        // ── Draw ─────────────────────────────────────────────────────────
        window.clear(Theme::BG_DARK);

        // Left panel — gradient
        Theme::drawGradientRect(window, {0.f, 0.f}, {leftW, wh},
                                sf::Color(18, 26, 50), sf::Color(10, 15, 28));

        // Left panel — thin right border
        sf::RectangleShape divider({2.f, wh});
        divider.setPosition({leftW, 0.f});
        divider.setFillColor(Theme::BORDER_IDLE);
        window.draw(divider);

        // Left panel — decorative top accent
        sf::RectangleShape topAccent({leftW, 3.f});
        topAccent.setFillColor(Theme::ACCENT);
        window.draw(topAccent);

        // Left panel — bus icon circle
        float iconCX = leftW * 0.5f;
        float iconCY = wh * 0.5f - 60.f;
        Theme::fillRoundedRect(window, {iconCX - 40.f, iconCY - 40.f},
                               {80.f, 80.f}, 40.f,
                               Theme::withAlpha(Theme::ACCENT, 30));
        Theme::drawIconCircle(window, font, {iconCX, iconCY},
                              30.f, Theme::withAlpha(Theme::ACCENT, 60),
                              "B", Theme::ACCENT, 26);

        // Left panel — app name
        {
            sf::Text t1(font); t1.setString("University");
            t1.setCharacterSize(28); t1.setFillColor(Theme::TEXT_PRIMARY);
            sf::FloatRect b = t1.getLocalBounds();
            t1.setOrigin({b.position.x + b.size.x * 0.5f, 0.f});
            t1.setPosition({iconCX, iconCY + 56.f});
            window.draw(t1);

            sf::Text t2(font); t2.setString("Bus Tracker");
            t2.setCharacterSize(28); t2.setFillColor(Theme::ACCENT);
            sf::FloatRect b2 = t2.getLocalBounds();
            t2.setOrigin({b2.position.x + b2.size.x * 0.5f, 0.f});
            t2.setPosition({iconCX, iconCY + 92.f});
            window.draw(t2);
        }

        // Left panel — tagline
        {
            sf::Text tag(font);
            tag.setString("Your campus transport,\nsimplified.");
            tag.setCharacterSize(13);
            tag.setFillColor(Theme::TEXT_MUTED);
            sf::FloatRect b = tag.getLocalBounds();
            tag.setOrigin({b.position.x + b.size.x * 0.5f, 0.f});
            tag.setPosition({iconCX, iconCY + 136.f});
            window.draw(tag);
        }

        Theme::drawSeparator(window, leftW * 0.15f, wh - 42.f, leftW * 0.7f);
        {
            sf::Text ver(font); ver.setString("v1.0  |  SFML 3");
            ver.setCharacterSize(11); ver.setFillColor(Theme::TEXT_MUTED);
            sf::FloatRect b = ver.getLocalBounds();
            ver.setOrigin({b.position.x + b.size.x * 0.5f, 0.f});
            ver.setPosition({iconCX, wh - 30.f});
            window.draw(ver);
        }

        // Right panel — "Select a Panel" prompt
        {
            sf::Text prompt(font);
            prompt.setString("Select a Panel");
            prompt.setCharacterSize(14);
            prompt.setFillColor(Theme::TEXT_MUTED);
            sf::FloatRect b = prompt.getLocalBounds();
            prompt.setOrigin({b.position.x + b.size.x * 0.5f, 0.f});
            prompt.setPosition({rightX + rightW * 0.5f, card1Y - 36.f});
            window.draw(prompt);
        }

        // ── Card 1: User Panel ───────────────────────────────────────────
        {
            sf::Color bg = Theme::lerp(Theme::BG_CARD, Theme::BG_CARD_HOVER, card1HoverT);
            Theme::drawCard(window, {cardX, card1Y}, {cardW, cardH}, bg, 12.f);
            // Left accent
            Theme::drawAccentBar(window, cardX, card1Y, cardH, Theme::ACCENT, 4.f);
            // Icon
            Theme::drawIconCircle(window, font,
                                  {cardX + 52.f, card1Y + cardH * 0.5f},
                                  26.f,
                                  Theme::withAlpha(Theme::ACCENT, 45), "U",
                                  Theme::ACCENT, 20);
            // Title
            sf::Text ct(font); ct.setString("User Panel");
            ct.setCharacterSize(20); ct.setFillColor(Theme::TEXT_PRIMARY);
            sf::FloatRect b = ct.getLocalBounds();
            ct.setPosition({cardX + 96.f,
                            card1Y + cardH * 0.5f - b.size.y - b.position.y - 10.f});
            window.draw(ct);
            // Subtitle
            sf::Text cs(font); cs.setString("Login or register to browse bus routes");
            cs.setCharacterSize(13); cs.setFillColor(Theme::TEXT_SECONDARY);
            sf::FloatRect bs = cs.getLocalBounds();
            cs.setPosition({cardX + 96.f,
                            card1Y + cardH * 0.5f - bs.position.y + 4.f});
            window.draw(cs);
            // Arrow
            sf::Text ar(font); ar.setString(">");
            ar.setCharacterSize(18);
            ar.setFillColor(Theme::lerp(Theme::TEXT_MUTED, Theme::ACCENT, card1HoverT));
            sf::FloatRect ba = ar.getLocalBounds();
            ar.setOrigin({ba.position.x + ba.size.x * 0.5f,
                          ba.position.y + ba.size.y * 0.5f});
            ar.setPosition({cardX + cardW - 28.f, card1Y + cardH * 0.5f});
            window.draw(ar);
        }

        // ── Card 2: Admin Panel ──────────────────────────────────────────
        {
            sf::Color bg = Theme::lerp(Theme::BG_CARD, Theme::BG_CARD_HOVER, card2HoverT);
            Theme::drawCard(window, {cardX, card2Y}, {cardW, cardH}, bg, 12.f);
            // Left accent (purple)
            Theme::drawAccentBar(window, cardX, card2Y, cardH, Theme::PURPLE, 4.f);
            // Icon
            Theme::drawIconCircle(window, font,
                                  {cardX + 52.f, card2Y + cardH * 0.5f},
                                  26.f,
                                  Theme::withAlpha(Theme::PURPLE, 45), "A",
                                  Theme::PURPLE, 20);
            // Title
            sf::Text ct(font); ct.setString("Admin Panel");
            ct.setCharacterSize(20); ct.setFillColor(Theme::TEXT_PRIMARY);
            sf::FloatRect b = ct.getLocalBounds();
            ct.setPosition({cardX + 96.f,
                            card2Y + cardH * 0.5f - b.size.y - b.position.y - 10.f});
            window.draw(ct);
            // Subtitle
            sf::Text cs(font); cs.setString("Manage universities, buses and routes");
            cs.setCharacterSize(13); cs.setFillColor(Theme::TEXT_SECONDARY);
            sf::FloatRect bs = cs.getLocalBounds();
            cs.setPosition({cardX + 96.f,
                            card2Y + cardH * 0.5f - bs.position.y + 4.f});
            window.draw(cs);
            // Arrow
            sf::Text ar(font); ar.setString(">");
            ar.setCharacterSize(18);
            ar.setFillColor(Theme::lerp(Theme::TEXT_MUTED, Theme::PURPLE, card2HoverT));
            sf::FloatRect ba = ar.getLocalBounds();
            ar.setOrigin({ba.position.x + ba.size.x * 0.5f,
                          ba.position.y + ba.size.y * 0.5f});
            ar.setPosition({cardX + cardW - 28.f, card2Y + cardH * 0.5f});
            window.draw(ar);
        }

        exitBtn.draw(window);
        window.display();
    }
}