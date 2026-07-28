#include <SFML/Graphics.hpp>
#include <iostream>

int main()
{
    sf::Font font;

    if (!font.openFromFile("assets/Roboto-Regular.ttf"))
    {
        std::cout << "Failed to load font!\n";
        return 1;
    }

    sf::Text title(font);
    title.setString("UNIVERSITY BUS TRACKER");
    title.setCharacterSize(40);

    sf::FloatRect bounds = title.getLocalBounds();
    std::cout << "position.x: " << bounds.position.x << "\n";
    std::cout << "position.y: " << bounds.position.y << "\n";
    std::cout << "size.x: " << bounds.size.x << "\n";
    std::cout << "size.y: " << bounds.size.y << "\n";

    return 0;
}