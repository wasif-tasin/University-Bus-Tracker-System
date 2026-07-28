#include "Button.h"

Button::Button(sf::Font& f,
               const std::string& text,
               sf::Vector2f size,
               sf::Vector2f position)
    : font(f), label(text)
{
    box.setSize(size);
    box.setPosition(position);
    box.setFillColor(sf::Color(40,120,220));
}

void Button::draw(sf::RenderWindow& window)
{
    window.draw(box);

    sf::Text txt(font);

    txt.setString(label);
    txt.setCharacterSize(24);
    txt.setFillColor(sf::Color::White);

    sf::FloatRect bounds = txt.getLocalBounds();

    txt.setOrigin({bounds.position.x + bounds.size.x / 2.f, bounds.position.y + bounds.size.y / 2.f});
    txt.setPosition({
        box.getPosition().x + box.getSize().x / 2.f,
        box.getPosition().y + box.getSize().y / 2.f
    });

    window.draw(txt);
}

void Button::update(sf::RenderWindow& window)
{
    auto mouse = window.mapPixelToCoords(
        sf::Mouse::getPosition(window));

    if (box.getGlobalBounds().contains(mouse))
        box.setFillColor(sf::Color(70,150,255));
    else
        box.setFillColor(sf::Color(40,120,220));
}

bool Button::isClicked(sf::RenderWindow& window)
{
    auto mouse = window.mapPixelToCoords(
        sf::Mouse::getPosition(window));

    return box.getGlobalBounds().contains(mouse);
}