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
    if (!Theme::loadUIFont(font)) {
        return;
    }

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
            Theme::syncViewToWindow(window, *event);
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
            Theme::drawTextHCentered(window, font, "University", Theme::Type::DISPLAY,
                                     Theme::TEXT_PRIMARY, iconCX, iconCY + 56.f,
                                     sf::Text::Bold);
            Theme::drawTextHCentered(window, font, "Bus Tracker", Theme::Type::DISPLAY,
                                     Theme::ACCENT, iconCX, iconCY + 94.f,
                                     sf::Text::Bold);
        }

        // Left panel — tagline. Drawn line by line so both lines are centred,
        // not just the block, with a roomier line height.
        {
            Theme::drawTextHCentered(window, font, "Your campus transport,",
                                     Theme::Type::META, Theme::TEXT_SECONDARY,
                                     iconCX, iconCY + 142.f);
            Theme::drawTextHCentered(window, font, "simplified.",
                                     Theme::Type::META, Theme::TEXT_SECONDARY,
                                     iconCX, iconCY + 164.f);
        }

        Theme::drawSeparator(window, leftW * 0.15f, wh - 42.f, leftW * 0.7f);
        {
            Theme::drawTextHCentered(window, font, "v1.0  |  SFML 3", Theme::Type::CAPTION,
                                     Theme::TEXT_MUTED, iconCX, wh - 30.f);
        }

        // Right panel — "Select a Panel" prompt
        {
            Theme::drawTextHCentered(window, font, "SELECT A PANEL", Theme::Type::LABEL,
                                     Theme::TEXT_MUTED, rightX + rightW * 0.5f,
                                     card1Y - 38.f, sf::Text::Bold);
        }

        // ── Panel cards ──────────────────────────────────────────────────
        // Shared so the two cards can't drift apart typographically.
        auto drawPanelCard = [&](float y, float hoverT, sf::Color accent,
                                 const std::string& letter,
                                 const std::string& title,
                                 const std::string& subtitle)
        {
            sf::Color bg = Theme::lerp(Theme::BG_CARD, Theme::BG_CARD_HOVER, hoverT);
            Theme::drawCard(window, {cardX, y}, {cardW, cardH}, bg, 12.f);
            Theme::drawAccentBar(window, cardX, y, cardH, accent, 4.f);
            Theme::drawIconCircle(window, font, {cardX + 52.f, y + cardH * 0.5f},
                                  26.f, Theme::withAlpha(accent, 45), letter,
                                  accent, Theme::Type::HEADING);

            // Title above, subtitle below, with a real gap between them —
            // the two used to sit almost on top of each other.
            Theme::drawText(window, font, title, Theme::Type::TITLE,
                            Theme::TEXT_PRIMARY,
                            {cardX + 96.f, y + cardH * 0.5f - 34.f}, sf::Text::Bold);
            Theme::drawText(window, font, subtitle, Theme::Type::META,
                            Theme::TEXT_SECONDARY,
                            {cardX + 96.f, y + cardH * 0.5f + 8.f});

            Theme::drawCenteredText(window, font, ">", Theme::Type::HEADING,
                                    Theme::lerp(Theme::TEXT_MUTED, accent, hoverT),
                                    {{cardX + cardW - 44.f, y + cardH * 0.5f - 16.f},
                                     {32.f, 32.f}}, sf::Text::Bold);
        };

        drawPanelCard(card1Y, card1HoverT, Theme::ACCENT, "U", "User Panel",
                      "Login or register to browse bus routes");
        drawPanelCard(card2Y, card2HoverT, Theme::PURPLE, "A", "Admin Panel",
                      "Manage universities, buses and routes");

        exitBtn.draw(window);
        window.display();
    }
}