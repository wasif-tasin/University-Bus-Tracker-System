#ifndef SCREENMANAGER_H
#define SCREENMANAGER_H

#include <SFML/Graphics.hpp>

#include <memory>
#include <vector>

class Screen;

enum class Transition
{
    None,
    Fade,
    SlideLeft,
    SlideRight
};

class ScreenManager
{
public:
    ScreenManager(sf::RenderWindow &window, sf::Font &font);

    void push(std::unique_ptr<Screen> screen, Transition t = Transition::SlideLeft);
    void pop(Transition t = Transition::SlideRight);
    void quit();

    void run(std::unique_ptr<Screen> initial);

    sf::Font &font() const { return m_font; }
    sf::Vector2f size() const;

    bool isTransitioning() const { return m_transT < 1.f; }

    sf::RenderTexture &scratch();

private:
    struct Pending
    {
        enum Kind
        {
            NONE,
            PUSH,
            POP,
            QUIT
        };

        Kind kind = NONE;
        std::unique_ptr<Screen> screen;
        Transition transition = Transition::None;
    };

    Screen *active() const;
    void applyPending();
    void activate(Screen &screen);
    bool snapshot(Screen &screen, sf::RenderTexture &target);
    bool resizeTarget(sf::RenderTexture &target);
    void render();
    void drawTransition();
    void finishTransition();

    sf::RenderWindow &m_window;
    sf::Font &m_font;

    std::vector<std::unique_ptr<Screen>> m_stack;
    Pending m_pending;

    sf::RenderTexture m_fromTex;
    sf::RenderTexture m_toTex;
    sf::RenderTexture m_scratch;

    Transition m_transKind = Transition::None;
    float m_transT = 1.f;
    float m_transDur = 0.f;
};

#endif
