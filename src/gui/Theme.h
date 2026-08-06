#ifndef THEME_H
#define THEME_H

#include <SFML/Graphics.hpp>
#include <string>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <system_error>
#include <vector>

namespace Theme
{

    inline constexpr float PI = 3.14159265358979f;

    inline const sf::Color BG_DARK = sf::Color(13, 18, 32);
    inline const sf::Color BG_CARD = sf::Color(23, 31, 51);
    inline const sf::Color BG_CARD_DARK = sf::Color(18, 25, 42);
    inline const sf::Color BG_HEADER = sf::Color(10, 14, 25);

    inline const sf::Color ACCENT = sf::Color(56, 114, 199);
    inline const sf::Color ACCENT_HOVER = sf::Color(86, 145, 226);
    inline const sf::Color ACCENT_PRESSED = sf::Color(40, 88, 160);

    inline const sf::Color SUCCESS = sf::Color(52, 150, 96);
    inline const sf::Color SUCCESS_DARK = sf::Color(36, 118, 74);
    inline const sf::Color DANGER = sf::Color(196, 74, 74);
    inline const sf::Color DANGER_HOVER = sf::Color(216, 106, 106);
    inline const sf::Color DANGER_DARK = sf::Color(156, 46, 46);
    inline const sf::Color WARNING = sf::Color(198, 156, 74);

    inline const sf::Color PURPLE = sf::Color(109, 74, 176);
    inline const sf::Color PURPLE_HOVER = sf::Color(134, 102, 200);

    inline const sf::Color TEXT_PRIMARY = sf::Color(238, 242, 248);
    inline const sf::Color TEXT_SECONDARY = sf::Color(196, 205, 220);
    inline const sf::Color TEXT_MUTED = sf::Color(140, 152, 172);
    inline const sf::Color TEXT_ROUTE = sf::Color(126, 138, 158);

    inline const sf::Color BORDER_IDLE = sf::Color(46, 58, 80);
    inline const sf::Color BORDER_FOCUS = sf::Color(86, 133, 196);

    inline const sf::Color BADGE_UNI_BG = sf::Color(34, 62, 110);
    inline const sf::Color BADGE_UNI_TEXT = sf::Color(226, 236, 250);
    inline const sf::Color BADGE_UNI_EDGE = sf::Color(70, 112, 176);

    inline const sf::Color BADGE_BUS_BG = sf::Color(64, 46, 106);
    inline const sf::Color BADGE_BUS_TEXT = sf::Color(228, 220, 244);
    inline const sf::Color BADGE_BUS_EDGE = sf::Color(120, 92, 186);

    inline const sf::Color ITEM_BG = sf::Color(19, 26, 45);
    inline const sf::Color ITEM_HOVER = sf::Color(32, 43, 70);
    inline const sf::Color ITEM_SELECTED = sf::Color(38, 74, 140);

    inline const sf::Color SIDEBAR_HOVER = sf::Color(25, 35, 58);
    inline const sf::Color SIDEBAR_SELECTED = sf::Color(38, 74, 140);

    inline const sf::Color BG_DEEP = sf::Color(7, 10, 18);

    inline const sf::Color ACCENT_CYAN = sf::Color(90, 170, 190);
    inline const sf::Color ACCENT_INDIGO = sf::Color(92, 104, 178);
    inline const sf::Color PURPLE_PINK = sf::Color(166, 88, 166);

    inline const sf::Color SHEEN = sf::Color(255, 255, 255, 10);

    inline sf::Vector2f px(float x, float y)
    {
        return {std::round(x), std::round(y)};
    }
    inline sf::Vector2f px(sf::Vector2f v)
    {
        return {std::round(v.x), std::round(v.y)};
    }

