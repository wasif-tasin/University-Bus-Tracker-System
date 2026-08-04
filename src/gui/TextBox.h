#ifndef TEXTBOX_H
#define TEXTBOX_H

#include <SFML/Graphics.hpp>
#include <string>

class TextBox
{
private:
    sf::Vector2f m_pos;
    sf::Vector2f m_size;
    sf::Font&    m_font;
    std::string  m_value;
    std::string  m_placeholder;
    bool         m_focused;
    bool         m_passwordMode;
    bool         m_showText;       // true = reveal password
    float        m_glowT;
    sf::Clock    m_cursorClock;

public:
    TextBox(sf::Font& font, sf::Vector2f size, sf::Vector2f position);

    // Original API (unchanged)
    void        draw(sf::RenderWindow& window);
    void        handleEvent(const sf::Event& event);
    void        setFocused(bool focused);
    std::string getText() const;

    // New helpers
    void setPlaceholder(const std::string& ph);
    void setPasswordMode(bool pm);
    void clear();
    void setText(const std::string& text);
    void setPosition(sf::Vector2f pos);
    sf::FloatRect getBounds() const;
};

#endif // TEXTBOX_H