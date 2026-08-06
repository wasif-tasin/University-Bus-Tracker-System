#include "Button.h"
#include "Theme.h"

#include <algorithm>

Button::Button(sf::Font &f, const std::string &label, sf::Vector2f size,
               sf::Vector2f pos, ButtonStyle style, float radius)
    : m_pos(pos), m_size(size), m_font(f), m_label(label),
      m_style(style), m_radius(radius),
      m_hoverT(0.f), m_hovered(false), m_pressed(false), m_focused(false)
{
}

sf::Color Button::idleColor() const
{
    switch (m_style)
    {
    case ButtonStyle::PRIMARY:
        return Theme::ACCENT;
    case ButtonStyle::SECONDARY:
        return Theme::BG_CARD;
    case ButtonStyle::DANGER:
        return Theme::DANGER;
    case ButtonStyle::SUCCESS:
        return Theme::SUCCESS_DARK;
    case ButtonStyle::GHOST:
        return sf::Color(0, 0, 0, 0);
    }
    return Theme::ACCENT;
}

sf::Color Button::hoverColor() const
{
    switch (m_style)
    {
    case ButtonStyle::PRIMARY:
        return Theme::ACCENT_HOVER;
    case ButtonStyle::SECONDARY:
        return Theme::ITEM_HOVER;
    case ButtonStyle::DANGER:
        return Theme::DANGER_HOVER;
    case ButtonStyle::SUCCESS:
        return Theme::SUCCESS;
    case ButtonStyle::GHOST:
        return Theme::ITEM_HOVER;
    }
    return Theme::ACCENT_HOVER;
}

sf::Color Button::pressedColor() const
{
    switch (m_style)
    {
    case ButtonStyle::PRIMARY:
        return Theme::ACCENT_PRESSED;
    case ButtonStyle::SECONDARY:
        return Theme::BG_DARK;
    case ButtonStyle::DANGER:
        return Theme::DANGER_DARK;
    case ButtonStyle::SUCCESS:
        return Theme::SUCCESS_DARK;
    case ButtonStyle::GHOST:
        return Theme::BG_DARK;
    }
    return Theme::ACCENT_PRESSED;
}

void Button::draw(sf::RenderTarget &target)
{

    float raw = std::max(m_hoverT, m_focused ? 1.f : 0.f);

    float lit = Theme::smoothstep01(raw);

    sf::Color base = m_pressed
                         ? pressedColor()
                         : Theme::lerp(idleColor(), hoverColor(), lit);

    const bool outlined = (m_style == ButtonStyle::SECONDARY ||
                           m_style == ButtonStyle::GHOST);

    float lift = m_pressed ? 0.f : 2.f * lit;
    sf::Vector2f pos{m_pos.x, m_pos.y - lift};

    if (m_focused)
    {
        Theme::drawGlow(target, pos, m_size, m_radius,
                        Theme::lerp(hoverColor(), sf::Color::White, 0.35f),
                        9.f, 22, 5);
    }
    else if (!outlined && !m_pressed)
    {
        Theme::drawShadow(target, pos, m_size, m_radius,
                          9.f + 5.f * lit,
                          static_cast<std::uint8_t>(15 + 9 * lit),
                          3.f + 2.f * lit, 5);
    }
    else if (outlined && lit > 0.01f)
    {

        Theme::drawGlow(target, pos, m_size, m_radius, Theme::ACCENT,
                        7.f, static_cast<std::uint8_t>(14 * lit), 4);
    }

    float borderW = 0.f;
    sf::Color borderC = sf::Color::Transparent;
    if (outlined)
    {
        borderW = 1.5f;
        borderC = Theme::lerp(Theme::BORDER_IDLE, Theme::ACCENT, lit * 0.85f);
    }
    else if (m_focused)
    {
        borderW = 1.5f;
        borderC = Theme::lerp(base, sf::Color::White, 0.55f);
    }
    else if (lit > 0.01f)
    {

        borderW = 1.5f;
        borderC = Theme::withAlpha(Theme::lerp(base, sf::Color::White, 0.45f),
                                   static_cast<std::uint8_t>(150 * lit));
    }

    sf::Color top = Theme::lerp(base, sf::Color::White, m_pressed ? 0.02f : 0.14f);
    sf::Color bot = Theme::lerp(base, sf::Color::Black, m_pressed ? 0.04f : 0.12f);
    Theme::drawRoundedRectV(target, pos, m_size, m_radius, top, bot, borderW, borderC);

    if (!outlined && !m_pressed && m_size.y > 12.f)
    {
        Theme::fillRoundedRectV(target,
                                {pos.x + 1.f, pos.y + 1.f},
                                {m_size.x - 2.f, m_size.y * 0.45f},
                                m_radius,
                                sf::Color(255, 255, 255,
                                          static_cast<std::uint8_t>(26 + 18 * lit)),
                                sf::Color(255, 255, 255, 0));
    }

    sf::FloatRect labelBox{pos, m_size};
    if (m_pressed)
        labelBox.position.y += 1.f;
    Theme::drawCenteredText(target, m_font, m_label, Theme::Type::BODY,
                            Theme::TEXT_PRIMARY, labelBox, sf::Text::Bold);
}

void Button::update(sf::Vector2f mouse, float dt)
{
    m_hovered = sf::FloatRect{m_pos, m_size}.contains(mouse);

    m_hoverT = Theme::approachHover(m_hoverT, m_hovered, dt);

    m_pressed = m_hovered && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
}

bool Button::isClicked(sf::Vector2f mouse) const
{
    return sf::FloatRect{m_pos, m_size}.contains(mouse);
}

void Button::settle() { m_hoverT = m_hovered ? 1.f : 0.f; }

void Button::setLabel(const std::string &l) { m_label = l; }
void Button::setStyle(ButtonStyle s) { m_style = s; }
void Button::setPosition(sf::Vector2f p) { m_pos = p; }
void Button::setFocused(bool f) { m_focused = f; }
bool Button::isFocused() const { return m_focused; }
void Button::setHoverT(float t) { m_hoverT = std::clamp(t, 0.f, 1.f); }
float Button::hoverT() const { return m_hoverT; }
sf::FloatRect Button::getBounds() const { return {m_pos, m_size}; }
