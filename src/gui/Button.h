#ifndef BUTTON_H
#define BUTTON_H

#include <SFML/Graphics.hpp>
#include <string>

enum class ButtonStyle
{
    PRIMARY,
    SECONDARY,
    DANGER,
    SUCCESS,
    GHOST
};

class Button
{
private:
    sf::Vector2f m_pos;
    sf::Vector2f m_size;
    sf::Font &m_font;
    std::string m_label;
    ButtonStyle m_style;
    float m_radius;
    float m_hoverT;
    bool m_hovered;
    bool m_pressed;
    bool m_focused;

    sf::Color idleColor() const;
    sf::Color hoverColor() const;
    sf::Color pressedColor() const;

public:
    Button(sf::Font &font,
           const std::string &label,
           sf::Vector2f size,
           sf::Vector2f position,
           ButtonStyle style = ButtonStyle::PRIMARY,
           float radius = 8.f);

    void draw(sf::RenderWindow &window);
    void update(sf::RenderWindow &window);
    bool isClicked(sf::RenderWindow &window);

    void setLabel(const std::string &label);
    void setStyle(ButtonStyle style);
    void setPosition(sf::Vector2f pos);
    void setFocused(bool focused);
    bool isFocused() const;

    // Screens that rebuild their buttons every frame must carry the hover
    // animation across frames themselves: seed it before update(), read it
    // back after. Without this the easing restarts from 0 every frame and the
    // hover never visibly ramps up.
    void setHoverT(float t);
    float hoverT() const;

    sf::FloatRect getBounds() const;
};

#endif