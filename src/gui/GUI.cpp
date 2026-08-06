#include "GUI.h"
#include "AdminLoginGUI.h"
#include "ScreenManager.h"
#include "Theme.h"
#include "UserLoginGUI.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>

namespace
{

    constexpr float INTRO_SECONDS = 0.9f;

    struct Logo
    {
        sf::Texture tex;
        bool ready = false;
    };

    Logo &logo()
    {
        static Logo l;
        static bool tried = false;
        if (!tried)
        {
            tried = true;
            if (l.tex.loadFromFile("assets/logo.png"))
            {
                l.tex.setSmooth(true);
                l.ready = true;
            }
        }
        return l;
    }

    sf::Color fadeC(sf::Color c, float a)
    {
        return Theme::withAlpha(
            c, static_cast<std::uint8_t>(std::lround(c.a * std::clamp(a, 0.f, 1.f))));
    }

    sf::Text fitText(const sf::Font &font, const std::string &str, unsigned cs,
                     unsigned minCs, float maxW, sf::Color col,
                     std::uint32_t style = sf::Text::Regular, float spacing = 1.f)
    {
        sf::Text t(font);
        t.setString(str);
        t.setCharacterSize(std::max(cs, minCs));
        t.setFillColor(col);
        t.setStyle(style);
        t.setLetterSpacing(spacing);

        while (t.getCharacterSize() > minCs && t.getLocalBounds().size.x > maxW)
            t.setCharacterSize(t.getCharacterSize() - 1);
        return t;
    }

    void drawCrownIcon(sf::RenderTarget &target, sf::Vector2f pos, float size,
                       sf::Color color)
    {
        const float w = size, h = size * 0.86f;
        auto P = [&](float u, float v)
        {
            return sf::Vector2f{pos.x + u * w, pos.y + v * h};
        };

        const float baseTop = 0.62f;
        const float tri[3][3][2] = {
            {{0.00f, baseTop}, {0.34f, baseTop}, {0.10f, 0.06f}},
            {{0.22f, baseTop}, {0.78f, baseTop}, {0.50f, 0.00f}},
            {{0.66f, baseTop}, {1.00f, baseTop}, {0.90f, 0.06f}},
        };

        sf::VertexArray peaks(sf::PrimitiveType::Triangles, 9);
        for (int t = 0; t < 3; ++t)
            for (int v = 0; v < 3; ++v)
            {
                peaks[t * 3 + v].position = P(tri[t][v][0], tri[t][v][1]);
                peaks[t * 3 + v].color = color;
            }
        target.draw(peaks);

        Theme::fillRoundedRect(target, P(0.f, baseTop - 0.04f),
                               {w, h * 0.42f}, h * 0.11f,
                               Theme::lerp(color, sf::Color::White, 0.22f));

        const float gemR = std::max(1.2f, size * 0.075f);
        for (float u : {0.10f, 0.50f, 0.90f})
        {
            sf::CircleShape gem(gemR, 12);
            gem.setOrigin({gemR, gemR});
            gem.setFillColor(Theme::lerp(color, sf::Color::White, 0.45f));
            gem.setPosition(Theme::px(P(u, u == 0.50f ? 0.00f : 0.06f)));
            target.draw(gem);
        }
    }

    void drawTeamIcon(sf::RenderTarget &target, sf::Vector2f pos, float size,
                      sf::Color color)
    {
        auto person = [&](float cx, float top, float s, sf::Color c)
        {
            const float headR = s * 0.20f;
            sf::CircleShape head(headR, 20);
            head.setOrigin({headR, headR});
            head.setFillColor(c);
            head.setPosition(Theme::px(pos.x + cx, pos.y + top + headR));
            target.draw(head);

            const float bw = s * 0.56f, bh = s * 0.34f;
            Theme::fillRoundedRect(target,
                                   {pos.x + cx - bw * 0.5f, pos.y + top + headR * 2.f + s * 0.06f},
                                   {bw, bh}, bw * 0.42f, c);
        };

        person(size * 0.66f, size * 0.10f, size * 0.82f, Theme::withAlpha(color, color.a / 2));
        person(size * 0.36f, size * 0.16f, size, color);
    }

}

