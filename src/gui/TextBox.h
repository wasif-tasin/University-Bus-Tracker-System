#ifndef TEXTBOX_H
#define TEXTBOX_H

#include <SFML/Graphics.hpp>
#include <cstddef>
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
    bool         m_showText;       
    float        m_glowT;
    std::size_t  m_caret;        
    sf::Clock    m_cursorClock;

   
    std::string  displayString() const;             
    sf::Text     makeText(const std::string& s) const;
    float        innerWidth() const;                
    bool         fits(const std::string& candidate) const;
    float        caretOffset(const sf::Text& t, std::size_t i) const;
    std::size_t  caretIndexAt(float mouseX) const;  
    std::size_t  prevWord(std::size_t i) const;
    std::size_t  nextWord(std::size_t i) const;
    void         insertText(const std::string& s);

public:
    TextBox(sf::Font& font, sf::Vector2f size, sf::Vector2f position);


    void        draw(sf::RenderWindow& window);
    void        handleEvent(const sf::Event& event);
    void        setFocused(bool focused);
    std::string getText() const;


    void setPlaceholder(const std::string& ph);
    void setPasswordMode(bool pm);
    void clear();
    void setText(const std::string& text);
    void setPosition(sf::Vector2f pos);
    sf::FloatRect getBounds() const;
};

#endif