    inline void drawEyeIcon(sf::RenderTarget &target, sf::Vector2f center,
                            float size, bool slashed, sf::Color col)
    {
        const int N = 14;
        const float w = size;
        const float h = size * 0.55f;

        sf::VertexArray topArc(sf::PrimitiveType::LineStrip, N);
        sf::VertexArray botArc(sf::PrimitiveType::LineStrip, N);
        for (int i = 0; i < N; ++i)
        {
            float t = static_cast<float>(i) / (N - 1);
            float x = center.x - w + 2.f * w * t;
            float yarc = -h * std::sin(3.14159265358979f * t);
            topArc[i].position = {x, center.y + yarc};
            topArc[i].color = col;
            botArc[i].position = {x, center.y - yarc};
            botArc[i].color = col;
        }
        target.draw(topArc);
        target.draw(botArc);

        float irisR = h * 0.68f;
        sf::CircleShape iris(irisR, 16);
        iris.setFillColor(col);
        iris.setOrigin({irisR, irisR});
        iris.setPosition(center);
        target.draw(iris);

        if (slashed)
        {
            float len = size * 2.6f;
            sf::RectangleShape slash({len, 2.f});
            slash.setFillColor(col);
            slash.setOrigin({len * 0.5f, 1.f});
            slash.setPosition(center);
            slash.setRotation(sf::degrees(-38.f));
            target.draw(slash);
        }
    }

    inline bool loadUiFont(sf::Font &font)
    {
        if (!font.openFromFile("assets/Inter-Regular.ttf") &&
            !font.openFromFile("assets/Roboto-Regular.ttf"))
            return false;
        font.setSmooth(false);
        return true;
    }

    inline sf::ContextSettings uiContext()
    {
        sf::ContextSettings s;
        s.antiAliasingLevel = 8;
        return s;
    }
    inline void syncViewToWindow(sf::RenderWindow &window, const sf::Event &event)
    {
        if (const auto *r = event.getIf<sf::Event::Resized>())
            window.setView(sf::View(sf::FloatRect(
                {0.f, 0.f},
                {static_cast<float>(r->size.x), static_cast<float>(r->size.y)})));
    }

    inline sf::Texture &backgroundTexture()
    {
        static sf::Texture tex;
        return tex;
    }

    inline bool &backgroundReady()
    {
        static bool ready = false;
        return ready;
    }

    inline void loadBackgroundImage()
    {
        static bool tried = false;
        if (tried)
            return;
        tried = true;

        static const char *const candidates[] = {
            "assets/background.png",
            "assets/background.jpg",
            "assets/background.jpeg",
        };

        sf::Texture &tex = backgroundTexture();
        for (const char *path : candidates)
        {
            std::error_code ec;
            if (!std::filesystem::exists(path, ec) || ec)
                continue;
            if (tex.loadFromFile(path))
            {
                tex.setSmooth(true);
                backgroundReady() = true;
                return;
            }
        }
    }

    inline bool drawBackgroundImage(sf::RenderTarget &target, sf::Vector2f size,
                                    std::uint8_t scrim = 145)
    {
        loadBackgroundImage();
        if (!backgroundReady())
            return false;

        const sf::Texture &tex = backgroundTexture();
        const sf::Vector2u ts = tex.getSize();
        if (ts.x == 0 || ts.y == 0)
            return false;

        const float scale = std::max(size.x / static_cast<float>(ts.x),
                                     size.y / static_cast<float>(ts.y));

        sf::Sprite spr(tex);
        spr.setScale({scale, scale});
        spr.setPosition({(size.x - ts.x * scale) * 0.5f,
                         (size.y - ts.y * scale) * 0.5f});
        target.draw(spr);

        if (scrim > 0)
        {
            sf::RectangleShape veil(size);
            veil.setFillColor(sf::Color(8, 12, 24, scrim));
            target.draw(veil);
        }
        return true;
    }

    namespace Type
    {
        inline constexpr unsigned H1 = 32;
        inline constexpr unsigned DISPLAY = 30;
        inline constexpr unsigned TITLE = 25;
        inline constexpr unsigned H3 = 24;
        inline constexpr unsigned BUS_NAME = 23;
        inline constexpr unsigned BADGE_BUS = 21;
        inline constexpr unsigned HEADING = 20;
        inline constexpr unsigned BADGE_UNI = 19;
        inline constexpr unsigned INPUT = 18;
        inline constexpr unsigned SUBTITLE = 17;
        inline constexpr unsigned BODY = 15;
        inline constexpr unsigned SMALL = 14;
        inline constexpr unsigned META = 14;
        inline constexpr unsigned ROUTE = 13;
        inline constexpr unsigned LABEL = 12;
        inline constexpr unsigned CAPTION = 11;

