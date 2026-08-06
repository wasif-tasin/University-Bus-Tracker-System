#ifndef SCREEN_H
#define SCREEN_H

#include <SFML/Graphics.hpp>
#include <string>

class ScreenManager;

class Screen
{
public:
    virtual ~Screen() = default;

    void attach(ScreenManager *app) { m_app = app; }

    virtual std::string title() const { return "University Bus Tracker"; }

    virtual void onEnter() {}

    virtual void prepare(sf::Vector2f size, sf::Vector2f mouse)
    {
        m_size = size;
        m_mouse = mouse;
    }

    virtual void handleEvent(const sf::Event &event) { (void)event; }
    virtual void update(float dt) { (void)dt; }
    virtual void draw(sf::RenderTarget &target) = 0;

    virtual void skipAnimations() {}

protected:
    ScreenManager *m_app = nullptr;

    sf::Vector2f m_size{};
    sf::Vector2f m_mouse{};
};

#endif
