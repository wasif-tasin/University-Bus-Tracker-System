#ifndef USERPANELGUI_H
#define USERPANELGUI_H

#include "Screen.h"
#include <SFML/Graphics.hpp>
#include <string>

class UserPanelScreen : public Screen
{
public:
    UserPanelScreen(sf::Font& font);

    std::string title() const override { return "User Panel"; }
    void prepare(sf::Vector2f size, sf::Vector2f mouse) override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void draw(sf::RenderTarget& target) override;
    void skipAnimations() override;

private:
    void step(int delta);
    void activate();

    sf::Font& m_font;
    int m_focus = -1;
    float m_loginHoverT = 0.f;
    float m_regHoverT = 0.f;
    float m_backHoverT = 0.f;
};

#endif