        inline constexpr float LEADING_BODY = 1.35f;
    }

    inline sf::Color withAlpha(sf::Color c, uint8_t a)
    {
        return sf::Color(c.r, c.g, c.b, a);
    }

    inline constexpr float HOVER_RATE_IN = 19.7f;
    inline constexpr float HOVER_RATE_OUT = 10.5f;

    inline float approach(float current, float target, float rate, float dt)
    {
        if (dt <= 0.f)
            return current;
        current += (target - current) * (1.f - std::exp(-rate * dt));
        return (std::abs(target - current) < 0.005f) ? target : current;
    }

    inline float approachHover(float current, bool on, float dt)
    {
        return approach(current, on ? 1.f : 0.f,
                        on ? HOVER_RATE_IN : HOVER_RATE_OUT, dt);
    }

    inline float smoothstep01(float t)
    {
        t = std::clamp(t, 0.f, 1.f);
        return t * t * (3.f - 2.f * t);
    }

    inline sf::Color lerp(sf::Color a, sf::Color b, float t)
    {
        t = std::clamp(t, 0.f, 1.f);
        return sf::Color(
            static_cast<uint8_t>(a.r + (int(b.r) - int(a.r)) * t),
            static_cast<uint8_t>(a.g + (int(b.g) - int(a.g)) * t),
            static_cast<uint8_t>(a.b + (int(b.b) - int(a.b)) * t),
            static_cast<uint8_t>(a.a + (int(b.a) - int(a.a)) * t));
    }

    inline sf::Text makeText(const sf::Font &font, const std::string &str,
                             unsigned charSize, sf::Color color,
                             std::uint32_t style = sf::Text::Regular)
    {
        sf::Text t(font);
        t.setString(str);
        t.setCharacterSize(charSize);
        t.setFillColor(color);
        t.setStyle(style);
        return t;
    }

    inline float textWidth(const sf::Font &font, const std::string &str,
                           unsigned charSize, std::uint32_t style = sf::Text::Regular)
    {
        return makeText(font, str, charSize, sf::Color::White, style).getLocalBounds().size.x;
    }
    inline void drawText(sf::RenderTarget &target, const sf::Font &font,
                         const std::string &str, unsigned charSize,
                         sf::Color color, sf::Vector2f pos,
                         std::uint32_t style = sf::Text::Regular,
                         float lineSpacing = 1.f)
    {
        sf::Text t = makeText(font, str, charSize, color, style);
        if (lineSpacing != 1.f)
            t.setLineSpacing(lineSpacing);
        t.setPosition(px(pos));
        target.draw(t);
    }

    inline float centeredTextY(const sf::Font &font, unsigned charSize,
                               float rectY, float rectH,
                               std::uint32_t style = sf::Text::Regular)
    {
        sf::FloatRect ref = makeText(font, "Ag", charSize, sf::Color::White, style).getLocalBounds();
        return std::round(rectY + (rectH - ref.size.y) * 0.5f - ref.position.y);
    }

    inline void drawTextVCentered(sf::RenderTarget &target, const sf::Font &font,
                                  const std::string &str, unsigned charSize,
                                  sf::Color color, float x,
                                  float rectY, float rectH,
                                  std::uint32_t style = sf::Text::Regular)
    {
        drawText(target, font, str, charSize, color,
                 {x, centeredTextY(font, charSize, rectY, rectH, style)}, style);
    }

    inline void drawTextHCentered(sf::RenderTarget &target, const sf::Font &font,
                                  const std::string &str, unsigned charSize,
                                  sf::Color color, float cx, float y,
                                  std::uint32_t style = sf::Text::Regular)
    {
        sf::Text t = makeText(font, str, charSize, color, style);
        sf::FloatRect b = t.getLocalBounds();
        t.setPosition(px(cx - (b.position.x + b.size.x * 0.5f), y));
        target.draw(t);
    }

