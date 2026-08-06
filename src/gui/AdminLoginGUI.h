#ifndef ADMINLOGINGUI_H
#define ADMINLOGINGUI_H

#include "Screen.h"
#include "TextBox.h"
#include "Admin.h"
#include <SFML/Graphics.hpp>
#include <string>

class AdminLoginScreen : public Screen
{
public:
    AdminLoginScreen(sf::Font &font);

    std::string title() const override { return "University Bus Tracker - Admin Login"; }
    void onEnter() override;
    void prepare(sf::Vector2f size, sf::Vector2f mouse) override;
    void handleEvent(const sf::Event &event) override;
    void update(float dt) override;
    void draw(sf::RenderTarget &target) override;
    void skipAnimations() override;

private:
    // F_NONE = nothing focused yet, so the screen opens with no highlight.
    // The first Tab/arrow lands on the first control.
    enum Focus
    {
        F_NONE = -1,
        F_USER = 0,
        F_PASS,
        F_LOGIN,
        F_BACK,
        F_COUNT
    };

    void submit();
    void step(int delta);

    sf::Font &m_font;
    Admin m_admin;

    TextBox m_userBox;
    TextBox m_passBox;

    std::string m_errorText;
    int m_focus = F_NONE;
    float m_loginHoverT = 0.f;
    float m_backHoverT = 0.f;
};

#endif
