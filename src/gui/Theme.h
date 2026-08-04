#ifndef THEME_H
#define THEME_H

#include <SFML/Graphics.hpp>
#include <string>
#include <algorithm>
#include <cmath>

namespace Theme {

// ─── Color Palette ──────────────────────────────────────────────────────────
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

inline const sf::Color TEXT_PRIMARY     = sf::Color(241, 245, 249);
inline const sf::Color TEXT_SECONDARY   = sf::Color(148, 163, 184);
inline const sf::Color TEXT_MUTED       = sf::Color(100, 116, 139);

inline const sf::Color BORDER_IDLE      = sf::Color(51,  65,  85 );
inline const sf::Color BORDER_FOCUS     = sf::Color(59,  130, 246);

inline const sf::Color ITEM_BG          = sf::Color(20,  30,  55 );
inline const sf::Color ITEM_HOVER       = sf::Color(36,  48,  80 );
inline const sf::Color ITEM_SELECTED    = sf::Color(29,  78,  216);

inline const sf::Color SIDEBAR_HOVER    = sf::Color(28,  40,  68 );
inline const sf::Color SIDEBAR_SELECTED = sf::Color(29,  78,  216);


// ─── Pixel-perfect helpers ───────────────────────────────────────────────────

// Round a position to the nearest integer pixel — eliminates sub-pixel blur.
inline sf::Vector2f px(float x, float y) {
    return { std::round(x), std::round(y) };
}
inline sf::Vector2f px(sf::Vector2f v) {
    return { std::round(v.x), std::round(v.y) };
}

// ─── Eye icon (password show/hide toggle) ───────────────────────────────────
// slashed = true  → password is hidden  (eye with slash = "click to reveal")
// slashed = false → password is visible (open eye      = "click to hide")
inline void drawEyeIcon(sf::RenderTarget& target, sf::Vector2f center,
                         float size, bool slashed, sf::Color col)
{
    const int  N  = 14;          // arc smoothness
    const float w = size;        // half-width of lens
    const float h = size * 0.55f;// half-height of lens

    // Top and bottom arcs forming the almond / lens shape
    sf::VertexArray topArc(sf::PrimitiveType::LineStrip, N);
    sf::VertexArray botArc(sf::PrimitiveType::LineStrip, N);
    for (int i = 0; i < N; ++i) {
        float t   = static_cast<float>(i) / (N - 1);       // 0 → 1
        float x   = center.x - w + 2.f * w * t;
        float yarc= -h * std::sin(3.14159265358979f * t);
        topArc[i].position = {x, center.y + yarc};
        topArc[i].color    = col;
        botArc[i].position = {x, center.y - yarc};
        botArc[i].color    = col;
    }
    target.draw(topArc);
    target.draw(botArc);

    // Filled iris circle in the centre
    float irisR = h * 0.68f;
    sf::CircleShape iris(irisR, 16);
    iris.setFillColor(col);
    iris.setOrigin({irisR, irisR});
    iris.setPosition(center);
    target.draw(iris);

    // Slash line drawn on top (for hidden/masked state)
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

// Load a font and configure it for crisp screen rendering.
// Call once per screen after openFromFile().
inline void configureFont(sf::Font& font) {
    // Disable bi-linear texture smoothing so glyph edges stay crisp.
    // SFML internally sets the glyph atlas texture smooth flag here.
    font.setSmooth(false);
}


// ─── Color Utilities ────────────────────────────────────────────────────────
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


// ─── Core Drawing Helpers ───────────────────────────────────────────────────

// Filled rounded rectangle
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

// Rounded rect with optional border
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

// Card shorthand
inline void drawCard(sf::RenderTarget& target,
                     sf::Vector2f pos, sf::Vector2f size,
                     sf::Color color = BG_CARD, float radius = 10.f)
{
    fillRoundedRect(target, pos, size, radius, color);
}

// Horizontal separator
inline void drawSeparator(sf::RenderTarget& target,
                           float x, float y, float w,
                           sf::Color color = BORDER_IDLE)
{
    sf::RectangleShape line({w, 1.f});
    line.setPosition(px(x, y));
    line.setFillColor(color);
    target.draw(line);
}

// Left accent bar
inline void drawAccentBar(sf::RenderTarget& target,
                           float x, float y, float h,
                           sf::Color color = ACCENT, float w = 4.f)
{
    sf::RectangleShape bar({w, h});
    bar.setPosition(px(x, y));
    bar.setFillColor(color);
    target.draw(bar);
}

// Small pill / badge — positions snapped to pixels
inline void drawBadge(sf::RenderTarget& target, const sf::Font& font,
                       const std::string& label, sf::Vector2f pos,
                       sf::Color bg, sf::Color textColor = sf::Color(255, 255, 255))
{
    sf::Text t(font);
    t.setString(label);
    t.setCharacterSize(12);
    sf::FloatRect b = t.getLocalBounds();
    const float px_ = 8.f, py_ = 3.f;
    float bw = std::round(b.size.x + 2.f * px_);
    float bh = std::round(b.size.y + 2.f * py_ + 2.f);
    fillRoundedRect(target, px(pos), {bw, bh}, bh * 0.5f, bg);
    t.setFillColor(textColor);
    // Snap text inside badge to integers
    t.setPosition(px(pos.x + px_ - b.position.x,
                     pos.y + py_ - b.position.y));
    target.draw(t);
}

// Text centered in a rect — position snapped to integer pixels
inline void drawCenteredText(sf::RenderTarget& target, const sf::Font& font,
                              const std::string& str, unsigned charSize,
                              sf::Color color, sf::FloatRect rect)
{
    sf::Text t(font);
    t.setString(str);
    t.setCharacterSize(charSize);
    t.setFillColor(color);
    sf::FloatRect b = t.getLocalBounds();
    // Compute ideal centre then snap to integer
    float idealX = rect.position.x + rect.size.x * 0.5f;
    float idealY = rect.position.y + rect.size.y * 0.5f;
    t.setOrigin({b.position.x + b.size.x * 0.5f,
                 b.position.y + b.size.y * 0.5f});
    t.setPosition(px(idealX, idealY));
    target.draw(t);
}

// Gradient background quad
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

// Letter inside a coloured circle — snapped to integer pixels
inline void drawIconCircle(sf::RenderTarget& target, const sf::Font& font,
                            sf::Vector2f center, float radius,
                            sf::Color circleColor, const std::string& letter,
                            sf::Color letterColor, unsigned letterSize = 20)
{
    sf::CircleShape circle(radius, 32);
    circle.setFillColor(circleColor);
    circle.setPosition(px(center.x - radius, center.y - radius));
    target.draw(circle);

    sf::Text lt(font);
    lt.setString(letter);
    lt.setCharacterSize(letterSize);
    lt.setFillColor(letterColor);
    sf::FloatRect b = lt.getLocalBounds();
    lt.setOrigin({b.position.x + b.size.x * 0.5f,
                  b.position.y + b.size.y * 0.5f});
    lt.setPosition(px(center));   // snap icon letter
    target.draw(lt);
}

// Auto-fade toast notification — snapped positions
inline void drawInfoToast(sf::RenderTarget& target, const sf::Font& font,
                           const std::string& msg, bool isError,
                           float windowW, float windowH, float alpha = 1.f)
{
    sf::Color bg  = isError ? withAlpha(DANGER_DARK, static_cast<uint8_t>(220 * alpha))
                            : withAlpha(sf::Color(15, 80, 40), static_cast<uint8_t>(220 * alpha));
    sf::Color txt = withAlpha(TEXT_PRIMARY, static_cast<uint8_t>(255 * alpha));

    float tw = std::min(500.f, windowW - 40.f);
    float th = 46.f;
    float tx = std::round((windowW - tw) * 0.5f);
    float ty = std::round(windowH - th - 16.f);

    fillRoundedRect(target, {tx, ty}, {tw, th}, 8.f, bg);

    sf::Text t(font);
    t.setString(msg);
    t.setCharacterSize(14);
    t.setFillColor(txt);
    sf::FloatRect b = t.getLocalBounds();
    t.setOrigin({b.position.x + b.size.x * 0.5f,
                 b.position.y + b.size.y * 0.5f});
    t.setPosition(px(tx + tw * 0.5f, ty + th * 0.5f));
    target.draw(t);
}

} // namespace Theme

#endif // THEME_H