    inline std::string ellipsize(const sf::Font &font, const std::string &str,
                                 unsigned charSize, float maxW,
                                 std::uint32_t style = sf::Text::Regular)
    {
        if (textWidth(font, str, charSize, style) <= maxW)
            return str;
        std::string out = str;
        while (!out.empty() &&
               textWidth(font, out + "...", charSize, style) > maxW)
            out.pop_back();
        while (!out.empty() && (out.back() == ' ' || out.back() == ','))
            out.pop_back();
        return out + "...";
    }

    inline void fillRoundedRectV(sf::RenderTarget &target,
                                 sf::Vector2f pos, sf::Vector2f size,
                                 float radius, sf::Color topColor, sf::Color botColor,
                                 int cornerSegments = 8)
    {
        if (size.x <= 0.f || size.y <= 0.f)
            return;
        radius = std::clamp(radius, 0.f, std::min(size.x, size.y) * 0.5f);
        cornerSegments = std::max(1, cornerSegments);

        struct Corner
        {
            sf::Vector2f c;
            float a0;
        };
        const Corner corners[4] = {
            {{pos.x + size.x - radius, pos.y + radius}, -PI * 0.5f},
            {{pos.x + size.x - radius, pos.y + size.y - radius}, 0.f},
            {{pos.x + radius, pos.y + size.y - radius}, PI * 0.5f},
            {{pos.x + radius, pos.y + radius}, PI},
        };

        std::vector<sf::Vector2f> pts;
        pts.reserve(4 * (cornerSegments + 1));
        for (const auto &k : corners)
            for (int i = 0; i <= cornerSegments; ++i)
            {
                float a = k.a0 + (PI * 0.5f) * (static_cast<float>(i) / cornerSegments);
                pts.push_back({k.c.x + std::cos(a) * radius, k.c.y + std::sin(a) * radius});
            }

        auto colAt = [&](float y)
        {
            return lerp(topColor, botColor, (y - pos.y) / size.y);
        };

        sf::VertexArray fan(sf::PrimitiveType::TriangleFan, pts.size() + 2);
        sf::Vector2f center{pos.x + size.x * 0.5f, pos.y + size.y * 0.5f};
        fan[0].position = center;
        fan[0].color = colAt(center.y);
        for (std::size_t i = 0; i < pts.size(); ++i)
        {
            fan[i + 1].position = pts[i];
            fan[i + 1].color = colAt(pts[i].y);
        }
        fan[pts.size() + 1] = fan[1];
        target.draw(fan);
    }

    inline void fillRoundedRect(sf::RenderTarget &target,
                                sf::Vector2f pos, sf::Vector2f size,
                                float radius, sf::Color color)
    {
        fillRoundedRectV(target, pos, size, radius, color, color);
    }

    inline void drawShadow(sf::RenderTarget &target,
                           sf::Vector2f pos, sf::Vector2f size, float radius,
                           float spread = 14.f, std::uint8_t strength = 18,
                           float offsetY = 6.f, int layers = 7)
    {
        if (layers < 1 || size.x <= 0.f || size.y <= 0.f)
            return;
        for (int i = layers; i >= 1; --i)
        {
            float e = spread * (static_cast<float>(i) / layers);
            fillRoundedRect(target,
                            {pos.x - e, pos.y - e + offsetY},
                            {size.x + 2.f * e, size.y + 2.f * e},
                            radius + e, sf::Color(0, 0, 0, strength));
        }
    }

    inline void drawGlow(sf::RenderTarget &target,
                         sf::Vector2f pos, sf::Vector2f size, float radius,
                         sf::Color color, float spread = 10.f,
                         std::uint8_t strength = 26, int layers = 5)
    {
        if (layers < 1 || size.x <= 0.f || size.y <= 0.f)
            return;
        for (int i = layers; i >= 1; --i)
        {
            float e = spread * (static_cast<float>(i) / layers);
            fillRoundedRect(target,
                            {pos.x - e, pos.y - e},
                            {size.x + 2.f * e, size.y + 2.f * e},
                            radius + e, withAlpha(color, strength));
        }
    }

