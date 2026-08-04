#ifndef BUTTON_H
#define BUTTON_H

#include <SFML/Graphics.hpp>
#include <string>

// Visual style variants for Button
enum class ButtonStyle {
    PRIMARY,    // Blue accent fill
    SECONDARY,  // Dark with border
    DANGER,     // Red fill
    SUCCESS,    // Green fill
    GHOST       // Transparent with border
};

class Button
{
private:
    sf::Vector2f m_pos;
    sf::Vector2f m_size;
    sf::Font&    m_font;
    std::string  m_label;
    ButtonStyle  m_style;
    float        m_radius;
    float        m_hoverT;
    bool         m_hovered;
    bool         m_pressed;

    sf::Color idleColor()    const;
    sf::Color hoverColor()   const;
    sf::Color pressedColor() const;

public:
    // Same primary constructor signature — style and radius are optional extras
    Button(sf::Font&            font,
           const std::string&   label,
           sf::Vector2f         size,
           sf::Vector2f         position,
           ButtonStyle          style  = ButtonStyle::PRIMARY,
           float                radius = 8.f);

    void draw(sf::RenderWindow& window);
    void update(sf::RenderWindow& window);
    bool isClicked(sf::RenderWindow& window);

    // Helpers
    void setLabel(const std::string& label);
    void setStyle(ButtonStyle style);
    void setPosition(sf::Vector2f pos);
    sf::FloatRect getBounds() const;
};

#endif // BUTTON_H