HomeScreen::HomeScreen(sf::Font &font)
    : m_font(font),
      m_exitBtn(font, "Exit", {150.f, 46.f}, {0.f, 0.f}, ButtonStyle::GHOST, 10.f),
      m_focus(-1), m_card1T(0.f), m_card2T(0.f), m_introT(0.f)
{
}

void HomeScreen::prepare(sf::Vector2f size, sf::Vector2f mouse)
{
    Screen::prepare(size, mouse);

    const bool wide = size.x >= 900.f;

    m_leftW = wide ? size.x * 0.46f : 0.f;
    m_rightX = m_leftW;
    m_rightW = size.x - m_leftW;

    m_cardW = std::min(m_rightW - 88.f, 420.f);
    m_cardH = std::clamp(size.y * 0.19f, 96.f, 132.f);
    m_cardX = m_rightX + (m_rightW - m_cardW) * 0.5f;

    const float gap = 22.f;
    const float block = m_cardH * 2.f + gap;
    const float top = std::max(size.y * 0.30f, (size.y - block) * 0.5f);

    m_card1Y = top;
    m_card2Y = top + m_cardH + gap;

    m_card1Hover = sf::FloatRect{{m_cardX, m_card1Y}, {m_cardW, m_cardH}}.contains(mouse);
    m_card2Hover = sf::FloatRect{{m_cardX, m_card2Y}, {m_cardW, m_cardH}}.contains(mouse);

    m_exitBtn.setPosition({m_cardX + (m_cardW - 150.f) * 0.5f, m_card2Y + m_cardH + 34.f});
}

void HomeScreen::step(int delta)
{
    if (m_focus < 0)
        m_focus = (delta > 0) ? 0 : 2;
    else
        m_focus = (m_focus + delta + 3) % 3;

    m_exitBtn.setFocused(m_focus == 2);
}

void HomeScreen::openUserPanel()
{
    m_app->push(std::make_unique<UserLoginScreen>(m_font));
}

void HomeScreen::openAdminPanel()
{
    m_app->push(std::make_unique<AdminLoginScreen>(m_font));
}

void HomeScreen::handleEvent(const sf::Event &event)
{
    if (event.is<sf::Event::MouseMoved>())
    {
        m_focus = -1;
        m_exitBtn.setFocused(false);
    }

    if (const auto *key = event.getIf<sf::Event::KeyPressed>())
    {
        switch (key->code)
        {
        case sf::Keyboard::Key::Tab:
        case sf::Keyboard::Key::Down:
        case sf::Keyboard::Key::Right:
            step(+1);
            return;

        case sf::Keyboard::Key::Up:
        case sf::Keyboard::Key::Left:
            step(-1);
            return;

        case sf::Keyboard::Key::Enter:
            if (m_focus < 0)
                step(+1);
            else if (m_focus == 0)
                openUserPanel();
            else if (m_focus == 1)
                openAdminPanel();
            else
                m_app->quit();
            return;

        case sf::Keyboard::Key::Escape:
            m_app->quit();
            return;
        default:
            return;
        }
    }

    if (const auto *mb = event.getIf<sf::Event::MouseButtonPressed>())
    {
        if (mb->button != sf::Mouse::Button::Left)
            return;

        if (m_card1Hover)
            openUserPanel();
        else if (m_card2Hover)
            openAdminPanel();
        else if (m_exitBtn.isClicked(m_mouse))
            m_app->quit();
    }
}

void HomeScreen::update(float dt)
{

    m_card1T = Theme::approachHover(m_card1T, m_card1Hover || m_focus == 0, dt);
    m_card2T = Theme::approachHover(m_card2T, m_card2Hover || m_focus == 1, dt);

    if (m_introT < 1.f)
        m_introT = std::min(1.f, m_introT + dt / INTRO_SECONDS);

    m_exitBtn.update(m_mouse, dt);
}

void HomeScreen::skipAnimations()
{
    m_card1T = (m_card1Hover || m_focus == 0) ? 1.f : 0.f;
    m_card2T = (m_card2Hover || m_focus == 1) ? 1.f : 0.f;
    m_introT = 1.f;
    m_exitBtn.settle();
}