    inline void drawRadialGlow(sf::RenderTarget &target, sf::Vector2f center,
                               float radius, sf::Color color,
                               std::uint8_t centerAlpha, int segments = 56)
    {
        if (radius <= 0.f)
            return;
        sf::VertexArray fan(sf::PrimitiveType::TriangleFan, segments + 2);
        fan[0].position = center;
        fan[0].color = withAlpha(color, centerAlpha);
        for (int i = 0; i <= segments; ++i)
        {
            float a = 2.f * PI * (static_cast<float>(i) / segments);
            fan[i + 1].position = {center.x + std::cos(a) * radius,
                                   center.y + std::sin(a) * radius};
            fan[i + 1].color = withAlpha(color, 0);
        }
        target.draw(fan);
    }

    inline void drawRoundedRect(sf::RenderTarget &target,
                                sf::Vector2f pos, sf::Vector2f size,
                                float radius, sf::Color fillColor,
                                float borderW = 0.f,
                                sf::Color borderColor = sf::Color::Transparent)
    {
        if (borderW > 0.f && borderColor != sf::Color::Transparent)
        {
            fillRoundedRect(target,
                            {pos.x - borderW, pos.y - borderW},
                            {size.x + 2.f * borderW, size.y + 2.f * borderW},
                            radius + borderW, borderColor);
        }
        fillRoundedRect(target, pos, size, radius, fillColor);
    }

    inline void drawRoundedRectV(sf::RenderTarget &target,
                                 sf::Vector2f pos, sf::Vector2f size,
                                 float radius, sf::Color topColor, sf::Color botColor,
                                 float borderW = 0.f,
                                 sf::Color borderColor = sf::Color::Transparent)
    {
        if (borderW > 0.f && borderColor != sf::Color::Transparent)
        {
            fillRoundedRect(target,
                            {pos.x - borderW, pos.y - borderW},
                            {size.x + 2.f * borderW, size.y + 2.f * borderW},
                            radius + borderW, borderColor);
        }
        fillRoundedRectV(target, pos, size, radius, topColor, botColor);
    }

    inline void drawCard(sf::RenderTarget &target,
                         sf::Vector2f pos, sf::Vector2f size,
                         sf::Color color = BG_CARD, float radius = 10.f)
    {

        fillRoundedRectV(target, pos, size, radius,
                         lerp(color, sf::Color::White, 0.06f),
                         lerp(color, sf::Color::Black, 0.10f));

        if (size.y > 6.f && size.x > 2.f * radius)
            fillRoundedRect(target, {pos.x + radius, pos.y}, {size.x - 2.f * radius, 1.f},
                            0.f, SHEEN);
    }

    inline void drawCardElevated(sf::RenderTarget &target,
                                 sf::Vector2f pos, sf::Vector2f size,
                                 sf::Color color = BG_CARD, float radius = 12.f,
                                 float spread = 18.f, std::uint8_t strength = 16)
    {
        drawShadow(target, pos, size, radius, spread, strength);
        drawCard(target, pos, size, color, radius);
    }

    inline void drawSeparatorSoft(sf::RenderTarget &target,
                                  float x, float y, float w,
                                  sf::Color color = BORDER_IDLE)
    {
        if (w <= 0.f)
            return;
        const float yy = std::round(y);
        const sf::Color e = withAlpha(color, 0);
        sf::VertexArray s(sf::PrimitiveType::TriangleStrip, 6);
        s[0].position = {x, yy};
        s[0].color = e;
        s[1].position = {x, yy + 1.f};
        s[1].color = e;
        s[2].position = {x + w * 0.5f, yy};
        s[2].color = color;
        s[3].position = {x + w * 0.5f, yy + 1.f};
        s[3].color = color;
        s[4].position = {x + w, yy};
        s[4].color = e;
        s[5].position = {x + w, yy + 1.f};
        s[5].color = e;
        target.draw(s);
    }

