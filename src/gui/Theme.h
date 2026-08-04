#ifndef THEME_H
#define THEME_H

#include <SFML/Graphics.hpp>
#include <string>
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace Theme {

inline const sf::Color BG_DARK          = sf::Color(15,  22,  40 );
inline const sf::Color BG_CARD          = sf::Color(26,  37,  64 );
inline const sf::Color BG_CARD_HOVER    = sf::Color(34,  50,  86 );
inline const sf::Color BG_HEADER        = sf::Color(10,  15,  28 );
inline const sf::Color BG_SIDEBAR       = sf::Color(13,  18,  32 );

inline const sf::Color ACCENT           = sf::Color(59,  130, 246);
inline const sf::Color ACCENT_HOVER     = sf::Color(96,  165, 250);
inline const sf::Color ACCENT_PRESSED   = sf::Color(37,  99,  235);
inline const sf::Color ACCENT_DIM       = sf::Color(59,  130, 246,  50);

inline const sf::Color SUCCESS          = sf::Color(34,  197, 94 );
inline const sf::Color SUCCESS_DARK     = sf::Color(22,  163, 74 );
inline const sf::Color DANGER           = sf::Color(239, 68,  68 );
inline const sf::Color DANGER_HOVER     = sf::Color(248, 113, 113);
inline const sf::Color DANGER_DARK      = sf::Color(185, 28,  28 );
inline const sf::Color WARNING          = sf::Color(234, 179, 8  );

inline const sf::Color PURPLE           = sf::Color(124, 58,  237);
inline const sf::Color PURPLE_HOVER     = sf::Color(139, 92,  246);
inline const sf::Color PURPLE_DIM       = sf::Color(124, 58,  237,  50);

inline const sf::Color TEXT_PRIMARY     = sf::Color(255, 255, 255); 
inline const sf::Color TEXT_SECONDARY   = sf::Color(215, 220, 231);
inline const sf::Color TEXT_MUTED       = sf::Color(154, 164, 181);
inline const sf::Color TEXT_ROUTE       = sf::Color(138, 149, 168);

inline const sf::Color BORDER_IDLE      = sf::Color(51,  65,  85 );
inline const sf::Color BORDER_FOCUS     = sf::Color(59,  130, 246);

inline const sf::Color BADGE_UNI_BG     = sf::Color(37,  71,  132);
inline const sf::Color BADGE_UNI_TEXT   = sf::Color(235, 244, 255);
inline const sf::Color BADGE_UNI_EDGE   = sf::Color(74,  126, 208);

inline const sf::Color BADGE_BUS_BG     = sf::Color(74,  48,  130);
inline const sf::Color BADGE_BUS_TEXT   = sf::Color(233, 224, 255);
inline const sf::Color BADGE_BUS_EDGE   = sf::Color(133, 96,  222);

inline const sf::Color ITEM_BG          = sf::Color(20,  30,  55 );
inline const sf::Color ITEM_HOVER       = sf::Color(36,  48,  80 );
inline const sf::Color ITEM_SELECTED    = sf::Color(29,  78,  216);

inline const sf::Color SIDEBAR_HOVER    = sf::Color(28,  40,  68 );
inline const sf::Color SIDEBAR_SELECTED = sf::Color(29,  78,  216);


inline sf::Vector2f px(float x, float y) {
    return { std::round(x), std::round(y) };
}
inline sf::Vector2f px(sf::Vector2f v) {
    return { std::round(v.x), std::round(v.y) };
}

inline void drawEyeIcon(sf::RenderTarget& target, sf::Vector2f center,
                         float size, bool slashed, sf::Color col)
{
    const int  N  = 14;        
    const float w = size;        
    const float h = size * 0.55f;

    sf::VertexArray topArc(sf::PrimitiveType::LineStrip, N);
    sf::VertexArray botArc(sf::PrimitiveType::LineStrip, N);
    for (int i = 0; i < N; ++i) {
        float t   = static_cast<float>(i) / (N - 1);      
        float x   = center.x - w + 2.f * w * t;
        float yarc= -h * std::sin(3.14159265358979f * t);
        topArc[i].position = {x, center.y + yarc};
        topArc[i].color    = col;
        botArc[i].position = {x, center.y - yarc};
        botArc[i].color    = col;
    }
    target.draw(topArc);
    target.draw(botArc);

    float irisR = h * 0.68f;
    sf::CircleShape iris(irisR, 16);
    iris.setFillColor(col);
    iris.setOrigin({irisR, irisR});
    iris.setPosition(center);
    target.draw(iris);

    if (slashed) {
        float len = size * 2.6f;
        sf::RectangleShape slash({len, 2.f});
        slash.setFillColor(col);
        slash.setOrigin({len * 0.5f, 1.f});
        slash.setPosition(center);
        slash.setRotation(sf::degrees(-38.f));
        target.draw(slash);
    }
}

inline void configureFont(sf::Font& font) {
    font.setSmooth(false);
}
inline bool loadUIFont(sf::Font& font) {
    if (!font.openFromFile("assets/Inter-Regular.ttf") &&
        !font.openFromFile("assets/Roboto-Regular.ttf"))
        return false;
    configureFont(font);
    return true;
}
inline void syncViewToWindow(sf::RenderWindow& window, const sf::Event& event) {
    if (const auto* r = event.getIf<sf::Event::Resized>())
        window.setView(sf::View(sf::FloatRect(
            {0.f, 0.f},
            {static_cast<float>(r->size.x), static_cast<float>(r->size.y)})));
}


namespace Type {
    inline constexpr unsigned DISPLAY   = 30;  
    inline constexpr unsigned TITLE     = 25; 
    inline constexpr unsigned BUS_NAME  = 23; 
    inline constexpr unsigned BADGE_BUS = 21;  
    inline constexpr unsigned HEADING   = 20; 
    inline constexpr unsigned BADGE_UNI = 19; 
    inline constexpr unsigned SUBTITLE  = 17;  
    inline constexpr unsigned BODY      = 15; 
    inline constexpr unsigned META      = 14; 
    inline constexpr unsigned ROUTE     = 13; 
    inline constexpr unsigned LABEL     = 12; 
    inline constexpr unsigned CAPTION   = 11;  

    inline constexpr float LEADING_BODY = 1.35f; 
}


inline sf::Color withAlpha(sf::Color c, uint8_t a) {
    return sf::Color(c.r, c.g, c.b, a);
}

inline sf::Color lerp(sf::Color a, sf::Color b, float t) {
    t = std::clamp(t, 0.f, 1.f);
    return sf::Color(
        static_cast<uint8_t>(a.r + (int(b.r) - int(a.r)) * t),
        static_cast<uint8_t>(a.g + (int(b.g) - int(a.g)) * t),
        static_cast<uint8_t>(a.b + (int(b.b) - int(a.b)) * t),
        static_cast<uint8_t>(a.a + (int(b.a) - int(a.a)) * t)
    );
}

inline sf::Text makeText(const sf::Font& font, const std::string& str,
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

inline float textWidth(const sf::Font& font, const std::string& str,
                        unsigned charSize, std::uint32_t style = sf::Text::Regular)
{
    return makeText(font, str, charSize, sf::Color::White, style).getLocalBounds().size.x;
}
inline void drawText(sf::RenderTarget& target, const sf::Font& font,
                      const std::string& str, unsigned charSize,
                      sf::Color color, sf::Vector2f pos,
                      std::uint32_t style = sf::Text::Regular,
                      float lineSpacing = 1.f)
{
    sf::Text t = makeText(font, str, charSize, color, style);
    if (lineSpacing != 1.f) t.setLineSpacing(lineSpacing);
    t.setPosition(px(pos));
    target.draw(t);
}

inline float centeredTextY(const sf::Font& font, unsigned charSize,
                            float rectY, float rectH,
                            std::uint32_t style = sf::Text::Regular)
{
    sf::FloatRect ref = makeText(font, "Ag", charSize, sf::Color::White, style).getLocalBounds();
    return std::round(rectY + (rectH - ref.size.y) * 0.5f - ref.position.y);
}

inline void drawTextVCentered(sf::RenderTarget& target, const sf::Font& font,
                               const std::string& str, unsigned charSize,
                               sf::Color color, float x,
                               float rectY, float rectH,
                               std::uint32_t style = sf::Text::Regular)
{
    drawText(target, font, str, charSize, color,
             {x, centeredTextY(font, charSize, rectY, rectH, style)}, style);
}

inline void drawTextHCentered(sf::RenderTarget& target, const sf::Font& font,
                               const std::string& str, unsigned charSize,
                               sf::Color color, float cx, float y,
                               std::uint32_t style = sf::Text::Regular)
{
    sf::Text t = makeText(font, str, charSize, color, style);
    sf::FloatRect b = t.getLocalBounds();
    t.setPosition(px(cx - (b.position.x + b.size.x * 0.5f), y));
    target.draw(t);
}

inline std::string ellipsize(const sf::Font& font, const std::string& str,
                              unsigned charSize, float maxW,
                              std::uint32_t style = sf::Text::Regular)
{
    if (textWidth(font, str, charSize, style) <= maxW) return str;
    std::string out = str;
    while (!out.empty() &&
           textWidth(font, out + "...", charSize, style) > maxW)
        out.pop_back();
    while (!out.empty() && (out.back() == ' ' || out.back() == ','))
        out.pop_back();
    return out + "...";
}

inline void fillRoundedRect(sf::RenderTarget& target,
                             sf::Vector2f pos, sf::Vector2f size,
                             float radius, sf::Color color)
{
    if (size.x <= 0.f || size.y <= 0.f) return;
    radius = std::clamp(radius, 0.f, std::min(size.x, size.y) * 0.5f);

    sf::RectangleShape mid({size.x, size.y - 2.f * radius});
    mid.setPosition({pos.x, pos.y + radius});
    mid.setFillColor(color);
    target.draw(mid);

    sf::RectangleShape top({size.x - 2.f * radius, radius});
    top.setPosition({pos.x + radius, pos.y});
    top.setFillColor(color);
    target.draw(top);

    sf::RectangleShape bot({size.x - 2.f * radius, radius});
    bot.setPosition({pos.x + radius, pos.y + size.y - radius});
    bot.setFillColor(color);
    target.draw(bot);

    auto corner = [&](float cx, float cy) {
        sf::CircleShape c(radius, 14);
        c.setFillColor(color);
        c.setPosition({cx - radius, cy - radius});
        target.draw(c);
    };
    corner(pos.x + radius,          pos.y + radius         );
    corner(pos.x + size.x - radius, pos.y + radius         );
    corner(pos.x + radius,          pos.y + size.y - radius);
    corner(pos.x + size.x - radius, pos.y + size.y - radius);
}

inline void drawRoundedRect(sf::RenderTarget& target,
                             sf::Vector2f pos, sf::Vector2f size,
                             float radius, sf::Color fillColor,
                             float borderW = 0.f,
                             sf::Color borderColor = sf::Color::Transparent)
{
    if (borderW > 0.f && borderColor != sf::Color::Transparent) {
        fillRoundedRect(target,
                        {pos.x - borderW, pos.y - borderW},
                        {size.x + 2.f * borderW, size.y + 2.f * borderW},
                        radius + borderW, borderColor);
    }
    fillRoundedRect(target, pos, size, radius, fillColor);
}

inline void drawCard(sf::RenderTarget& target,
                     sf::Vector2f pos, sf::Vector2f size,
                     sf::Color color = BG_CARD, float radius = 10.f)
{
    fillRoundedRect(target, pos, size, radius, color);
}

inline void drawSeparator(sf::RenderTarget& target,
                           float x, float y, float w,
                           sf::Color color = BORDER_IDLE)
{
    sf::RectangleShape line({w, 1.f});
    line.setPosition(px(x, y));
    line.setFillColor(color);
    target.draw(line);
}

inline void drawAccentBar(sf::RenderTarget& target,
                           float x, float y, float h,
                           sf::Color color = ACCENT, float w = 4.f)
{
    sf::RectangleShape bar({w, h});
    bar.setPosition(px(x, y));
    bar.setFillColor(color);
    target.draw(bar);
}
inline float drawBadge(sf::RenderTarget& target, const sf::Font& font,
                       const std::string& label, sf::Vector2f pos,
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

inline float badgeHeight(const sf::Font& font, unsigned charSize = Type::BADGE_UNI) {
    return std::round(makeText(font, "H", charSize, sf::Color::White,
                               sf::Text::Bold).getLocalBounds().size.y + 16.f);
}

inline void drawCenteredText(sf::RenderTarget& target, const sf::Font& font,
                              const std::string& str, unsigned charSize,
                              sf::Color color, sf::FloatRect rect,
                              std::uint32_t style = sf::Text::Regular)
{
    sf::Text t = makeText(font, str, charSize, color, style);
    sf::FloatRect b = t.getLocalBounds();
    t.setPosition(px(rect.position.x + (rect.size.x - b.size.x) * 0.5f - b.position.x,
                     rect.position.y + (rect.size.y - b.size.y) * 0.5f - b.position.y));
    target.draw(t);
}

inline void drawGradientRect(sf::RenderTarget& target,
                              sf::Vector2f pos, sf::Vector2f size,
                              sf::Color topColor, sf::Color botColor)
{
    sf::VertexArray quad(sf::PrimitiveType::TriangleStrip, 4);
    quad[0].position = {pos.x,           pos.y          };
    quad[1].position = {pos.x + size.x,  pos.y          };
    quad[2].position = {pos.x,           pos.y + size.y };
    quad[3].position = {pos.x + size.x,  pos.y + size.y };
    quad[0].color = topColor;
    quad[1].color = topColor;
    quad[2].color = botColor;
    quad[3].color = botColor;
    target.draw(quad);
}

inline void drawIconCircle(sf::RenderTarget& target, const sf::Font& font,
                            sf::Vector2f center, float radius,
                            sf::Color circleColor, const std::string& letter,
                            sf::Color letterColor, unsigned letterSize = 20)
{
    sf::CircleShape circle(radius, 32);
    circle.setFillColor(circleColor);
    circle.setPosition(px(center.x - radius, center.y - radius));
    target.draw(circle);

    drawCenteredText(target, font, letter, letterSize, letterColor,
                     {{center.x - radius, center.y - radius}, {radius * 2.f, radius * 2.f}},
                     sf::Text::Bold);
}

inline void drawInfoToast(sf::RenderTarget& target, const sf::Font& font,
                           const std::string& msg, bool isError,
                           float windowW, float windowH, float alpha = 1.f)
{
    sf::Color bg  = isError ? withAlpha(DANGER_DARK, static_cast<uint8_t>(230 * alpha))
                            : withAlpha(sf::Color(15, 80, 40), static_cast<uint8_t>(230 * alpha));
    sf::Color txt = withAlpha(TEXT_PRIMARY, static_cast<uint8_t>(255 * alpha));

    float tw = std::min(500.f, windowW - 40.f);
    float th = 48.f;
    float tx = std::round((windowW - tw) * 0.5f);
    float ty = std::round(windowH - th - 16.f);

    fillRoundedRect(target, {tx, ty}, {tw, th}, 8.f, bg);
    drawCenteredText(target, font, msg, Type::BODY, txt,
                     {{tx, ty}, {tw, th}}, sf::Text::Bold);
}

} 

#endif 
