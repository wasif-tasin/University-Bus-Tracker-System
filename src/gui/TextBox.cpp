#include "TextBox.h"

TextBox::TextBox(sf::Font &f,
                 sf::Vector2f size,
                 sf::Vector2f position)
    : font(f), text(font)
{
    box.setSize(size);
    box.setPosition(position);
    box.setFillColor(sf::Color::White);
    box.setOutlineThickness(2);
    box.setOutlineColor(sf::Color::Black);

    text.setCharacterSize(22);
    text.setFillColor(sf::Color::Black);
    text.setPosition({position.x + 10.f,
                      position.y + 8.f});

    value = "";
}

void TextBox::draw(sf::RenderWindow &window)
{
    std::string displayText = value;
    if (isFocused)
    {
        if ((cursorClock.getElapsedTime().asMilliseconds() / 500) % 2 == 0)
        {
            displayText += "|";
        }
    }
    text.setString(displayText);

    window.draw(box);
    window.draw(text);
}

void TextBox::handleEvent(const sf::Event &event)
{
    if (!isFocused) return;

    if (const auto *txt = event.getIf<sf::Event::TextEntered>())
    {
        if (txt->unicode == 8) 
        {
            if (!value.empty())
                value.pop_back();
        }
        else if (txt->unicode >= 32 && txt->unicode <= 126)
        {
            std::string temp = value + static_cast<char>(txt->unicode);
            sf::Text tempText(font);
            tempText.setCharacterSize(text.getCharacterSize());
            tempText.setString(temp);
            if (tempText.getLocalBounds().size.x < box.getSize().x - 20.f)
            {
                value = temp;
            }
        }
    }
}

void TextBox::setFocused(bool focused)
{
    isFocused = focused;
    if (focused)
    {
        box.setOutlineColor(sf::Color(40, 120, 220)); 
        box.setOutlineThickness(3);
        cursorClock.restart();
    }
    else
    {
        box.setOutlineColor(sf::Color::Black); 
        box.setOutlineThickness(2);
    }
}

std::string TextBox::getText() const
{
    return value;
}