    inline void drawAccentBar(sf::RenderTarget &target,
                              float x, float y, float h,
                              sf::Color color = ACCENT, float w = 4.f)
    {

        float inset = std::min(12.f, h * 0.14f);
        fillRoundedRectV(target, px(x + 2.f, y + inset), {w, h - 2.f * inset}, w * 0.5f,
                         lerp(color, sf::Color::White, 0.32f), color);
    }
    inline float drawBadge(sf::RenderTarget &target, const sf::Font &font,
                           const std::string &label, sf::Vector2f pos,
                           sf::Color bg, sf::Color textColor,
                           unsigned charSize = Type::BADGE_UNI,
                           sf::Color edgeColor = sf::Color::Transparent,
                           float minWidth = 48.f)
    {
        const std::uint32_t style = sf::Text::Bold;
        const float padX = 12.f, padY = 8.f;

        sf::Text t = makeText(font, label, charSize, textColor, style);
        sf::FloatRect ink = t.getLocalBounds();
        sf::FloatRect cap = makeText(font, "H", charSize, textColor, style).getLocalBounds();

        float bw = std::max(minWidth, std::round(ink.size.x + 2.f * padX));
        float bh = std::round(cap.size.y + 2.f * padY);

        sf::Vector2f p = px(pos);
        if (edgeColor != sf::Color::Transparent)
            drawRoundedRect(target, p, {bw, bh}, bh * 0.5f, bg, 1.f, edgeColor);
        else
            fillRoundedRect(target, p, {bw, bh}, bh * 0.5f, bg);

        t.setPosition(px(p.x + (bw - ink.size.x) * 0.5f - ink.position.x,
                         p.y + (bh - cap.size.y) * 0.5f - cap.position.y));
        target.draw(t);
        return bw;
    }

    inline float badgeHeight(const sf::Font &font, unsigned charSize = Type::BADGE_UNI)
    {
        return std::round(makeText(font, "H", charSize, sf::Color::White,
                                   sf::Text::Bold)
                              .getLocalBounds()
                              .size.y +
                          16.f);
    }

    inline void drawCenteredText(sf::RenderTarget &target, const sf::Font &font,
                                 const std::string &str, unsigned charSize,
                                 sf::Color color, sf::FloatRect rect,
                                 std::uint32_t style = sf::Text::Regular)
    {
        sf::Text t = makeText(font, str, charSize, color, style);
        sf::FloatRect b = t.getLocalBounds();
        t.setPosition(px(rect.position.x + (rect.size.x - b.size.x) * 0.5f - b.position.x,
                         rect.position.y + (rect.size.y - b.size.y) * 0.5f - b.position.y));
        target.draw(t);
    }

    inline void drawGradientRect(sf::RenderTarget &target,
                                 sf::Vector2f pos, sf::Vector2f size,
                                 sf::Color topColor, sf::Color botColor)
    {
        sf::VertexArray quad(sf::PrimitiveType::TriangleStrip, 4);
        quad[0].position = {pos.x, pos.y};
        quad[1].position = {pos.x + size.x, pos.y};
        quad[2].position = {pos.x, pos.y + size.y};
        quad[3].position = {pos.x + size.x, pos.y + size.y};
        quad[0].color = topColor;
        quad[1].color = topColor;
        quad[2].color = botColor;
        quad[3].color = botColor;
        target.draw(quad);
    }

    inline void drawAppBase(sf::RenderTarget &target, sf::Vector2f size,
                            sf::Color top = BG_DARK, sf::Color bottom = BG_DEEP,
                            std::uint8_t scrim = 178)
    {
        if (drawBackgroundImage(target, size, scrim))
            return;
        drawGradientRect(target, {0.f, 0.f}, size, top, bottom);
    }

