#include "Button.h"
#include "Theme.h"

#include <algorithm>

Button::Button(sf::Font& f, const std::string& label, sf::Vector2f size,
               sf::Vector2f pos, ButtonStyle style, float radius)
    : m_pos(pos), m_size(size), m_font(f), m_label(label),
      m_style(style), m_radius(radius),
      m_hoverT(0.f), m_hovered(false), m_pressed(false), m_focused(false)
{}

sf::Color Button::idleColor() const {
    switch (m_style) {
        case ButtonStyle::PRIMARY:   return Theme::ACCENT;
        case ButtonStyle::SECONDARY: return Theme::BG_CARD;
        case ButtonStyle::DANGER:    return Theme::DANGER;
        case ButtonStyle::SUCCESS:   return Theme::SUCCESS_DARK;
        case ButtonStyle::GHOST:     return sf::Color(0, 0, 0, 0);
    }
    return Theme::ACCENT;
}

sf::Color Button::hoverColor() const {
    switch (m_style) {
        case ButtonStyle::PRIMARY:   return Theme::ACCENT_HOVER;
        case ButtonStyle::SECONDARY: return Theme::ITEM_HOVER;
        case ButtonStyle::DANGER:    return Theme::DANGER_HOVER;
        case ButtonStyle::SUCCESS:   return Theme::SUCCESS;
        case ButtonStyle::GHOST:     return Theme::ITEM_HOVER;
    }
    return Theme::ACCENT_HOVER;
}

sf::Color Button::pressedColor() const {
    switch (m_style) {
        case ButtonStyle::PRIMARY:   return Theme::ACCENT_PRESSED;
        case ButtonStyle::SECONDARY: return Theme::BG_DARK;
        case ButtonStyle::DANGER:    return Theme::DANGER_DARK;
        case ButtonStyle::SUCCESS:   return Theme::SUCCESS_DARK;
        case ButtonStyle::GHOST:     return Theme::BG_DARK;
    }
    return Theme::ACCENT_PRESSED;
}

void Button::draw(sf::RenderWindow& window) {
    // A keyboard-focused button reads as if the mouse were hovering it, so the
    // user can always see what Enter is about to press.
    float raw = std::max(m_hoverT, m_focused ? 1.f : 0.f);

    // Smoothstep: the highlight eases in and out instead of ramping linearly,
    // which is what makes the motion read as deliberate rather than twitchy.
    float lit = raw * raw * (3.f - 2.f * raw);

    sf::Color base = m_pressed
        ? pressedColor()
        : Theme::lerp(idleColor(), hoverColor(), lit);

    const bool outlined = (m_style == ButtonStyle::SECONDARY ||
                           m_style == ButtonStyle::GHOST);

    // The button lifts a couple of pixels on hover and settles back when
    // pressed, so the shadow below it has something to sell.
    float lift = m_pressed ? 0.f : 2.f * lit;
    sf::Vector2f pos{m_pos.x, m_pos.y - lift};

    // Coloured halo while focused; a growing drop shadow while hovered. Both
    // are skipped for the transparent GHOST style so it stays weightless.
    if (m_focused) {
        Theme::drawGlow(window, pos, m_size, m_radius,
                        Theme::lerp(hoverColor(), sf::Color::White, 0.35f),
                        9.f, 22, 5);
    } else if (!outlined && !m_pressed) {
        Theme::drawShadow(window, pos, m_size, m_radius,
                          9.f + 5.f * lit,
                          static_cast<std::uint8_t>(15 + 9 * lit),
                          3.f + 2.f * lit, 5);
    } else if (outlined && lit > 0.01f) {
        // Outlined styles get a faint tinted wash instead of a hard shadow.
        Theme::drawGlow(window, pos, m_size, m_radius, Theme::ACCENT,
                        7.f, static_cast<std::uint8_t>(14 * lit), 4);
    }

    float borderW = 0.f;
    sf::Color borderC = sf::Color::Transparent;
    if (outlined) {
        borderW = 1.5f;
        borderC = Theme::lerp(Theme::BORDER_IDLE, Theme::ACCENT, lit * 0.85f);
    } else if (m_focused) {
        borderW = 1.5f;
        borderC = Theme::lerp(base, sf::Color::White, 0.55f);
    } else if (lit > 0.01f) {
        // Rim brightens as it lights up -- catches the eye without a hard edge.
        borderW = 1.5f;
        borderC = Theme::withAlpha(Theme::lerp(base, sf::Color::White, 0.45f),
                                   static_cast<std::uint8_t>(150 * lit));
    }

    // Lit from above: brighter top edge easing into the base colour.
    sf::Color top = Theme::lerp(base, sf::Color::White, m_pressed ? 0.02f : 0.14f);
    sf::Color bot = Theme::lerp(base, sf::Color::Black, m_pressed ? 0.04f : 0.12f);
    Theme::drawRoundedRectV(window, pos, m_size, m_radius, top, bot, borderW, borderC);

    // Glass highlight across the upper half, strengthening on hover.
    if (!outlined && !m_pressed && m_size.y > 12.f) {
        Theme::fillRoundedRectV(window,
                                {pos.x + 1.f, pos.y + 1.f},
                                {m_size.x - 2.f, m_size.y * 0.45f},
                                m_radius,
                                sf::Color(255, 255, 255,
                                          static_cast<std::uint8_t>(26 + 18 * lit)),
                                sf::Color(255, 255, 255, 0));
    }

    // Nudge the label down a pixel while pressed so the button feels physical.
    sf::FloatRect labelBox{pos, m_size};
    if (m_pressed) labelBox.position.y += 1.f;
    Theme::drawCenteredText(window, m_font, m_label, Theme::Type::BODY,
                            Theme::TEXT_PRIMARY, labelBox, sf::Text::Bold);
}

void Button::update(sf::RenderWindow& window) {
    auto mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window));
    m_hovered = sf::FloatRect{m_pos, m_size}.contains(mouse);

    // Exponential approach rather than a fixed step: snappy on the way in
    // (~7 frames), a slower release on the way out, which is what makes a
    // hover feel responsive instead of mushy.
    float target = m_hovered ? 1.f : 0.f;
    float rate   = m_hovered ? 0.28f : 0.16f;
    m_hoverT += (target - m_hoverT) * rate;
    if (std::abs(target - m_hoverT) < 0.005f) m_hoverT = target;

    m_pressed = m_hovered && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
}

bool Button::isClicked(sf::RenderWindow& window) {
    auto mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window));
    return sf::FloatRect{m_pos, m_size}.contains(mouse);
}

void Button::setLabel(const std::string& l)  { m_label = l; }
void Button::setStyle(ButtonStyle s)          { m_style = s; }
void Button::setPosition(sf::Vector2f p)      { m_pos   = p; }
void Button::setFocused(bool f)               { m_focused = f; }
bool Button::isFocused() const                { return m_focused; }
void Button::setHoverT(float t)               { m_hoverT = std::clamp(t, 0.f, 1.f); }
float Button::hoverT() const                  { return m_hoverT; }
sf::FloatRect Button::getBounds() const       { return {m_pos, m_size}; }