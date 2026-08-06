#ifndef USERREGISTRATIONGUI_H
#define USERREGISTRATIONGUI_H

#include "Screen.h"
#include "TextBox.h"
#include "User.h"
#include <SFML/Graphics.hpp>
#include <string>

class UserRegistrationScreen : public Screen
{
public:
    UserRegistrationScreen(sf::Font &font);

    std::string title() const override { return "Register"; }
    void onEnter() override;
    void prepare(sf::Vector2f size, sf::Vector2f mouse) override;
    void handleEvent(const sf::Event &event) override;
    void update(float dt) override;
    void draw(sf::RenderTarget &target) override;
    void skipAnimations() override;

private:
    enum Focus
    {
        F_NONE = -1,
        F_EMAIL = 0,
        F_PASS,
        F_REGISTER,
        F_BACK,
        F_COUNT
    };

    void submit();
    void step(int delta);

    sf::Font &m_font;
    User m_user;

    TextBox m_emailBox;
    TextBox m_passBox;

    std::string m_infoText;
    bool m_infoIsError = false;
    int m_focus = F_NONE;
    float m_regHoverT = 0.f;
    float m_backHoverT = 0.f;
};

#endif