    inline void drawIconCircle(sf::RenderTarget &target, const sf::Font &font,
                               sf::Vector2f center, float radius,
                               sf::Color circleColor, const std::string &letter,
                               sf::Color letterColor, unsigned letterSize = 20)
    {

        drawRadialGlow(target, center, radius * 2.1f, letterColor, 20);

        sf::CircleShape ring(radius, 48);
        ring.setFillColor(sf::Color::Transparent);
        ring.setOutlineThickness(1.5f);
        ring.setOutlineColor(withAlpha(letterColor, 110));
        ring.setOrigin({radius, radius});
        ring.setPosition(px(center));

        sf::VertexArray disc(sf::PrimitiveType::TriangleFan, 50);
        disc[0].position = center;
        disc[0].color = lerp(circleColor, sf::Color::White, 0.10f);
        for (int i = 0; i <= 48; ++i)
        {
            float a = 2.f * PI * (static_cast<float>(i) / 48);
            disc[i + 1].position = {center.x + std::cos(a) * radius,
                                    center.y + std::sin(a) * radius};
            disc[i + 1].color = circleColor;
        }
        target.draw(disc);
        target.draw(ring);

        drawCenteredText(target, font, letter, letterSize, letterColor,
                         {{center.x - radius, center.y - radius}, {radius * 2.f, radius * 2.f}},
                         sf::Text::Bold);
    }

    inline void drawBackdrop(sf::RenderTarget &target, float w, float h,
                             sf::Color glowA = ACCENT, sf::Color glowB = PURPLE)
    {

        if (drawBackgroundImage(target, {w, h}))
            return;

        drawGradientRect(target, {0.f, 0.f}, {w, h}, BG_DARK, BG_DEEP);
        const float r = std::max(w, h);
        drawRadialGlow(target, {w * 0.16f, h * 0.08f}, r * 0.60f, glowA, 30);
        drawRadialGlow(target, {w * 0.92f, h * 0.96f}, r * 0.55f, glowB, 26);
        drawRadialGlow(target, {w * 0.75f, h * 0.15f}, r * 0.35f, ACCENT_CYAN, 12);
    }

    inline void drawGradientRectH(sf::RenderTarget &target,
                                  sf::Vector2f pos, sf::Vector2f size,
                                  sf::Color leftColor, sf::Color rightColor)
    {
        sf::VertexArray quad(sf::PrimitiveType::TriangleStrip, 4);
        quad[0].position = {pos.x, pos.y};
        quad[0].color = leftColor;
        quad[1].position = {pos.x, pos.y + size.y};
        quad[1].color = leftColor;
        quad[2].position = {pos.x + size.x, pos.y};
        quad[2].color = rightColor;
        quad[3].position = {pos.x + size.x, pos.y + size.y};
        quad[3].color = rightColor;
        target.draw(quad);
    }

    inline void drawSidebarBackdrop(sf::RenderTarget &target, float w, float h,
                                    sf::Color glow = ACCENT)
    {
        drawGradientRect(target, {0.f, 0.f}, {w, h},
                         sf::Color(19, 27, 50), sf::Color(9, 13, 26));
        drawRadialGlow(target, {w * 0.5f, h * 0.10f}, w * 1.5f, glow, 26);

        fillRoundedRectV(target, {w - 1.5f, 0.f}, {1.5f, h}, 0.f,
                         withAlpha(glow, 100), withAlpha(glow, 25));

        drawGradientRectH(target, {w, 0.f}, {10.f, h},
                          sf::Color(0, 0, 0, 60), sf::Color(0, 0, 0, 0));
    }

    inline void drawNavItem(sf::RenderTarget &target, float sidebarW,
                            float y, float h, bool active, float hoverT,
                            sf::Color accent = ACCENT)
    {
        hoverT = std::clamp(hoverT, 0.f, 1.f);

        if (active)
            fillRoundedRectV(target, {8.f, y}, {sidebarW - 18.f, h}, 10.f,
                             lerp(SIDEBAR_SELECTED, sf::Color::White, 0.12f),
                             SIDEBAR_SELECTED);
        else if (hoverT > 0.004f)
            fillRoundedRect(target, {8.f, y}, {sidebarW - 18.f, h}, 10.f,
                            withAlpha(SIDEBAR_HOVER,
                                      static_cast<uint8_t>(255 * hoverT)));

        if (active)
            fillRoundedRect(target, {0.f, y + h * 0.22f}, {3.f, h * 0.56f}, 1.5f,
                            lerp(accent, sf::Color::White, 0.30f));
        else if (hoverT > 0.004f)
            fillRoundedRect(target, {0.f, y + h * (0.5f - 0.28f * hoverT)},
                            {3.f, h * 0.56f * hoverT}, 1.5f,
                            withAlpha(accent, static_cast<uint8_t>(150 * hoverT)));
    }

