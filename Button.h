#ifndef BUTTON_H
#define BUTTON_H

#include <SFML/Graphics.hpp>
#include <string>

class Button
{
private:
    sf::RectangleShape box;
    sf::Font& font;
    std::string label;

public:
    Button(sf::Font& font,
           const std::string& text,
           sf::Vector2f size,
           sf::Vector2f position);

    void draw(sf::RenderWindow& window);
    void update(sf::RenderWindow& window);
    bool isClicked(sf::RenderWindow& window);
};

#endif