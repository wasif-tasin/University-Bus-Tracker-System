#include "UserPanelGUI.h"
#include "Button.h"
#include "TextBox.h"
#include "Theme.h"
#include "UserLoginGUI.h"
#include "UserRegistrationGUI.h"

#include <SFML/Graphics.hpp>
#include <algorithm>

namespace {
// Keyboard focus order: Up/Down/Tab move, Enter presses.
// Starts at F_NONE so nothing looks pre-selected before the user acts -- the
// ring only appears once they actually press a key.
enum Focus { F_NONE = -1, F_LOGIN = 0, F_REGISTER, F_BACK, F_COUNT };
}

void UserPanelGUI::run()
{
    sf::RenderWindow window(sf::VideoMode({720, 460}),
                            "University Bus Tracker - User Panel",
                            sf::Style::Default, sf::State::Windowed,
                            Theme::uiContext());
    window.setFramerateLimit(60);

    sf::Font font;
    if (!Theme::loadUIFont(font)) return;

    int focus = F_NONE;

    // Hover animation lives out here: the buttons are rebuilt every frame, so
    // they cannot carry their own easing state.
    float loginHoverT = 0.f, regHoverT = 0.f, backHoverT = 0.f;

    // Moving the mouse hands control back to the pointer, so the keyboard ring
    // and the hover highlight are never lit at the same time.
    auto step = [&](int delta) {
        if (focus == F_NONE) focus = (delta > 0) ? F_LOGIN : F_BACK;
        else                 focus = (focus + delta + F_COUNT) % F_COUNT;
    };

    while (window.isOpen())
    {
        auto sz  = window.getSize();
        float ww = static_cast<float>(sz.x);
        float wh = static_cast<float>(sz.y);

        float cardW  = std::min(420.f, ww - 60.f);
        float cardH  = 274.f;
        float cardX  = (ww - cardW) * 0.5f;
        float cardY  = (wh - cardH) * 0.5f - 10.f;

        float btnW   = cardW - 60.f;
        float btnH   = 50.f;
        float btnX   = cardX + 30.f;
        float loginY = cardY + 140.f;
        float regY   = cardY + 204.f;

        Button loginBtn(font, "User Login",   {btnW, btnH}, {btnX, loginY});
        Button regBtn  (font, "Register",     {btnW, btnH}, {btnX, regY},   ButtonStyle::SECONDARY);
        Button backBtn (font, "< Back",       {90.f, 34.f}, {20.f, 16.f},  ButtonStyle::GHOST);
        loginBtn.setFocused(focus == F_LOGIN);
        regBtn.setFocused(focus == F_REGISTER);
        backBtn.setFocused(focus == F_BACK);
        loginBtn.setHoverT(loginHoverT);
        regBtn.setHoverT(regHoverT);
        backBtn.setHoverT(backHoverT);

        auto activate = [&]() {
            if (focus == F_LOGIN) {
                UserLoginGUI ui;
                ui.run();
            } else if (focus == F_REGISTER) {
                UserRegistrationGUI ur;
                ur.run();
            } else if (focus == F_BACK) {
                window.close();
            }
        };

        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>()) window.close();
            Theme::syncViewToWindow(window, *event);

            // Pointer takes over: drop the keyboard ring so only one thing is lit.
            if (event->is<sf::Event::MouseMoved>()) focus = F_NONE;

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

            if (const auto* kp = event->getIf<sf::Event::KeyPressed>())
            {
                using Key = sf::Keyboard::Key;
                switch (kp->code)
                {
                    case Key::Enter:
                        // Enter before any arrow/Tab press means "the obvious
                        // action", which here is logging in.
                        if (focus == F_NONE) focus = F_LOGIN;
                        activate();
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

        loginBtn.update(window);
        regBtn.update(window);
        backBtn.update(window);

        // Carry the eased hover values into the next frame.
        loginHoverT = loginBtn.hoverT();
        regHoverT   = regBtn.hoverT();
        backHoverT  = backBtn.hoverT();

        window.clear(Theme::BG_DARK);
        Theme::drawBackdrop(window, ww, wh, Theme::ACCENT, Theme::ACCENT_INDIGO);

        Theme::drawCardElevated(window, {cardX, cardY}, {cardW, cardH},
                                Theme::BG_CARD, 16.f, 26.f, 15);

        Theme::fillRoundedRectV(window, {cardX + 18.f, cardY + 1.f}, {cardW - 36.f, 3.f},
                                1.5f, Theme::ACCENT_CYAN, Theme::ACCENT);

        Theme::drawIconCircle(window, font,
                              {cardX + cardW * 0.5f, cardY + 46.f},
                              23.f,
                              Theme::withAlpha(Theme::ACCENT, 60), "U",
                              Theme::ACCENT_HOVER, 19);

        Theme::drawTextHCentered(window, font, "User Panel", Theme::Type::TITLE,
                                 Theme::TEXT_PRIMARY, cardX + cardW * 0.5f,
                                 cardY + 76.f, sf::Text::Bold);

        Theme::drawSeparatorSoft(window, cardX + 30.f, cardY + 120.f, cardW - 60.f);

        loginBtn.draw(window);
        regBtn.draw(window);
        backBtn.draw(window);

        window.display();
    }
}
