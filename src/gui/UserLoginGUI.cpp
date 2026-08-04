#include "UserLoginGUI.h"
#include "Button.h"
#include "TextBox.h"
#include "Theme.h"
#include "User.h"
#include "UserDashboardGUI.h"

#include <SFML/Graphics.hpp>
#include <algorithm>

namespace {
// Keyboard focus order: Enter walks down this chain and fires the last stop.
enum Focus { F_EMAIL = 0, F_PASS, F_LOGIN, F_BACK, F_COUNT };

// Vertical rhythm inside the card, all relative to its top edge.
constexpr float CARD_H    = 408.f;
constexpr float Y_ICON    = 46.f;
constexpr float Y_TITLE   = 78.f;
constexpr float Y_SUB     = 110.f;
constexpr float Y_RULE    = 138.f;
constexpr float Y_LABEL1  = 156.f;
constexpr float Y_FIELD1  = 176.f;
constexpr float Y_LABEL2  = 236.f;
constexpr float Y_FIELD2  = 256.f;
constexpr float Y_BUTTONS = 324.f;
constexpr float Y_ERROR   = 382.f;
constexpr float FIELD_H   = 46.f;
}

void UserLoginGUI::run()
{
    sf::RenderWindow window(sf::VideoMode({720, 500}),
                            "University Bus Tracker - User Login",
                            sf::Style::Default, sf::State::Windowed,
                            Theme::uiContext());
    window.setFramerateLimit(60);

    sf::Font font;
    if (!Theme::loadUIFont(font)) return;

    User user;
    std::string errorText;
    int focus = F_EMAIL;

    // Hover easing lives out here: the buttons are rebuilt every frame and so
    // cannot carry their own animation state.
    float loginHoverT = 0.f, backHoverT = 0.f;

    const float ww = 720.f, wh = 500.f;
    const float cW = std::min(480.f, ww - 60.f);
    const float cH = CARD_H;
    const float cX = std::round((ww - cW) * 0.5f);
    const float cY = std::round((wh - cH) * 0.5f);
    const float fX = cX + 28.f;
    const float fW = cW - 56.f;

    TextBox emailBox(font, {fW, FIELD_H}, {fX, cY + Y_FIELD1});
    TextBox passBox (font, {fW, FIELD_H}, {fX, cY + Y_FIELD2});
    emailBox.setPlaceholder("yourname@gmail.com");
    passBox.setPlaceholder("Enter password");
    passBox.setPasswordMode(true);
    emailBox.setFocused(true);

    auto submit = [&]() {
        std::string errMsg;
        if (user.loginUser(emailBox.getText(), passBox.getText(), errMsg)) {
            errorText = "";
            UserDashboardGUI dash;
            dash.run();
        } else {
            errorText = errMsg;
            focus = F_PASS;
        }
    };

    while (window.isOpen())
    {
        emailBox.setFocused(focus == F_EMAIL);
        passBox.setFocused(focus == F_PASS);

        Button loginBtn(font, "Login",  {fW * 0.58f, FIELD_H}, {fX,              cY + Y_BUTTONS});
        Button backBtn (font, "< Back", {fW * 0.36f, FIELD_H}, {fX + fW * 0.62f, cY + Y_BUTTONS},
                        ButtonStyle::SECONDARY);
        loginBtn.setFocused(focus == F_LOGIN);
        backBtn.setFocused(focus == F_BACK);
        loginBtn.setHoverT(loginHoverT);
        backBtn.setHoverT(backHoverT);

        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>()) window.close();
            Theme::syncViewToWindow(window, *event);
            if (event->is<sf::Event::MouseButtonPressed>())
            {
                auto mp = sf::Mouse::getPosition(window);
                float mx = static_cast<float>(mp.x);
                float my = static_cast<float>(mp.y);

                if (emailBox.getBounds().contains({mx, my})) { focus = F_EMAIL; }
                if (passBox.getBounds().contains({mx, my}))  { focus = F_PASS;  }

                if (loginBtn.isClicked(window)) {
                    focus = F_LOGIN;
                    submit();
                }
                if (backBtn.isClicked(window)) window.close();

                emailBox.handleEvent(*event);
                passBox.handleEvent(*event);
            }

            if (const auto* kp = event->getIf<sf::Event::KeyPressed>())
            {
                using Key = sf::Keyboard::Key;
                bool consumed = true;

                switch (kp->code)
                {
                    case Key::Enter:
                        // Finish a field -> drop to the next one; on the last
                        // field Enter presses Login, so the form never needs
                        // the mouse.
                        if      (focus == F_EMAIL) focus = F_PASS;
                        else if (focus == F_BACK)  window.close();
                        else                       submit();
                        break;

                    case Key::Tab:
                        focus = kp->shift ? (focus + F_COUNT - 1) % F_COUNT
                                          : (focus + 1) % F_COUNT;
                        break;

                    case Key::Down:
                        focus = (focus + 1) % F_COUNT;
                        break;

                    case Key::Up:
                        focus = (focus + F_COUNT - 1) % F_COUNT;
                        break;

                    case Key::Left:
                        // Only steer between the buttons; inside a field the
                        // arrows still belong to the caret.
                        if (focus == F_BACK) focus = F_LOGIN;
                        else                 consumed = false;
                        break;

                    case Key::Right:
                        if (focus == F_LOGIN) focus = F_BACK;
                        else                  consumed = false;
                        break;

                    case Key::Escape:
                        window.close();
                        break;

                    default:
                        consumed = false;
                        break;
                }

                if (consumed) continue;
            }

            if (event->is<sf::Event::TextEntered>() ||
                event->is<sf::Event::KeyPressed>()) {
                if      (focus == F_EMAIL) emailBox.handleEvent(*event);
                else if (focus == F_PASS)  passBox.handleEvent(*event);
            }
        }

        loginBtn.update(window);
        backBtn.update(window);
        loginHoverT = loginBtn.hoverT();
        backHoverT  = backBtn.hoverT();

        window.clear(Theme::BG_DARK);
        Theme::drawBackdrop(window, ww, wh, Theme::ACCENT, Theme::ACCENT_INDIGO);

        Theme::drawCardElevated(window, {cX, cY}, {cW, cH}, Theme::BG_CARD, 16.f, 26.f, 15);

        // Accent stripe that follows the card's rounded top edge.
        Theme::fillRoundedRectV(window, {cX + 18.f, cY + 1.f}, {cW - 36.f, 3.f}, 1.5f,
                                Theme::ACCENT_CYAN, Theme::ACCENT);

        Theme::drawIconCircle(window, font, {cX + cW * 0.5f, cY + Y_ICON},
                              23.f, Theme::withAlpha(Theme::ACCENT, 60),
                              "U", Theme::ACCENT_HOVER, 19);

        Theme::drawTextHCentered(window, font, "User Login", Theme::Type::TITLE,
                                 Theme::TEXT_PRIMARY, cX + cW * 0.5f, cY + Y_TITLE,
                                 sf::Text::Bold);
        Theme::drawTextHCentered(window, font, "Sign in to track your campus buses",
                                 Theme::Type::META, Theme::TEXT_MUTED,
                                 cX + cW * 0.5f, cY + Y_SUB);

        Theme::drawSeparatorSoft(window, cX + 28.f, cY + Y_RULE, cW - 56.f);

        auto drawLabel = [&](const std::string& s, float y, bool active) {
            Theme::drawText(window, font, s, Theme::Type::LABEL,
                            active ? Theme::ACCENT_HOVER : Theme::TEXT_MUTED,
                            {fX + 2.f, cY + y}, sf::Text::Bold);
        };
        drawLabel("GMAIL ADDRESS", Y_LABEL1, focus == F_EMAIL);
        drawLabel("PASSWORD",      Y_LABEL2, focus == F_PASS);

        emailBox.draw(window);
        passBox.draw(window);
        loginBtn.draw(window);
        backBtn.draw(window);

        if (!errorText.empty()) {
            // Small red dot in front of the message so errors read at a glance.
            sf::CircleShape dot(3.f, 16);
            dot.setFillColor(Theme::DANGER_HOVER);
            dot.setPosition(Theme::px(fX + 2.f, cY + Y_ERROR + 5.f));
            window.draw(dot);
            Theme::drawText(window, font, errorText, Theme::Type::META,
                            Theme::DANGER_HOVER, {fX + 14.f, cY + Y_ERROR},
                            sf::Text::Bold);
        }

        window.display();
    }
}
