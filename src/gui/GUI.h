#ifndef GUI_H
#define GUI_H

#include "Button.h"
#include "Screen.h"

#include <string>

class HomeScreen : public Screen
{
public:
    explicit HomeScreen(sf::Font &font);

    std::string title() const override { return "University Bus Tracker"; }

    void prepare(sf::Vector2f size, sf::Vector2f mouse) override;
    void handleEvent(const sf::Event &event) override;
    void update(float dt) override;
    void draw(sf::RenderTarget &target) override;
    void skipAnimations() override;

private:
    void openUserPanel();
    void openAdminPanel();
    void step(int delta);

    void drawInfoPanel(sf::RenderTarget &target);

    float paintInfo(sf::RenderTarget &target, float x, float w, float y0,
                    float k, bool measure);

    sf::Font &m_font;
    Button    m_exitBtn;

    int   m_focus;
    float m_card1T;
    float m_card2T;
    float m_introT;

    float m_leftW  = 0.f, m_rightX = 0.f, m_rightW = 0.f;
    float m_cardX  = 0.f, m_cardW  = 0.f, m_cardH  = 0.f;
    float m_card1Y = 0.f, m_card2Y = 0.f;
    bool  m_card1Hover = false, m_card2Hover = false;
};

class GUI
{
public:
    void run();
};

#endif
