#include "GUI.h"
#include "Button.h"
#include "Theme.h"
#include "AdminLoginGUI.h"
#include "UserPanelGUI.h"

#include <SFML/Graphics.hpp>
#include <algorithm>

namespace {
// Keyboard focus order: Up/Down/Tab move, Enter opens.
// Starts at F_NONE so nothing looks pre-selected before the user acts.
enum Focus { F_NONE = -1, F_USER = 0, F_ADMIN, F_EXIT, F_COUNT };
}

void GUI::run()
{
    sf::RenderWindow window(sf::VideoMode({980, 620}),
                            "University Bus Tracker",
                            sf::Style::Default, sf::State::Windowed,
                            Theme::uiContext());
    window.setFramerateLimit(60);

    sf::Font font;
    if (!Theme::loadUIFont(font))
    {
        return;
    }

    float card1HoverT = 0.f;
    float card2HoverT = 0.f;
    float exitHoverT  = 0.f;
    int focus = F_NONE;

    auto step = [&](int delta) {
        if (focus == F_NONE) focus = (delta > 0) ? F_USER : F_EXIT;
        else                 focus = (focus + delta + F_COUNT) % F_COUNT;
    };

    while (window.isOpen())
    {
        auto sz = window.getSize();
        float ww = static_cast<float>(sz.x);
        float wh = static_cast<float>(sz.y);
        float leftW = ww * 0.38f;
        float rightX = leftW + 2.f;
        float rightW = ww - rightX;

        const float cardW = std::min(rightW - 50.f, 500.f);
        const float cardH = 120.f;
        const float cardGap = 22.f;
        const float totalH = 2.f * cardH + cardGap;
        float cardX = rightX + (rightW - cardW) * 0.5f;
        float card1Y = (wh - totalH) * 0.5f;
        float card2Y = card1Y + cardH + cardGap;
        Button exitBtn(font, "Exit", {90.f, 34.f},
                       {ww - 110.f, wh - 52.f}, ButtonStyle::GHOST);
        exitBtn.setFocused(focus == F_EXIT);
        exitBtn.setHoverT(exitHoverT);
        auto mp = sf::Mouse::getPosition(window);
        bool c1h = mp.x >= (int)cardX && mp.x < (int)(cardX + cardW) &&
                   mp.y >= (int)card1Y && mp.y < (int)(card1Y + cardH);
        bool c2h = mp.x >= (int)cardX && mp.x < (int)(cardX + cardW) &&
                   mp.y >= (int)card2Y && mp.y < (int)(card2Y + cardH);

        // Same asymmetric easing as Button: snappy in, gentler out.
        auto ease = [](float t, bool on) {
            float rate = on ? 0.28f : 0.16f;
            float v = t + ((on ? 1.f : 0.f) - t) * rate;
            return (std::abs((on ? 1.f : 0.f) - v) < 0.005f) ? (on ? 1.f : 0.f) : v;
        };
        card1HoverT = ease(card1HoverT, c1h);
        card2HoverT = ease(card2HoverT, c2h);

        auto openUserPanel = [&]() {
            UserPanelGUI up;
            up.run();
        };
        auto openAdminPanel = [&]() {
            AdminLoginGUI al;
            al.run();
        };

        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
            Theme::syncViewToWindow(window, *event);

            // Pointer takes over: drop the keyboard ring so only one thing is lit.
            if (event->is<sf::Event::MouseMoved>()) focus = F_NONE;

            if (event->is<sf::Event::MouseButtonPressed>())
            {
                if (c1h)
                {
                    openUserPanel();
                }
                if (c2h)
                {
                    openAdminPanel();
                }
                if (exitBtn.isClicked(window))
                    window.close();
            }

            if (const auto* kp = event->getIf<sf::Event::KeyPressed>())
            {
                using Key = sf::Keyboard::Key;
                switch (kp->code)
                {
                    case Key::Enter:
                        // Enter before any arrow/Tab press means "the obvious
                        // action", which here is the user panel.
                        if (focus == F_NONE) focus = F_USER;
                        if      (focus == F_USER)  openUserPanel();
                        else if (focus == F_ADMIN) openAdminPanel();
                        else                       window.close();
                        break;

                    case Key::Tab:
                        step(kp->shift ? -1 : 1);
                        break;

                    case Key::Down:
                    case Key::Right:
                        step(1);
                        break;

                    case Key::Up:
                    case Key::Left:
                        step(-1);
                        break;

                    case Key::Escape:
                        window.close();
                        break;

                    default:
                        break;
                }
            }
        }

        exitBtn.update(window);
        exitHoverT = exitBtn.hoverT();
        window.clear(Theme::BG_DARK);

        // Left brand rail: its own deeper gradient plus an ambient glow behind
        // the logo, so the split reads as two distinct surfaces.
        Theme::drawGradientRect(window, {0.f, 0.f}, {leftW, wh},
                                sf::Color(20, 30, 58), sf::Color(9, 13, 26));
        Theme::drawRadialGlow(window, {leftW * 0.5f, wh * 0.42f}, leftW * 1.05f,
                              Theme::ACCENT, 34);

        // Right pane sits slightly darker than the rail.
        Theme::drawGradientRect(window, {rightX, 0.f}, {rightW, wh},
                                Theme::BG_DARK, Theme::BG_DEEP);
        Theme::drawRadialGlow(window, {rightX + rightW * 0.85f, wh * 0.9f},
                              rightW * 0.9f, Theme::PURPLE, 26);

        // Glowing seam between the two panes.
        Theme::fillRoundedRectV(window, {leftW, 0.f}, {2.f, wh}, 0.f,
                                Theme::withAlpha(Theme::ACCENT, 90),
                                Theme::withAlpha(Theme::PURPLE, 70));

        sf::RectangleShape topAccent({leftW, 3.f});
        topAccent.setFillColor(Theme::ACCENT);
        window.draw(topAccent);
        float iconCX = leftW * 0.5f;
        float iconCY = wh * 0.5f - 60.f;
        Theme::fillRoundedRect(window, {iconCX - 40.f, iconCY - 40.f},
                               {80.f, 80.f}, 40.f,
                               Theme::withAlpha(Theme::ACCENT, 30));
        Theme::drawIconCircle(window, font, {iconCX, iconCY},
                              30.f, Theme::withAlpha(Theme::ACCENT, 70),
                              "B", Theme::ACCENT_HOVER, 26);

        {
            Theme::drawTextHCentered(window, font, "University", Theme::Type::DISPLAY,
                                     Theme::TEXT_PRIMARY, iconCX, iconCY + 56.f,
                                     sf::Text::Bold);
            Theme::drawTextHCentered(window, font, "Bus Tracker", Theme::Type::DISPLAY,
                                     Theme::ACCENT, iconCX, iconCY + 94.f,
                                     sf::Text::Bold);
        }

        {
            Theme::drawTextHCentered(window, font, "Your campus transport,",
                                     Theme::Type::META, Theme::TEXT_SECONDARY,
                                     iconCX, iconCY + 142.f);
            Theme::drawTextHCentered(window, font, "simplified.",
                                     Theme::Type::META, Theme::TEXT_SECONDARY,
                                     iconCX, iconCY + 164.f);
        }

        Theme::drawSeparatorSoft(window, leftW * 0.15f, wh - 42.f, leftW * 0.7f);
        {
            Theme::drawTextHCentered(window, font, "v1.0  |  SFML 3", Theme::Type::CAPTION,
                                     Theme::TEXT_MUTED, iconCX, wh - 30.f);
        }
        {
            Theme::drawTextHCentered(window, font, "SELECT A PANEL", Theme::Type::LABEL,
                                     Theme::TEXT_MUTED, rightX + rightW * 0.5f,
                                     card1Y - 38.f, sf::Text::Bold);
        }
        auto drawPanelCard = [&](float y, float hoverT, bool focused, sf::Color accent,
                                 const std::string &letter,
                                 const std::string &title,
                                 const std::string &subtitle)
        {
            // A keyboard-focused card lights up exactly like a hovered one, so
            // the user can always see what Enter is about to open.
            float raw = std::max(hoverT, focused ? 1.f : 0.f);
            float lit = raw * raw * (3.f - 2.f * raw);   // smoothstep

            // Cards lift toward the cursor, same language as the buttons.
            float lift = 3.f * lit;
            float cy   = y - lift;

            if (focused)
                Theme::drawGlow(window, {cardX, cy}, {cardW, cardH}, 14.f, accent,
                                12.f, 20, 5);
            else
                Theme::drawShadow(window, {cardX, cy}, {cardW, cardH}, 14.f,
                                  14.f + 8.f * lit,
                                  static_cast<std::uint8_t>(14 + 8 * lit),
                                  6.f + 3.f * lit, 6);

            sf::Color bg = Theme::lerp(Theme::BG_CARD, Theme::BG_CARD_HOVER, lit);

            // Rim brightens with the lift: a slightly larger rounded rect that
            // the card is then drawn on top of, leaving a 1.5px ring.
            if (lit > 0.01f)
                Theme::fillRoundedRect(window, {cardX - 1.5f, cy - 1.5f},
                                       {cardW + 3.f, cardH + 3.f}, 15.5f,
                                       Theme::withAlpha(accent,
                                           static_cast<std::uint8_t>(130 * lit)));

            Theme::drawCard(window, {cardX, cy}, {cardW, cardH}, bg, 14.f);

            Theme::drawAccentBar(window, cardX, cy, cardH, accent, 4.f);
            Theme::drawIconCircle(window, font, {cardX + 56.f, cy + cardH * 0.5f},
                                  26.f, Theme::withAlpha(accent, 55), letter,
                                  Theme::lerp(accent, sf::Color::White, 0.35f),
                                  Theme::Type::HEADING);
            Theme::drawText(window, font, title, Theme::Type::TITLE,
                            Theme::TEXT_PRIMARY,
                            {cardX + 100.f, cy + cardH * 0.5f - 34.f}, sf::Text::Bold);
            Theme::drawText(window, font, subtitle, Theme::Type::META,
                            Theme::TEXT_SECONDARY,
                            {cardX + 100.f, cy + cardH * 0.5f + 8.f});

            // Chevron slides right as the card lights up.
            Theme::drawCenteredText(window, font, ">", Theme::Type::HEADING,
                                    Theme::lerp(Theme::TEXT_MUTED, accent, lit),
                                    {{cardX + cardW - 44.f + 5.f * lit,
                                      cy + cardH * 0.5f - 16.f},
                                     {32.f, 32.f}},
                                    sf::Text::Bold);
        };

        drawPanelCard(card1Y, card1HoverT, focus == F_USER, Theme::ACCENT, "U", "User Panel",
                      "Login or register to browse bus routes");
        drawPanelCard(card2Y, card2HoverT, focus == F_ADMIN, Theme::PURPLE, "A", "Admin Panel",
                      "Manage universities, buses and routes");

        exitBtn.draw(window);
        window.display();
    }
}