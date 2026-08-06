#include "ScreenManager.h"
#include "Screen.h"
#include "Theme.h"

#include <algorithm>
#include <cmath>

namespace
{

    constexpr float SLIDE_SECONDS = 0.34f;
    constexpr float FADE_SECONDS = 0.24f;

    constexpr float PARALLAX = 0.24f;

    float easeOutCubic(float t)
    {
        float u = 1.f - std::clamp(t, 0.f, 1.f);
        return 1.f - u * u * u;
    }

    float easeInOutQuad(float t)
    {
        t = std::clamp(t, 0.f, 1.f);
        return t < 0.5f ? 2.f * t * t : 1.f - 2.f * (1.f - t) * (1.f - t);
    }

    sf::Color fade(float alpha)
    {
        return sf::Color(255, 255, 255,
                         static_cast<std::uint8_t>(std::clamp(alpha, 0.f, 255.f)));
    }

}

ScreenManager::ScreenManager(sf::RenderWindow &window, sf::Font &font)
    : m_window(window), m_font(font)
{

    Theme::loadBackgroundImage();
}

sf::Vector2f ScreenManager::size() const
{
    return m_window.getView().getSize();
}

Screen *ScreenManager::active() const
{
    return m_stack.empty() ? nullptr : m_stack.back().get();
}

void ScreenManager::push(std::unique_ptr<Screen> screen, Transition t)
{
    if (!screen)
        return;
    m_pending.kind = Pending::PUSH;
    m_pending.screen = std::move(screen);
    m_pending.transition = t;
}

void ScreenManager::pop(Transition t)
{
    m_pending.kind = Pending::POP;
    m_pending.transition = t;
}

void ScreenManager::quit()
{
    m_pending.kind = Pending::QUIT;
}

bool ScreenManager::resizeTarget(sf::RenderTexture &target)
{
    const sf::Vector2u want(std::max(1u, static_cast<unsigned>(std::lround(size().x))),
                            std::max(1u, static_cast<unsigned>(std::lround(size().y))));

    if (target.getSize() == want)
        return true;

    if (target.resize(want, Theme::uiContext()) || target.resize(want))
    {
        target.setSmooth(true);
        return true;
    }
    return false;
}

sf::RenderTexture &ScreenManager::scratch()
{
    resizeTarget(m_scratch);
    return m_scratch;
}

bool ScreenManager::snapshot(Screen &screen, sf::RenderTexture &target)
{
    screen.skipAnimations();
    if (!resizeTarget(target))
        return false;

    target.clear(Theme::BG_DEEP);
    screen.draw(target);
    target.display();
    return true;
}

void ScreenManager::activate(Screen &screen)
{
    screen.attach(this);
    screen.onEnter();
    screen.prepare(size(), m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window)));
    m_window.setTitle(screen.title());
}

void ScreenManager::finishTransition()
{
    m_transT = 1.f;
}

void ScreenManager::applyPending()
{
    const Pending::Kind kind = m_pending.kind;
    if (kind == Pending::NONE)
        return;

    m_pending.kind = Pending::NONE;

    if (kind == Pending::QUIT)
    {
        m_pending.screen.reset();
        m_window.close();
        return;
    }

    if (kind == Pending::POP && m_stack.size() <= 1)
    {
        m_window.close();
        return;
    }

    Transition t = m_pending.transition;

    if (Screen *current = active())
    {
        if (!snapshot(*current, m_fromTex))
            t = Transition::None;
    }
    else
    {
        t = Transition::None;
    }

    if (kind == Pending::PUSH)
    {
        m_stack.push_back(std::move(m_pending.screen));
    }
    else
    {
        m_stack.pop_back();
    }

    Screen *next = active();
    if (!next)
    {
        m_window.close();
        return;
    }

    activate(*next);

    if (t == Transition::None || !snapshot(*next, m_toTex))
    {
        finishTransition();
        return;
    }

    m_transKind = t;
    m_transDur = (t == Transition::Fade) ? FADE_SECONDS : SLIDE_SECONDS;
    m_transT = 0.f;
}

void ScreenManager::drawTransition()
{
    const float w = size().x;
    const float h = size().y;

    const float e = (m_transKind == Transition::Fade) ? easeInOutQuad(m_transT)
                                                      : easeOutCubic(m_transT);

    sf::Sprite fromSpr(m_fromTex.getTexture());
    sf::Sprite toSpr(m_toTex.getTexture());

    auto edgeShadow = [&](float x, float strength)
    {
        const float sw = 30.f;
        Theme::drawGradientRectH(
            m_window, {x - sw, 0.f}, {sw, h},
            sf::Color(0, 0, 0, 0),
            sf::Color(0, 0, 0, static_cast<std::uint8_t>(110 * std::clamp(strength, 0.f, 1.f))));
    };

    switch (m_transKind)
    {
    case Transition::Fade:
        m_window.draw(toSpr);
        fromSpr.setColor(fade(255.f * (1.f - e)));
        m_window.draw(fromSpr);
        break;

    case Transition::SlideLeft:

        fromSpr.setPosition({-PARALLAX * w * e, 0.f});
        fromSpr.setColor(fade(255.f - 80.f * e));
        m_window.draw(fromSpr);

        edgeShadow(w * (1.f - e), e);
        toSpr.setPosition({w * (1.f - e), 0.f});
        m_window.draw(toSpr);
        break;

    case Transition::SlideRight:

        toSpr.setPosition({-PARALLAX * w * (1.f - e), 0.f});
        toSpr.setColor(fade(255.f - 80.f * (1.f - e)));
        m_window.draw(toSpr);

        edgeShadow(w * e, 1.f - e);
        fromSpr.setPosition({w * e, 0.f});
        m_window.draw(fromSpr);
        break;

    case Transition::None:
        m_window.draw(toSpr);
        break;
    }
}

void ScreenManager::render()
{
    m_window.clear(Theme::BG_DEEP);

    if (isTransitioning())
        drawTransition();
    else if (Screen *current = active())
        current->draw(m_window);

    m_window.display();
}

void ScreenManager::run(std::unique_ptr<Screen> initial)
{
    push(std::move(initial), Transition::None);
    applyPending();

    sf::Clock clock;

    while (m_window.isOpen())
    {

        const float dt = std::min(clock.restart().asSeconds(), 0.05f);

        const sf::Vector2f mouse =
            m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window));

        if (!isTransitioning())
            if (Screen *current = active())
                current->prepare(size(), mouse);

        while (const std::optional event = m_window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                m_window.close();
                break;
            }

            if (event->is<sf::Event::Resized>())
            {
                Theme::syncViewToWindow(m_window, *event);

                finishTransition();
                if (Screen *current = active())
                    current->prepare(size(), mouse);
                continue;
            }

            if (isTransitioning())
                continue;

            if (Screen *current = active())
                current->handleEvent(*event);
        }

        if (!m_window.isOpen())
            break;

        if (isTransitioning())
        {
            m_transT = std::min(1.f, m_transT + dt / m_transDur);
        }
        else if (Screen *current = active())
        {
            current->update(dt);
        }

        applyPending();
        if (!m_window.isOpen())
            break;

        render();
    }
}