void HomeScreen::draw(sf::RenderTarget &target)
{
    Theme::drawAppBackground(target, m_size);

    const bool wide = m_leftW > 0.f;

    if (wide)
    {
        drawInfoPanel(target);
    }
    else
    {

        Theme::drawCenteredText(target, m_font, "University Bus Tracker",
                                Theme::Type::H1, Theme::TEXT_PRIMARY,
                                {{0.f, m_card1Y - 96.f}, {m_size.x, 44.f}},
                                sf::Text::Bold);
        Theme::drawCenteredText(target, m_font, "Choose how you want to sign in",
                                Theme::Type::BODY, Theme::TEXT_MUTED,
                                {{0.f, m_card1Y - 50.f}, {m_size.x, 24.f}});
    }

    auto card = [&](float y, float lit, const std::string &heading,
                    const std::string &detail, sf::Color accent)
    {
        const float e = Theme::smoothstep01(lit);
        const float lift = 3.f * e;

        sf::Vector2f pos{m_cardX, y - lift};
        sf::Vector2f dim{m_cardW, m_cardH};

        Theme::drawShadow(target, pos, dim, 14.f,
                          10.f + 8.f * e,
                          static_cast<std::uint8_t>(20 + 16 * e),
                          4.f + 3.f * e, 5);

        Theme::drawRoundedRectV(target, pos, dim, 14.f,
                                Theme::lerp(Theme::BG_CARD, Theme::ITEM_HOVER, e),
                                Theme::lerp(Theme::BG_CARD_DARK, Theme::BG_CARD, e),
                                1.5f,
                                Theme::lerp(Theme::BORDER_IDLE, accent, 0.85f * e));

        Theme::fillRoundedRect(target, {pos.x, pos.y + m_cardH * (0.5f - 0.2f - 0.15f * e)},
                               {3.f, m_cardH * (0.4f + 0.3f * e)}, 1.5f,
                               Theme::withAlpha(accent,
                                                static_cast<std::uint8_t>(120 + 135 * e)));

        const float padX = 26.f;
        Theme::drawText(target, m_font, heading, Theme::Type::TITLE,
                        Theme::TEXT_PRIMARY,
                        {pos.x + padX, pos.y + m_cardH * 0.5f - 26.f}, sf::Text::Bold);
        Theme::drawText(target, m_font, detail, Theme::Type::META,
                        Theme::TEXT_SECONDARY,
                        {pos.x + padX, pos.y + m_cardH * 0.5f + 6.f});

        Theme::drawChevron(target, {pos.x + m_cardW - padX - 8.f + 6.f * e, pos.y + m_cardH * 0.5f},
                           9.f, Theme::withAlpha(Theme::TEXT_MUTED, static_cast<std::uint8_t>(120 + 135 * e)));
    };

    card(m_card1Y, m_card1T, "User Panel",
         "Login or register to browse bus routes", Theme::ACCENT);
    card(m_card2Y, m_card2T, "Admin Panel",
         "Manage universities, buses and routes", Theme::PURPLE);

    m_exitBtn.draw(target);
}

