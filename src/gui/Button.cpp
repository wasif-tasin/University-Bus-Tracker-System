#include "Button.h"
#include "Theme.h"

#include <algorithm>

Button::Button(sf::Font& f, const std::string& label, sf::Vector2f size,
               sf::Vector2f pos, ButtonStyle style, float radius)
    : m_pos(pos), m_size(size), m_font(f), m_label(label),
      m_style(style), m_radius(radius),
      m_hoverT(0.f), m_hovered(false), m_pressed(false)
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
    sf::Color fill = m_pressed
        ? pressedColor()
        : Theme::lerp(idleColor(), hoverColor(), m_hoverT);

    float borderW = 0.f;
    sf::Color borderC = sf::Color::Transparent;
    if (m_style == ButtonStyle::SECONDARY || m_style == ButtonStyle::GHOST) {
        borderW = 1.5f;
        borderC = Theme::lerp(Theme::BORDER_IDLE, Theme::ACCENT, m_hoverT * 0.6f);
    }

    Theme::drawRoundedRect(window, m_pos, m_size, m_radius, fill, borderW, borderC);

    // Bold label, pure white, centred on whole pixels — button text is a
    // primary action and was the most obviously washed-out text in the old UI.
    Theme::drawCenteredText(window, m_font, m_label, Theme::Type::BODY,
                            Theme::TEXT_PRIMARY, {m_pos, m_size}, sf::Text::Bold);
}

void Button::update(sf::RenderWindow& window) {
    auto mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window));
    m_hovered = sf::FloatRect{m_pos, m_size}.contains(mouse);
    m_hoverT  = std::clamp(m_hoverT + (m_hovered ? 0.12f : -0.12f), 0.f, 1.f);
    m_pressed = m_hovered && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
}

bool Button::isClicked(sf::RenderWindow& window) {
    auto mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window));
    return sf::FloatRect{m_pos, m_size}.contains(mouse);
}

void Button::setLabel(const std::string& l)  { m_label = l; }
void Button::setStyle(ButtonStyle s)          { m_style = s; }
void Button::setPosition(sf::Vector2f p)      { m_pos   = p; }
sf::FloatRect Button::getBounds() const       { return {m_pos, m_size}; }