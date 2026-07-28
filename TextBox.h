#ifndef TEXTBOX_H
#define TEXTBOX_H

#include <SFML/Graphics.hpp>
#include <string>

class TextBox
{
private:
    sf::RectangleShape box;
    sf::Font& font;
    sf::Text text;
    std::string value;
    bool isFocused{false};
    sf::Clock cursorClock;

public:
    TextBox(sf::Font& font,
            sf::Vector2f size,
            sf::Vector2f position);

    void draw(sf::RenderWindow& window);
    void handleEvent(const sf::Event& event);
    void setFocused(bool focused);
    std::string getText() const;
};

#endif