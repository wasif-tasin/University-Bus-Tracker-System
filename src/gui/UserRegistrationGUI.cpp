#include "UserRegistrationGUI.h"
#include "ScreenManager.h"
#include "Button.h"
#include "Theme.h"

#include <SFML/Graphics.hpp>
#include <algorithm>

namespace
{
    constexpr float CARD_H = 424.f;
    constexpr float Y_ICON = 46.f;
    constexpr float Y_TITLE = 78.f;
    constexpr float Y_SUB = 110.f;
    constexpr float Y_RULE = 138.f;
    constexpr float Y_LABEL1 = 156.f;
    constexpr float Y_FIELD1 = 176.f;
    constexpr float Y_LABEL2 = 236.f;
    constexpr float Y_FIELD2 = 256.f;
    constexpr float Y_HINT = 310.f;
    constexpr float Y_BUTTONS = 336.f;
    constexpr float Y_INFO = 394.f;
    constexpr float FIELD_H = 46.f;
}

UserRegistrationScreen::UserRegistrationScreen(sf::Font &font)
    : m_font(font), m_emailBox(font, {100.f, FIELD_H}, {0.f, 0.f}), m_passBox(font, {100.f, FIELD_H}, {0.f, 0.f})
{
    m_emailBox.setPlaceholder("yourname@gmail.com");
    m_passBox.setPlaceholder("Choose a password");
    m_passBox.setPasswordMode(true);
}

void UserRegistrationScreen::onEnter()
{
    m_focus = F_NONE;
    m_infoText.clear();
    m_infoIsError = false;
    m_emailBox.setFocused(false);
}

void UserRegistrationScreen::step(int delta)
{
    if (m_focus == F_NONE)
        m_focus = (delta > 0) ? 0 : F_COUNT - 1;
    else
        m_focus = (m_focus + delta + F_COUNT) % F_COUNT;
}

void UserRegistrationScreen::submit()
{
    std::string errMsg;
    if (m_user.registerUser(m_emailBox.getText(), m_passBox.getText(), errMsg))
    {
        m_infoText = "Account created! You can now login.";
        m_infoIsError = false;
        m_emailBox.clear();
        m_passBox.clear();
        m_focus = F_NONE;
    }
    else
    {
        m_infoText = errMsg;
        m_infoIsError = true;
        m_focus = F_PASS;
    }
}

void UserRegistrationScreen::prepare(sf::Vector2f size, sf::Vector2f mouse)
{
    Screen::prepare(size, mouse);

    const float cW = std::min(480.f, size.x - 60.f);
    const float cH = CARD_H;
    const float cX = std::round((size.x - cW) * 0.5f);
    const float cY = std::round((size.y - cH) * 0.5f);
    const float fX = cX + 28.f;
    const float fW = cW - 56.f;

    m_emailBox.setPosition({fX, cY + Y_FIELD1});
    m_passBox.setPosition({fX, cY + Y_FIELD2});

    m_emailBox.setFocused(m_focus == F_EMAIL);
    m_passBox.setFocused(m_focus == F_PASS);
}

void UserRegistrationScreen::handleEvent(const sf::Event &event)
{
    const float cW = std::min(480.f, m_size.x - 60.f);
    const float cH = CARD_H;
    const float cX = std::round((m_size.x - cW) * 0.5f);
    const float cY = std::round((m_size.y - cH) * 0.5f);
    const float fX = cX + 28.f;
    const float fW = cW - 56.f;

    const sf::FloatRect regRect{{fX, cY + Y_BUTTONS}, {fW * 0.58f, FIELD_H}};
    const sf::FloatRect backRect{{fX + fW * 0.62f, cY + Y_BUTTONS}, {fW * 0.36f, FIELD_H}};

    if (event.is<sf::Event::MouseButtonPressed>())
    {
        if (m_emailBox.getBounds().contains(m_mouse))
            m_focus = F_EMAIL;
        if (m_passBox.getBounds().contains(m_mouse))
            m_focus = F_PASS;

        if (regRect.contains(m_mouse))
        {
            m_focus = F_REGISTER;
            submit();
            return;
        }
        if (backRect.contains(m_mouse))
        {
            m_app->pop();
            return;
        }

        m_emailBox.handleEvent(event);
        m_passBox.handleEvent(event);
    }

    if (const auto *kp = event.getIf<sf::Event::KeyPressed>())
    {
        using Key = sf::Keyboard::Key;
        bool consumed = true;

        switch (kp->code)
        {
        case Key::Enter:
            if (m_focus == F_NONE)
                m_focus = F_EMAIL;
            else if (m_focus == F_EMAIL)
                m_focus = F_PASS;
            else if (m_focus == F_BACK)
                m_app->pop();
            else
                submit();
            break;

        case Key::Tab:
            step(kp->shift ? -1 : 1);
            break;

        case Key::Down:
            step(1);
            break;

        case Key::Up:
            step(-1);
            break;

        case Key::Left:
            if (m_focus == F_BACK)
                m_focus = F_REGISTER;
            else
                consumed = false;
            break;

        case Key::Right:
            if (m_focus == F_REGISTER)
                m_focus = F_BACK;
            else
                consumed = false;
            break;

        case Key::Escape:
            m_app->pop();
            break;

        default:
            consumed = false;
            break;
        }

        if (consumed)
            return;
    }

    if (event.is<sf::Event::TextEntered>() || event.is<sf::Event::KeyPressed>())
    {
        if (m_focus == F_EMAIL)
            m_emailBox.handleEvent(event);
        else if (m_focus == F_PASS)
            m_passBox.handleEvent(event);
    }
}

