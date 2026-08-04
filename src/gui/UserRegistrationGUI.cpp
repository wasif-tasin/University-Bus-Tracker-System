#include "UserRegistrationGUI.h"
#include "Button.h"
#include "TextBox.h"
#include "Theme.h"
#include "User.h"

#include <SFML/Graphics.hpp>
#include <algorithm>

namespace {
// Keyboard focus order: Enter walks down this chain and fires the last stop.
enum Focus { F_EMAIL = 0, F_PASS, F_REGISTER, F_BACK, F_COUNT };

// Vertical rhythm inside the card, all relative to its top edge.
constexpr float CARD_H    = 424.f;
constexpr float Y_ICON    = 46.f;
constexpr float Y_TITLE   = 78.f;
constexpr float Y_SUB     = 110.f;
constexpr float Y_RULE    = 138.f;
constexpr float Y_LABEL1  = 156.f;
constexpr float Y_FIELD1  = 176.f;
constexpr float Y_LABEL2  = 236.f;
constexpr float Y_FIELD2  = 256.f;
constexpr float Y_HINT    = 310.f;
constexpr float Y_BUTTONS = 336.f;
constexpr float Y_INFO    = 394.f;
constexpr float FIELD_H   = 46.f;
}

void UserRegistrationGUI::run()
{
    sf::RenderWindow window(sf::VideoMode({720, 520}),
                            "University Bus Tracker - Register",
                            sf::Style::Default, sf::State::Windowed,
                            Theme::uiContext());
    window.setFramerateLimit(60);

    sf::Font font;
    if (!Theme::loadUIFont(font)) return;

    User user;
    std::string infoText;
    bool infoIsError = false;
    int  focus       = F_EMAIL;

    // Hover easing lives out here: the buttons are rebuilt every frame and so
    // cannot carry their own animation state.
    float regHoverT = 0.f, backHoverT = 0.f;

    const float ww = 720.f, wh = 520.f;
    const float cW = std::min(480.f, ww - 60.f);
    const float cH = CARD_H;
    const float cX = std::round((ww - cW) * 0.5f);
    const float cY = std::round((wh - cH) * 0.5f);
    const float fX = cX + 28.f;
    const float fW = cW - 56.f;

    TextBox emailBox(font, {fW, FIELD_H}, {fX, cY + Y_FIELD1});
    TextBox passBox (font, {fW, FIELD_H}, {fX, cY + Y_FIELD2});
    emailBox.setPlaceholder("yourname@gmail.com");
    passBox.setPlaceholder("Choose a password");
    passBox.setPasswordMode(true);
    emailBox.setFocused(true);

    auto submit = [&]() {
        std::string errMsg;
        if (user.registerUser(emailBox.getText(), passBox.getText(), errMsg)) {
            infoText    = "Account created! You can now login.";
            infoIsError = false;
            emailBox.clear();
            passBox.clear();
            focus = F_EMAIL;
        } else {
            infoText    = errMsg;
            infoIsError = true;
            focus       = F_PASS;
        }
    };

    while (window.isOpen())
    {
        emailBox.setFocused(focus == F_EMAIL);
        passBox.setFocused(focus == F_PASS);

        Button regBtn (font, "Create Account", {fW * 0.58f, FIELD_H}, {fX,              cY + Y_BUTTONS},
                       ButtonStyle::SUCCESS);
        Button backBtn(font, "< Back",         {fW * 0.36f, FIELD_H}, {fX + fW * 0.62f, cY + Y_BUTTONS},
                       ButtonStyle::SECONDARY);
        regBtn.setFocused(focus == F_REGISTER);
        backBtn.setFocused(focus == F_BACK);
        regBtn.setHoverT(regHoverT);
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

                if (regBtn.isClicked(window)) {
                    focus = F_REGISTER;
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
                        // field Enter presses Create Account, so the form never
                        // needs the mouse.
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
                        if (focus == F_BACK) focus = F_REGISTER;
                        else                 consumed = false;
                        break;

                    case Key::Right:
                        if (focus == F_REGISTER) focus = F_BACK;
                        else                     consumed = false;
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

        regBtn.update(window);
        backBtn.update(window);
        regHoverT  = regBtn.hoverT();
        backHoverT = backBtn.hoverT();

        window.clear(Theme::BG_DARK);
        Theme::drawBackdrop(window, ww, wh, Theme::SUCCESS, Theme::ACCENT);

        Theme::drawCardElevated(window, {cX, cY}, {cW, cH}, Theme::BG_CARD, 16.f, 26.f, 15);

        // Accent stripe that follows the card's rounded top edge.
        Theme::fillRoundedRectV(window, {cX + 18.f, cY + 1.f}, {cW - 36.f, 3.f}, 1.5f,
                                Theme::SUCCESS, Theme::SUCCESS_DARK);

        Theme::drawIconCircle(window, font, {cX + cW * 0.5f, cY + Y_ICON},
                              23.f, Theme::withAlpha(Theme::SUCCESS, 55),
                              "+", Theme::SUCCESS, 23);

        Theme::drawTextHCentered(window, font, "Create Account", Theme::Type::TITLE,
                                 Theme::TEXT_PRIMARY, cX + cW * 0.5f, cY + Y_TITLE,
                                 sf::Text::Bold);
        Theme::drawTextHCentered(window, font, "One account, every route on campus",
                                 Theme::Type::META, Theme::TEXT_MUTED,
                                 cX + cW * 0.5f, cY + Y_SUB);

        Theme::drawSeparatorSoft(window, cX + 28.f, cY + Y_RULE, cW - 56.f);

        auto drawLabel = [&](const std::string& s, float y, bool active) {
            Theme::drawText(window, font, s, Theme::Type::LABEL,
                            active ? Theme::SUCCESS : Theme::TEXT_MUTED,
                            {fX + 2.f, cY + y}, sf::Text::Bold);
        };
        drawLabel("GMAIL ADDRESS", Y_LABEL1, focus == F_EMAIL);
        drawLabel("PASSWORD",      Y_LABEL2, focus == F_PASS);

        emailBox.draw(window);
        passBox.draw(window);

        // Requirement moved out of the label into its own quiet hint line.
        Theme::drawText(window, font, "Must end in @gmail.com", Theme::Type::CAPTION,
                        Theme::TEXT_ROUTE, {fX + 2.f, cY + Y_HINT});

        regBtn.draw(window);
        backBtn.draw(window);

        if (!infoText.empty()) {
            sf::Color c = infoIsError ? Theme::DANGER_HOVER : Theme::SUCCESS;
            sf::CircleShape dot(3.f, 16);
            dot.setFillColor(c);
            dot.setPosition(Theme::px(fX + 2.f, cY + Y_INFO + 5.f));
            window.draw(dot);
            Theme::drawText(window, font, infoText, Theme::Type::META, c,
                            {fX + 14.f, cY + Y_INFO}, sf::Text::Bold);
        }

        window.display();
    }
}
