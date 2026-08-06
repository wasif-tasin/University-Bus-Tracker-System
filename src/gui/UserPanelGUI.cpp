#include "UserPanelGUI.h"
#include "Button.h"
#include "ScreenManager.h"
#include "Theme.h"
#include "UserLoginGUI.h"
#include "UserRegistrationGUI.h"

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>

namespace
{

    enum Focus
    {
        F_NONE = -1,
        F_LOGIN = 0,
        F_REGISTER,
        F_BACK,
        F_COUNT
    };

    constexpr float CARD_H = 274.f;
    constexpr float BTN_H = 50.f;

    struct Layout
    {
        float cardX, cardY, cardW;
        float btnX, btnW, loginY, regY;
    };

    Layout layoutFor(sf::Vector2f size)
    {
        Layout L{};
        L.cardW = std::min(420.f, size.x - 60.f);
        L.cardX = (size.x - L.cardW) * 0.5f;
        L.cardY = (size.y - CARD_H) * 0.5f - 10.f;

        L.btnW = L.cardW - 60.f;
        L.btnX = L.cardX + 30.f;
        L.loginY = L.cardY + 140.f;
        L.regY = L.cardY + 204.f;
        return L;
    }
}

UserPanelScreen::UserPanelScreen(sf::Font &font)
    : m_font(font)
{
}

void UserPanelScreen::step(int delta)
{
    if (m_focus == F_NONE)
        m_focus = (delta > 0) ? F_LOGIN : F_BACK;
    else
        m_focus = (m_focus + delta + F_COUNT) % F_COUNT;
}

void UserPanelScreen::activate()
{
    if (m_focus == F_LOGIN)
        m_app->push(std::make_unique<UserLoginScreen>(m_font));
    else if (m_focus == F_REGISTER)
        m_app->push(std::make_unique<UserRegistrationScreen>(m_font));
    else if (m_focus == F_BACK)
        m_app->pop();
}

void UserPanelScreen::prepare(sf::Vector2f size, sf::Vector2f mouse)
{
    Screen::prepare(size, mouse);
}

void UserPanelScreen::handleEvent(const sf::Event &event)
{
    const Layout L = layoutFor(m_size);

    const sf::FloatRect loginRect{{L.btnX, L.loginY}, {L.btnW, BTN_H}};
    const sf::FloatRect regRect{{L.btnX, L.regY}, {L.btnW, BTN_H}};
    const sf::FloatRect backRect{{20.f, 16.f}, {90.f, 34.f}};

    if (event.is<sf::Event::MouseMoved>())
        m_focus = F_NONE;

    if (event.is<sf::Event::MouseButtonPressed>())
    {
        if (loginRect.contains(m_mouse))
        {
            m_app->push(std::make_unique<UserLoginScreen>(m_font));
            return;
        }
        if (regRect.contains(m_mouse))
        {
            m_app->push(std::make_unique<UserRegistrationScreen>(m_font));
            return;
        }
        if (backRect.contains(m_mouse))
        {
            m_app->pop();
            return;
        }
    }

    if (const auto *kp = event.getIf<sf::Event::KeyPressed>())
    {
        using Key = sf::Keyboard::Key;
        switch (kp->code)
        {
        case Key::Enter:

            if (m_focus == F_NONE)
                m_focus = F_LOGIN;
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
            m_app->pop();
            break;

        default:
            break;
        }
    }
}

void UserPanelScreen::update(float dt)
{
    const Layout L = layoutFor(m_size);

    const bool loginHot = sf::FloatRect{{L.btnX, L.loginY}, {L.btnW, BTN_H}}.contains(m_mouse);
    const bool regHot = sf::FloatRect{{L.btnX, L.regY}, {L.btnW, BTN_H}}.contains(m_mouse);
    const bool backHot = sf::FloatRect{{20.f, 16.f}, {90.f, 34.f}}.contains(m_mouse);

    m_loginHoverT = Theme::approachHover(m_loginHoverT, loginHot, dt);
    m_regHoverT = Theme::approachHover(m_regHoverT, regHot, dt);
    m_backHoverT = Theme::approachHover(m_backHoverT, backHot, dt);
}

void UserPanelScreen::skipAnimations()
{
    const Layout L = layoutFor(m_size);

    m_loginHoverT = sf::FloatRect{{L.btnX, L.loginY}, {L.btnW, BTN_H}}.contains(m_mouse) ? 1.f : 0.f;
    m_regHoverT = sf::FloatRect{{L.btnX, L.regY}, {L.btnW, BTN_H}}.contains(m_mouse) ? 1.f : 0.f;
    m_backHoverT = sf::FloatRect{{20.f, 16.f}, {90.f, 34.f}}.contains(m_mouse) ? 1.f : 0.f;
}

void UserPanelScreen::draw(sf::RenderTarget &target)
{
    const Layout L = layoutFor(m_size);

    Theme::drawBackdrop(target, m_size.x, m_size.y, Theme::ACCENT, Theme::ACCENT_INDIGO);

    Theme::drawCardElevated(target, {L.cardX, L.cardY}, {L.cardW, CARD_H},
                            Theme::BG_CARD, 16.f, 26.f, 15);

    Theme::fillRoundedRectV(target, {L.cardX + 18.f, L.cardY + 1.f}, {L.cardW - 36.f, 3.f},
                            1.5f, Theme::ACCENT_CYAN, Theme::ACCENT);

    Theme::drawIconCircle(target, m_font,
                          {L.cardX + L.cardW * 0.5f, L.cardY + 46.f}, 23.f,
                          Theme::withAlpha(Theme::ACCENT, 60), "U",
                          Theme::ACCENT_HOVER, 19);

    Theme::drawTextHCentered(target, m_font, "User Panel", Theme::Type::TITLE,
                             Theme::TEXT_PRIMARY, L.cardX + L.cardW * 0.5f,
                             L.cardY + 76.f, sf::Text::Bold);

    Theme::drawSeparatorSoft(target, L.cardX + 30.f, L.cardY + 120.f, L.cardW - 60.f);

    Button loginBtn(m_font, "User Login", {L.btnW, BTN_H}, {L.btnX, L.loginY});
    Button regBtn(m_font, "Register", {L.btnW, BTN_H}, {L.btnX, L.regY},
                  ButtonStyle::SECONDARY);
    Button backBtn(m_font, "< Back", {90.f, 34.f}, {20.f, 16.f},
                   ButtonStyle::GHOST);

    loginBtn.setFocused(m_focus == F_LOGIN);
    regBtn.setFocused(m_focus == F_REGISTER);
    backBtn.setFocused(m_focus == F_BACK);
    loginBtn.setHoverT(m_loginHoverT);
    regBtn.setHoverT(m_regHoverT);
    backBtn.setHoverT(m_backHoverT);

    loginBtn.draw(target);
    regBtn.draw(target);
    backBtn.draw(target);
}
