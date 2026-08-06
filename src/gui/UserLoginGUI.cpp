#include "UserLoginGUI.h"
#include "ScreenManager.h"
#include "Button.h"
#include "Theme.h"
#include "UserDashboardGUI.h"

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>

namespace {

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
constexpr float CARD_W    = 480.f;
}

namespace {
struct Layout
{
    float cX, cY, cW, cH, fX, fW;
};

Layout layoutFor(sf::Vector2f size)
{
    Layout L{};
    L.cW = std::min(CARD_W, size.x - 60.f);
    L.cH = CARD_H;
    L.cX = std::round((size.x - L.cW) * 0.5f);
    L.cY = std::round((size.y - L.cH) * 0.5f);
    L.fX = L.cX + 28.f;
    L.fW = L.cW - 56.f;
    return L;
}
}

UserLoginScreen::UserLoginScreen(sf::Font& font)
    : m_font(font),
      m_emailBox(font, {CARD_W - 56.f, FIELD_H}, {0.f, 0.f}),
      m_passBox(font, {CARD_W - 56.f, FIELD_H}, {0.f, 0.f})
{
    m_emailBox.setPlaceholder("yourname@gmail.com");
    m_passBox.setPlaceholder("Enter password");
    m_passBox.setPasswordMode(true);
}

void UserLoginScreen::onEnter()
{

    m_focus = F_NONE;
    m_errorText.clear();
    m_passBox.clear();
    m_emailBox.setFocused(false);
    m_passBox.setFocused(false);
}

void UserLoginScreen::step(int delta)
{
    if (m_focus == F_NONE) m_focus = (delta > 0) ? 0 : F_COUNT - 1;
    else                   m_focus = (m_focus + delta + F_COUNT) % F_COUNT;
}

void UserLoginScreen::submit()
{
    std::string errMsg;
    if (m_user.loginUser(m_emailBox.getText(), m_passBox.getText(), errMsg))
    {
        m_errorText.clear();
        m_app->push(std::make_unique<UserDashboardScreen>(m_font));
    }
    else
    {
        m_errorText = errMsg;
        m_focus     = F_PASS;
    }
}

void UserLoginScreen::prepare(sf::Vector2f size, sf::Vector2f mouse)
{
    Screen::prepare(size, mouse);

    const Layout L = layoutFor(size);

    m_emailBox.setPosition({L.fX, L.cY + Y_FIELD1});
    m_passBox.setPosition({L.fX, L.cY + Y_FIELD2});

    m_emailBox.setFocused(m_focus == F_EMAIL);
    m_passBox.setFocused(m_focus == F_PASS);
}