    inline void drawHeaderBar(sf::RenderTarget &target, float x, float w, float h,
                              sf::Color accent = ACCENT, sf::Color accent2 = PURPLE)
    {
        drawGradientRect(target, {x, 0.f}, {w, h}, sf::Color(15, 21, 38), BG_HEADER);

        sf::VertexArray line(sf::PrimitiveType::TriangleStrip, 6);
        const float y0 = h - 2.f, y1 = h;
        const sf::Color midC = lerp(accent, accent2, 0.5f);
        line[0].position = {x, y0};
        line[0].color = accent;
        line[1].position = {x, y1};
        line[1].color = accent;
        line[2].position = {x + w * 0.5f, y0};
        line[2].color = midC;
        line[3].position = {x + w * 0.5f, y1};
        line[3].color = midC;
        line[4].position = {x + w, y0};
        line[4].color = withAlpha(accent2, 130);
        line[5].position = {x + w, y1};
        line[5].color = withAlpha(accent2, 130);
        target.draw(line);

        drawGradientRect(target, {x, h}, {w, 10.f},
                         sf::Color(0, 0, 0, 55), sf::Color(0, 0, 0, 0));
    }

    inline void drawInfoToast(sf::RenderTarget &target, const sf::Font &font,
                              const std::string &msg, bool isError,
                              float windowW, float windowH, float alpha = 1.f)
    {
        sf::Color base = isError ? DANGER_DARK : sf::Color(15, 80, 40);
        sf::Color edge = isError ? DANGER : SUCCESS;
        sf::Color txt = withAlpha(TEXT_PRIMARY, static_cast<uint8_t>(255 * alpha));

        float tw = std::min(500.f, windowW - 40.f);
        float th = 50.f;
        float tx = std::round((windowW - tw) * 0.5f);
        float ty = std::round(windowH - th - 18.f);
        float r = th * 0.5f;

        drawShadow(target, {tx, ty}, {tw, th}, r, 16.f,
                   static_cast<std::uint8_t>(20 * alpha), 6.f);
        drawRoundedRectV(target, {tx, ty}, {tw, th}, r,
                         withAlpha(lerp(base, sf::Color::White, 0.10f),
                                   static_cast<uint8_t>(242 * alpha)),
                         withAlpha(base, static_cast<uint8_t>(242 * alpha)),
                         1.f, withAlpha(edge, static_cast<uint8_t>(150 * alpha)));

        float dotR = 4.f;
        sf::CircleShape dot(dotR, 20);
        dot.setFillColor(withAlpha(edge, static_cast<uint8_t>(255 * alpha)));
        dot.setOrigin({dotR, dotR});
        dot.setPosition(px(tx + 22.f, ty + th * 0.5f));
        target.draw(dot);

        drawCenteredText(target, font, msg, Type::BODY, txt,
                         {{tx + 16.f, ty}, {tw - 16.f, th}}, sf::Text::Bold);
    }

    inline void drawAppBackground(sf::RenderTarget &target, sf::Vector2f size)
    {
        drawAppBase(target, size, BG_DEEP, BG_DARK);
    }

    inline void drawChevron(sf::RenderTarget &target, sf::Vector2f pos, float size,
                            sf::Color color)
    {
        sf::VertexArray arrow(sf::PrimitiveType::LineStrip, 3);
        arrow[0].position = {pos.x, pos.y - size};
        arrow[1].position = {pos.x + size, pos.y};
        arrow[2].position = {pos.x, pos.y + size};
        arrow[0].color = arrow[1].color = arrow[2].color = color;
        target.draw(arrow);
    }

}

#endif
