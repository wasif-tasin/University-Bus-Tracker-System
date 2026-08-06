#ifndef USERLOGINGUI_H
#define USERLOGINGUI_H

#include "Screen.h"
#include "TextBox.h"
#include "User.h"
#include <SFML/Graphics.hpp>
#include <string>

class UserLoginScreen : public Screen
{
public:
    UserLoginScreen(sf::Font& font);

    std::string title() const override { return "University Bus Tracker - User Login"; }
    void onEnter() override;
    void prepare(sf::Vector2f size, sf::Vector2f mouse) override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void draw(sf::RenderTarget& target) override;
    void skipAnimations() override;

private:
    enum Focus { F_EMAIL = 0, F_PASS, F_LOGIN, F_BACK, F_COUNT };

    void submit();

    sf::Font& m_font;
    User m_user;

    TextBox m_emailBox;
    TextBox m_passBox;

    std::string m_errorText;
    int m_focus = F_EMAIL;
    float m_loginHoverT = 0.f;
    float m_backHoverT = 0.f;
};

#endif