void UserRegistrationScreen::update(float dt)
{
    m_emailBox.update(dt);
    m_passBox.update(dt);

    const float cW = std::min(480.f, m_size.x - 60.f);
    const float cY = std::round((m_size.y - CARD_H) * 0.5f);
    const float fX = std::round((m_size.x - cW) * 0.5f) + 28.f;
    const float fW = cW - 56.f;

    const bool regHot = sf::FloatRect{{fX, cY + Y_BUTTONS}, {fW * 0.58f, FIELD_H}}.contains(m_mouse);
    const bool backHot = sf::FloatRect{{fX + fW * 0.62f, cY + Y_BUTTONS}, {fW * 0.36f, FIELD_H}}.contains(m_mouse);

    m_regHoverT = Theme::approachHover(m_regHoverT, regHot, dt);
    m_backHoverT = Theme::approachHover(m_backHoverT, backHot, dt);
}

void UserRegistrationScreen::skipAnimations()
{
    m_emailBox.settle();
    m_passBox.settle();
}

void UserRegistrationScreen::draw(sf::RenderTarget &target)
{
    const float cW = std::min(480.f, m_size.x - 60.f);
    const float cH = CARD_H;
    const float cX = std::round((m_size.x - cW) * 0.5f);
    const float cY = std::round((m_size.y - cH) * 0.5f);
    const float fX = cX + 28.f;
    const float fW = cW - 56.f;

    Theme::drawBackdrop(target, m_size.x, m_size.y, Theme::SUCCESS, Theme::ACCENT);

    Theme::drawCardElevated(target, {cX, cY}, {cW, cH}, Theme::BG_CARD, 16.f, 26.f, 15);

    Theme::fillRoundedRectV(target, {cX + 18.f, cY + 1.f}, {cW - 36.f, 3.f}, 1.5f,
                            Theme::SUCCESS, Theme::SUCCESS_DARK);

    Theme::drawIconCircle(target, m_font, {cX + cW * 0.5f, cY + Y_ICON},
                          23.f, Theme::withAlpha(Theme::SUCCESS, 55),
                          "+", Theme::SUCCESS, 23);

    Theme::drawTextHCentered(target, m_font, "Create Account", Theme::Type::TITLE,
                             Theme::TEXT_PRIMARY, cX + cW * 0.5f, cY + Y_TITLE,
                             sf::Text::Bold);
    Theme::drawTextHCentered(target, m_font, "One account, every route on campus",
                             Theme::Type::META, Theme::TEXT_MUTED,
                             cX + cW * 0.5f, cY + Y_SUB);

    Theme::drawSeparatorSoft(target, cX + 28.f, cY + Y_RULE, cW - 56.f);

    auto drawLabel = [&](const std::string &s, float y, bool active)
    {
        Theme::drawText(target, m_font, s, Theme::Type::LABEL,
                        active ? Theme::SUCCESS : Theme::TEXT_MUTED,
                        {fX + 2.f, cY + y}, sf::Text::Bold);
    };
    drawLabel("GMAIL ADDRESS", Y_LABEL1, m_focus == F_EMAIL);
    drawLabel("PASSWORD", Y_LABEL2, m_focus == F_PASS);

    m_emailBox.draw(target);
    m_passBox.draw(target);

    Theme::drawText(target, m_font, "Must end in @gmail.com", Theme::Type::CAPTION,
                    Theme::TEXT_ROUTE, {fX + 2.f, cY + Y_HINT});

    Button regBtn(m_font, "Create Account", {fW * 0.58f, FIELD_H}, {fX, cY + Y_BUTTONS},
                  ButtonStyle::SUCCESS);
    Button backBtn(m_font, "< Back", {fW * 0.36f, FIELD_H},
                   {fX + fW * 0.62f, cY + Y_BUTTONS}, ButtonStyle::SECONDARY);
    regBtn.setFocused(m_focus == F_REGISTER);
    backBtn.setFocused(m_focus == F_BACK);
    regBtn.setHoverT(m_regHoverT);
    backBtn.setHoverT(m_backHoverT);
    regBtn.draw(target);
    backBtn.draw(target);

    if (!m_infoText.empty())
    {
        sf::Color c = m_infoIsError ? Theme::DANGER_HOVER : Theme::SUCCESS;
        sf::CircleShape dot(3.f, 16);
        dot.setFillColor(c);
        dot.setPosition(Theme::px(fX + 2.f, cY + Y_INFO + 5.f));
        target.draw(dot);
        Theme::drawText(target, m_font, m_infoText, Theme::Type::META, c,
                        {fX + 14.f, cY + Y_INFO}, sf::Text::Bold);
    }
}
