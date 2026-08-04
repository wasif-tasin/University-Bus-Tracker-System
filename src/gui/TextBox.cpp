#include "TextBox.h"
#include "Theme.h"

#include <algorithm>
#include <cmath>

// Eye icon hit-area: right 38px of the box
static constexpr float EYE_AREA_W = 38.f;

TextBox::TextBox(sf::Font& f, sf::Vector2f size, sf::Vector2f pos)
    : m_pos(pos), m_size(size), m_font(f),
      m_value(""), m_placeholder(""),
      m_focused(false), m_passwordMode(false), m_showText(false), m_glowT(0.f)
{}

void TextBox::draw(sf::RenderWindow& window) {
    // Animate focus glow
    m_glowT = std::clamp(m_glowT + (m_focused ? 0.1f : -0.1f), 0.f, 1.f);

    // Soft glow halo
    if (m_glowT > 0.01f) {
        sf::RectangleShape glow({m_size.x + 10.f, m_size.y + 10.f});
        glow.setPosition({m_pos.x - 5.f, m_pos.y - 5.f});
        glow.setFillColor(sf::Color(59, 130, 246, static_cast<uint8_t>(45 * m_glowT)));
        window.draw(glow);
    }

    // Border & background
    sf::Color bgColor     = m_focused ? sf::Color(22, 32, 60) : sf::Color(15, 22, 40);
    sf::Color borderColor = Theme::lerp(Theme::BORDER_IDLE, Theme::BORDER_FOCUS, m_glowT);
    Theme::drawRoundedRect(window, m_pos, m_size, 7.f, bgColor, 1.5f, borderColor);

    // ── Eye icon (password mode only) ────────────────────────────────────
    if (m_passwordMode) {
        sf::Vector2f eyeCenter = Theme::px(
            m_pos.x + m_size.x - EYE_AREA_W * 0.5f,
            m_pos.y + m_size.y * 0.5f
        );
        // Hover highlight: thin vertical divider line
        sf::RectangleShape div({1.f, m_size.y * 0.55f});
        div.setPosition(Theme::px(m_pos.x + m_size.x - EYE_AREA_W,
                                  m_pos.y + m_size.y * 0.225f));
        div.setFillColor(Theme::BORDER_IDLE);
        window.draw(div);

        // slashed = password is hidden (prompt: "click to show")
        Theme::drawEyeIcon(window, eyeCenter, 9.f,
                           /*slashed=*/!m_showText,
                           Theme::TEXT_SECONDARY);
    }

    // ── Input text / placeholder ──────────────────────────────────────────
    bool showPH = m_value.empty() && !m_focused && !m_placeholder.empty();
    std::string display;
    if (showPH) {
        display = m_placeholder;
    } else {
        // Mask with '*' unless user clicked the eye to reveal
        bool mask = m_passwordMode && !m_showText;
        display   = mask ? std::string(m_value.size(), '*') : m_value;
        if (m_focused) {
            bool cursorOn = (m_cursorClock.getElapsedTime().asMilliseconds() / 530) % 2 == 0;
            if (cursorOn) display += '|';
        }
    }

    sf::Text txt(m_font);
    txt.setString(display);
    txt.setCharacterSize(15);
    txt.setFillColor(showPH ? Theme::TEXT_MUTED : Theme::TEXT_PRIMARY);

    // Vertical centering — snapped to integer pixel
    sf::FloatRect b = txt.getLocalBounds();
    float ty = std::round(m_pos.y + (m_size.y - b.size.y) * 0.5f - b.position.y);
    txt.setPosition({std::round(m_pos.x + 14.f), ty});
    window.draw(txt);
}

void TextBox::handleEvent(const sf::Event& event) {
    // ── Eye icon click — works regardless of whether box is focused ───────
    if (m_passwordMode) {
        if (const auto* mb = event.getIf<sf::Event::MouseButtonPressed>()) {
            if (mb->button == sf::Mouse::Button::Left) {
                float eyeLeft = m_pos.x + m_size.x - EYE_AREA_W;
                sf::FloatRect eyeArea({eyeLeft, m_pos.y},
                                      {EYE_AREA_W, m_size.y});
                if (eyeArea.contains({static_cast<float>(mb->position.x),
                                      static_cast<float>(mb->position.y)})) {
                    m_showText = !m_showText;
                    return; // consumed — don't also process text below
                }
            }
        }
    }

    // ── Text input — only when this box has focus ─────────────────────────
    if (!m_focused) return;

    if (const auto* te = event.getIf<sf::Event::TextEntered>()) {
        uint32_t u = te->unicode;
        if (u == 8) {                                   // Backspace
            if (!m_value.empty()) m_value.pop_back();
        } else if (u >= 32 && u <= 126) {               // Printable ASCII
            std::string temp = m_value + static_cast<char>(u);
            sf::Text check(m_font);
            check.setCharacterSize(15);
            bool mask = m_passwordMode && !m_showText;
            check.setString(mask ? std::string(temp.size(), '*') : temp);
            // Reserve space for the eye icon when in password mode
            float maxW = m_size.x - 14.f - (m_passwordMode ? EYE_AREA_W + 8.f : 14.f);
            if (check.getLocalBounds().size.x < maxW)
                m_value = temp;
        }
    }
}

void TextBox::setFocused(bool f) {
    m_focused = f;
    if (f) m_cursorClock.restart();
}

std::string TextBox::getText() const { return m_value; }

void TextBox::setPlaceholder(const std::string& ph) { m_placeholder = ph; }
void TextBox::setPasswordMode(bool pm)               { m_passwordMode = pm; if (!pm) m_showText = false; }
void TextBox::clear()                                { m_value = ""; m_showText = false; }
void TextBox::setText(const std::string& t)          { m_value = t; }
void TextBox::setPosition(sf::Vector2f p)            { m_pos   = p; }
sf::FloatRect TextBox::getBounds() const             { return {m_pos, m_size}; }