float HomeScreen::paintInfo(sf::RenderTarget &target, float x, float w,
                            float y0, float k, bool measure)
{
    const float cx = x + w * 0.5f;
    float y = y0;
    int blk = 0;

    auto CS = [&](float v)
    {
        return static_cast<unsigned>(std::max(9.f, std::round(v * k)));
    };
    auto SP = [&](float v)
    { return std::round(v * k); };

    auto stage = [&](int i)
    {
        return Theme::smoothstep01(
            std::clamp((m_introT - 0.06f * i) / 0.42f, 0.f, 1.f));
    };

    auto driftY = [&](float a)
    { return (1.f - a) * SP(12.f); };
    auto centered = [&](const std::string &s, unsigned cs, unsigned minCs,
                        sf::Color col, std::uint32_t style, float spacing,
                        float advance, int b)
    {
        if (!measure)
        {
            const float a = stage(b);
            sf::Text t = fitText(m_font, s, cs, minCs, w, fadeC(col, a), style, spacing);
            const sf::FloatRect bb = t.getLocalBounds();
            t.setPosition(Theme::px(cx - (bb.position.x + bb.size.x * 0.5f),
                                    y + driftY(a) - bb.position.y));
            target.draw(t);
        }
        y += advance;
    };

    auto leftLine = [&](const std::string &s, float lx, float lw, unsigned cs,
                        unsigned minCs, sf::Color col, std::uint32_t style,
                        float spacing, float advance, int b)
    {
        if (!measure)
        {
            const float a = stage(b);
            sf::Text t = fitText(m_font, s, cs, minCs, lw, fadeC(col, a), style, spacing);
            const sf::FloatRect bb = t.getLocalBounds();
            t.setPosition(Theme::px(lx - bb.position.x,
                                    y + driftY(a) - bb.position.y));
            target.draw(t);
        }
        y += advance;
    };

    {

        const float ls = std::clamp(w * 0.27f, 88.f, 106.f) * k;
        if (!measure)
        {
            const float a = stage(blk);
            const float cy = y + ls * 0.5f + driftY(a);

            Theme::drawRadialGlow(target, {cx, cy}, ls * 0.95f, Theme::ACCENT,
                                  static_cast<std::uint8_t>(48 * a));

            Logo &lg = logo();
            if (lg.ready)
            {
                const sf::Vector2u ts = lg.tex.getSize();
                const float s = ls / std::max(1.f, static_cast<float>(std::max(ts.x, ts.y)));
                sf::Sprite spr(lg.tex);
                spr.setScale({s, s});
                spr.setColor(sf::Color(255, 255, 255,
                                       static_cast<std::uint8_t>(255 * a)));
                spr.setPosition(Theme::px(cx - ts.x * s * 0.5f, cy - ts.y * s * 0.5f));
                target.draw(spr);
            }
            else
            {

                Theme::drawIconCircle(target, m_font, {cx, cy}, ls * 0.5f,
                                      fadeC(Theme::ACCENT_PRESSED, a), "B",
                                      fadeC(Theme::TEXT_PRIMARY, a),
                                      static_cast<unsigned>(ls * 0.42f));
            }
        }
        y += ls + SP(16.f);
    }
    ++blk;

    {
        const float a = stage(blk);
        const float l1 = SP(46.f), l2 = SP(46.f);

        if (!measure)
            Theme::drawRadialGlow(target, {cx, y + (l1 + l2) * 0.45f},
                                  w * 0.66f, Theme::ACCENT,
                                  static_cast<std::uint8_t>(34 * a));

        centered("UNIVERSITY", CS(41.f), 21u, Theme::TEXT_PRIMARY,
                 sf::Text::Bold, 1.01f, l1, blk);
        centered("BUS TRACKER", CS(41.f), 21u, Theme::TEXT_PRIMARY,
                 sf::Text::Bold, 1.01f, l2, blk);
    }
    ++blk;

    {
        const float a = stage(blk);
        const float bw = std::max(52.f, SP(78.f));
        const float bh = std::max(3.f, SP(3.f));
        if (!measure)
            Theme::drawGradientRectH(target,
                                     {std::round(cx - bw * 0.5f), y + driftY(a)},
                                     {bw, bh},
                                     fadeC(Theme::ACCENT, a),
                                     fadeC(Theme::ACCENT_CYAN, a));
        y += bh + SP(16.f);
    }
    ++blk;

    centered("Real-time Campus Bus Management System", CS(15.f), 11u,
             Theme::TEXT_SECONDARY, sf::Text::Bold, 1.f, SP(26.f), blk);

    {
        const float a = stage(blk);
        const std::string s = "Developed by BUS(Y) CODERS";
        const unsigned cs = CS(14.f);
        const float chipH = SP(30.f);
        if (!measure)
        {
            sf::Text t = fitText(m_font, s, cs, 10u, w - SP(36.f),
                                 fadeC(sf::Color(226, 236, 255), a),
                                 sf::Text::Bold, 1.06f);
            const sf::FloatRect bb = t.getLocalBounds();
            const float chipW = bb.size.x + SP(34.f);
            const float chipX = cx - chipW * 0.5f;
            const float chipY = y + driftY(a);

            Theme::drawRoundedRectV(target, {chipX, chipY}, {chipW, chipH},
                                    chipH * 0.5f,
                                    fadeC(sf::Color(59, 130, 246, 62), a),
                                    fadeC(sf::Color(124, 58, 237, 62), a),
                                    1.f, fadeC(sf::Color(120, 160, 240, 95), a));
            t.setPosition(Theme::px(cx - (bb.position.x + bb.size.x * 0.5f),
                                    chipY + (chipH - bb.size.y) * 0.5f - bb.position.y));
            target.draw(t);
        }
        y += chipH + SP(13.f);
    }
    ++blk;

    centered("Smart. Simple. Reliable.", CS(13.f), 11u, Theme::ACCENT_CYAN,
             sf::Text::Bold, 1.05f, SP(24.f), blk);
    ++blk;

    if (!measure)
        Theme::drawSeparatorSoft(target, x, y, w,
                                 fadeC(sf::Color(120, 150, 210, 120), stage(blk)));
    y += SP(18.f);

    auto heading = [&](const std::string &label, bool crown, int b)
    {
        const float ih = SP(19.f);
        if (!measure)
        {
            const float a = stage(b);
            const float dy = driftY(a);
            if (crown)
                drawCrownIcon(target, {x, y + dy + SP(1.f)}, ih,
                              fadeC(Theme::WARNING, a));
            else
                drawTeamIcon(target, {x, y + dy}, ih, fadeC(Theme::ACCENT_HOVER, a));

            sf::Text t = fitText(m_font, label, CS(14.f), 11u,
                                 w - ih - SP(12.f),
                                 fadeC(Theme::TEXT_SECONDARY, a),
                                 sf::Text::Bold, 1.10f);
            const sf::FloatRect bb = t.getLocalBounds();
            t.setPosition(Theme::px(x + ih + SP(12.f) - bb.position.x,
                                    y + dy + (ih - bb.size.y) * 0.5f - bb.position.y));
            target.draw(t);
        }
        y += ih + SP(12.f);
    };

    heading("PROJECT LEADER", true, blk);
    leftLine("Md Wasif Tasin", x + SP(2.f), w - SP(2.f), CS(25.f), 15u,
             Theme::TEXT_PRIMARY, sf::Text::Bold, 1.f, SP(40.f), blk);
    ++blk;

    heading("TEAM MEMBERS", false, blk);
    {
        static const char *kMembers[] = {"Nishat Mahzaben", "Nur Safrin",
                                         "Jiaul Kabir Joy", "Samia Bhuiyan"};
        const float rowH = SP(29.f);
        const float dotR = std::max(2.f, SP(3.f));
        for (const char *name : kMembers)
        {
            if (!measure)
            {
                const float a = stage(blk);
                const float dy = driftY(a);

                sf::CircleShape dot(dotR, 16);
                dot.setOrigin({dotR, dotR});
                dot.setFillColor(fadeC(Theme::ACCENT_HOVER, a));
                dot.setPosition(Theme::px(x + SP(6.f), y + dy + rowH * 0.42f));
                target.draw(dot);

                sf::Text t = fitText(m_font, name, CS(18.f), 12u,
                                     w - SP(22.f), fadeC(Theme::TEXT_PRIMARY, a),
                                     sf::Text::Bold, 1.f);
                const sf::FloatRect bb = t.getLocalBounds();
                t.setPosition(Theme::px(x + SP(20.f) - bb.position.x,
                                        y + dy + rowH * 0.42f - bb.size.y * 0.5f - bb.position.y));
                target.draw(t);
            }
            y += rowH;
        }
        y += SP(8.f);
    }
    ++blk;

    if (!measure)
        Theme::drawSeparatorSoft(target, x, y, w,
                                 fadeC(sf::Color(120, 150, 210, 120), stage(blk)));
    y += SP(18.f);

    centered("Powered by C++17 & SFML 3", CS(13.f), 10u,
             Theme::lerp(Theme::ACCENT_HOVER, sf::Color::White, 0.30f),
             sf::Text::Bold, 1.f, SP(22.f), blk);
    centered("Department of Computer Science & Engineering", CS(13.f), 10u,
             Theme::TEXT_SECONDARY, sf::Text::Bold, 1.f, SP(22.f), blk);
    centered("Bangladesh University of Business and Technology (BUBT)",
             CS(13.f), 10u, Theme::TEXT_MUTED, sf::Text::Bold, 1.f,
             SP(24.f), blk);

    centered("Version 1.0", CS(12.f), 9u, Theme::TEXT_MUTED,
             sf::Text::Bold, 1.08f, SP(14.f), blk);

    return y - y0;
}