void UserLoginScreen::handleEvent(const sf::Event& event)
{
    const Layout L = layoutFor(m_size);

    const sf::FloatRect loginRect{{L.fX, L.cY + Y_BUTTONS}, {L.fW * 0.58f, FIELD_H}};
    const sf::FloatRect backRect {{L.fX + L.fW * 0.62f, L.cY + Y_BUTTONS},
                                  {L.fW * 0.36f, FIELD_H}};

    if (event.is<sf::Event::MouseButtonPressed>())
    {
        if (m_emailBox.getBounds().contains(m_mouse)) m_focus = F_EMAIL;
        if (m_passBox.getBounds().contains(m_mouse))  m_focus = F_PASS;

        if (loginRect.contains(m_mouse))
        {
            m_focus = F_LOGIN;
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

    if (const auto* kp = event.getIf<sf::Event::KeyPressed>())
    {
        using Key = sf::Keyboard::Key;
        bool consumed = true;

        switch (kp->code)
        {
            case Key::Enter:

                if      (m_focus == F_NONE)  m_focus = F_EMAIL;
                else if (m_focus == F_EMAIL) m_focus = F_PASS;
                else if (m_focus == F_BACK)  m_app->pop();
                else                         submit();
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

                if (m_focus == F_BACK) m_focus = F_LOGIN;
                else                   consumed = false;
                break;

            case Key::Right:
                if (m_focus == F_LOGIN) m_focus = F_BACK;
                else                    consumed = false;
                break;

            case Key::Escape:
                m_app->pop();
                break;

            default:
                consumed = false;
                break;
        }

        if (consumed) return;
    }

    if (event.is<sf::Event::TextEntered>() || event.is<sf::Event::KeyPressed>())
    {
        if      (m_focus == F_EMAIL) m_emailBox.handleEvent(event);
        else if (m_focus == F_PASS)  m_passBox.handleEvent(event);
    }
}

void UserLoginScreen::update(float dt)
{
    m_emailBox.update(dt);
    m_passBox.update(dt);

    const Layout L = layoutFor(m_size);

    const bool loginHot = sf::FloatRect{{L.fX, L.cY + Y_BUTTONS},
                                        {L.fW * 0.58f, FIELD_H}}.contains(m_mouse);
    const bool backHot  = sf::FloatRect{{L.fX + L.fW * 0.62f, L.cY + Y_BUTTONS},
                                        {L.fW * 0.36f, FIELD_H}}.contains(m_mouse);

    m_loginHoverT = Theme::approachHover(m_loginHoverT, loginHot, dt);
    m_backHoverT  = Theme::approachHover(m_backHoverT,  backHot,  dt);
}

void UserLoginScreen::skipAnimations()
{
    m_emailBox.settle();
    m_passBox.settle();
}

void UserLoginScreen::draw(sf::RenderTarget& target)
{
    const Layout L = layoutFor(m_size);

    Theme::drawBackdrop(target, m_size.x, m_size.y, Theme::ACCENT, Theme::ACCENT_INDIGO);

    Theme::drawCardElevated(target, {L.cX, L.cY}, {L.cW, L.cH}, Theme::BG_CARD, 16.f, 26.f, 15);

    Theme::fillRoundedRectV(target, {L.cX + 18.f, L.cY + 1.f}, {L.cW - 36.f, 3.f}, 1.5f,
                            Theme::ACCENT_CYAN, Theme::ACCENT);

    Theme::drawIconCircle(target, m_font, {L.cX + L.cW * 0.5f, L.cY + Y_ICON},
                          23.f, Theme::withAlpha(Theme::ACCENT, 60),
                          "U", Theme::ACCENT_HOVER, 19);

    Theme::drawTextHCentered(target, m_font, "User Login", Theme::Type::TITLE,
                             Theme::TEXT_PRIMARY, L.cX + L.cW * 0.5f, L.cY + Y_TITLE,
                             sf::Text::Bold);
    Theme::drawTextHCentered(target, m_font, "Sign in to track your campus buses",
                             Theme::Type::META, Theme::TEXT_MUTED,
                             L.cX + L.cW * 0.5f, L.cY + Y_SUB);

    Theme::drawSeparatorSoft(target, L.cX + 28.f, L.cY + Y_RULE, L.cW - 56.f);

    auto drawLabel = [&](const std::string& s, float y, bool active) {
        Theme::drawText(target, m_font, s, Theme::Type::LABEL,
                        active ? Theme::ACCENT_HOVER : Theme::TEXT_MUTED,
                        {L.fX + 2.f, L.cY + y}, sf::Text::Bold);
    };
    drawLabel("GMAIL ADDRESS", Y_LABEL1, m_focus == F_EMAIL);
    drawLabel("PASSWORD",      Y_LABEL2, m_focus == F_PASS);

    m_emailBox.draw(target);
    m_passBox.draw(target);

    Button loginBtn(m_font, "Login",  {L.fW * 0.58f, FIELD_H}, {L.fX, L.cY + Y_BUTTONS});
    Button backBtn (m_font, "< Back", {L.fW * 0.36f, FIELD_H},
                    {L.fX + L.fW * 0.62f, L.cY + Y_BUTTONS}, ButtonStyle::SECONDARY);
    loginBtn.setFocused(m_focus == F_LOGIN);
    backBtn.setFocused(m_focus == F_BACK);
    loginBtn.setHoverT(m_loginHoverT);
    backBtn.setHoverT(m_backHoverT);
    loginBtn.draw(target);
    backBtn.draw(target);

    if (!m_errorText.empty())
    {

        sf::CircleShape dot(3.f, 16);
        dot.setFillColor(Theme::DANGER_HOVER);
        dot.setPosition(Theme::px(L.fX + 2.f, L.cY + Y_ERROR + 5.f));
        target.draw(dot);
        Theme::drawText(target, m_font, m_errorText, Theme::Type::META,
                        Theme::DANGER_HOVER, {L.fX + 14.f, L.cY + Y_ERROR},
                        sf::Text::Bold);
    }
}