void HomeScreen::drawInfoPanel(sf::RenderTarget &target)
{
    const float a = Theme::smoothstep01(m_introT / 0.55f);

    Theme::drawGradientRectH(target, {0.f, 0.f}, {m_leftW, m_size.y},
                             fadeC(sf::Color(6, 10, 22, 130), a),
                             fadeC(sf::Color(6, 10, 22, 0), a));
    Theme::drawRadialGlow(target, {m_leftW * 0.30f, m_size.y * 0.16f},
                          m_leftW * 0.55f, Theme::ACCENT,
                          static_cast<std::uint8_t>(34 * a));
    Theme::drawRadialGlow(target, {m_leftW * 0.52f, m_size.y * 0.90f},
                          m_leftW * 0.44f, Theme::PURPLE,
                          static_cast<std::uint8_t>(30 * a));

    const float mx = std::clamp(m_leftW * 0.062f, 20.f, 40.f);
    const float my = std::clamp(m_size.y * 0.048f, 16.f, 36.f);
    const sf::Vector2f pos{mx, my};
    const sf::Vector2f dim{m_leftW - 2.f * mx, m_size.y - 2.f * my};
    if (dim.x <= 40.f || dim.y <= 40.f)
        return;

    const float R = 22.f;

    Theme::drawShadow(target, pos, dim, R, 26.f,
                      static_cast<std::uint8_t>(15 * a), 10.f, 8);
    Theme::drawRoundedRectV(target, pos, dim, R,
                            fadeC(sf::Color(44, 60, 98, 148), a),
                            fadeC(sf::Color(13, 19, 36, 196), a),
                            1.5f, fadeC(sf::Color(140, 168, 224, 62), a));

    Theme::fillRoundedRect(target, {pos.x + R, pos.y}, {dim.x - 2.f * R, 1.f},
                           0.f, fadeC(sf::Color(255, 255, 255, 46), a));
    Theme::drawGradientRectH(target, {pos.x + R * 0.9f, pos.y + 1.f},
                             {dim.x - 1.8f * R, 2.f},
                             fadeC(Theme::withAlpha(Theme::ACCENT_CYAN, 0), a),
                             fadeC(Theme::withAlpha(Theme::ACCENT, 120), a));

    const float padX = std::clamp(dim.x * 0.085f, 20.f, 34.f);
    const float padY = std::clamp(dim.y * 0.020f, 8.f, 14.f);
    const float innerX = pos.x + padX;
    const float innerW = dim.x - 2.f * padX;
    const float innerY = pos.y + padY;
    const float innerH = dim.y - 2.f * padY;

    float needH = paintInfo(target, innerX, innerW, innerY, 1.f, true);
    float k = 1.f;

    for (int i = 0; i < 7 && needH > innerH; ++i)
    {
        k = std::clamp(k * innerH / std::max(1.f, needH), 0.40f, 1.f);
        needH = paintInfo(target, innerX, innerW, innerY, k, true);
    }

    paintInfo(target, innerX, innerW,
              innerY + std::max(0.f, (innerH - needH) * 0.5f), k, false);
}

void GUI::run()
{

    sf::RenderWindow window(sf::VideoMode({1180u, 760u}),
                            "University Bus Tracker",
                            sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize,
                            sf::State::Windowed,
                            Theme::uiContext());

    window.setVerticalSyncEnabled(true);
    window.setMinimumSize(sf::Vector2u{860u, 620u});

    sf::Font font;
    if (!Theme::loadUiFont(font))
        return;

    ScreenManager app(window, font);
    app.run(std::make_unique<HomeScreen>(font));
}
