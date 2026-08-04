#include "TextBox.h"
#include "Theme.h"

#include <SFML/Window/Clipboard.hpp>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <limits>

static constexpr float EYE_AREA_W = 38.f;
static constexpr unsigned CHAR_SIZE = Theme::Type::BODY;
static constexpr float    PAD_L     = 14.f;

static bool isWordChar(char c) {
    unsigned char u = static_cast<unsigned char>(c);
    return std::isalnum(u) != 0 || c == '_';
}

TextBox::TextBox(sf::Font& f, sf::Vector2f size, sf::Vector2f pos)
    : m_pos(pos), m_size(size), m_font(f),
      m_value(""), m_placeholder(""),
      m_focused(false), m_passwordMode(false), m_showText(false), m_glowT(0.f),
      m_caret(0)
{}

std::string TextBox::displayString() const {
    bool mask = m_passwordMode && !m_showText;
    return mask ? std::string(m_value.size(), '*') : m_value;
}

sf::Text TextBox::makeText(const std::string& s) const {
    return Theme::makeText(m_font, s, CHAR_SIZE, Theme::TEXT_PRIMARY);
}

float TextBox::innerWidth() const {
    return m_size.x - PAD_L - (m_passwordMode ? EYE_AREA_W + 8.f : PAD_L);
}

bool TextBox::fits(const std::string& candidate) const {
    bool mask = m_passwordMode && !m_showText;
    sf::Text check = makeText(mask ? std::string(candidate.size(), '*') : candidate);
    return check.getLocalBounds().size.x < innerWidth();
}
float TextBox::caretOffset(const sf::Text& t, std::size_t i) const {
    const auto& glyphs = t.getShapedGlyphs();
    if (glyphs.empty()) return 0.f;
    const sf::Text::ShapedGlyph* best = nullptr;
    for (const auto& g : glyphs) {
        if (g.cluster < static_cast<std::uint32_t>(i)) continue;
        if (!best || g.cluster < best->cluster) best = &g;
    }
    if (best) return best->position.x;
    const auto& last = glyphs.back();         
    return last.position.x + last.glyph.advance;
}
std::size_t TextBox::caretIndexAt(float mouseX) const {
    sf::Text t = makeText(displayString());
    const float originX = std::round(m_pos.x + PAD_L);

    std::size_t best = 0;
    float bestDist = std::numeric_limits<float>::max();
    for (std::size_t i = 0; i <= m_value.size(); ++i) {
        float d = std::abs(originX + caretOffset(t, i) - mouseX);
        if (d < bestDist) { bestDist = d; best = i; }
    }
    return best;
}

std::size_t TextBox::prevWord(std::size_t i) const {
    while (i > 0 && !isWordChar(m_value[i - 1])) --i;
    while (i > 0 &&  isWordChar(m_value[i - 1])) --i;
    return i;
}

std::size_t TextBox::nextWord(std::size_t i) const {
    const std::size_t n = m_value.size();
    while (i < n && !isWordChar(m_value[i])) ++i;
    while (i < n &&  isWordChar(m_value[i])) ++i;
    return i;
}

void TextBox::insertText(const std::string& s) {
    for (char c : s) {
        if (c < 32 || c > 126) continue;            
        std::string temp = m_value;
        temp.insert(m_caret, 1, c);
        if (!fits(temp)) break;                  
        m_value = temp;
        ++m_caret;
    }
}


void TextBox::draw(sf::RenderWindow& window) {
    m_glowT = std::clamp(m_glowT + (m_focused ? 0.1f : -0.1f), 0.f, 1.f);

    const float RADIUS = 9.f;

    // Rounded halo that follows the field's own shape (the old one was a
    // hard-cornered rectangle sitting behind a rounded box).
    if (m_glowT > 0.01f) {
        Theme::drawGlow(window, m_pos, m_size, RADIUS, Theme::ACCENT,
                        9.f, static_cast<std::uint8_t>(20 * m_glowT), 5);
    }

    // Inset field: darker than the card it sits on, darkest at the top, so it
    // reads as carved in rather than floating.
    sf::Color bgTop = Theme::lerp(sf::Color(11, 17, 33), sf::Color(18, 27, 50), m_glowT);
    sf::Color bgBot = Theme::lerp(sf::Color(17, 25, 46), sf::Color(26, 38, 68), m_glowT);
    sf::Color borderColor = Theme::lerp(Theme::BORDER_IDLE, Theme::BORDER_FOCUS, m_glowT);
    Theme::drawRoundedRectV(window, m_pos, m_size, RADIUS, bgTop, bgBot,
                            1.5f, borderColor);

    if (m_passwordMode) {
        sf::Vector2f eyeCenter = Theme::px(
            m_pos.x + m_size.x - EYE_AREA_W * 0.5f,
            m_pos.y + m_size.y * 0.5f
        );
        sf::RectangleShape div({1.f, m_size.y * 0.55f});
        div.setPosition(Theme::px(m_pos.x + m_size.x - EYE_AREA_W,
                                  m_pos.y + m_size.y * 0.225f));
        div.setFillColor(Theme::BORDER_IDLE);
        window.draw(div);
        Theme::drawEyeIcon(window, eyeCenter, 9.f,
                           /*slashed=*/!m_showText,
                           m_showText ? Theme::ACCENT_HOVER : Theme::TEXT_SECONDARY);
    }

    bool showPH = m_value.empty() && !m_focused && !m_placeholder.empty();
    std::string display = showPH ? m_placeholder : displayString();

    sf::Text txt = makeText(display);
    txt.setFillColor(showPH ? Theme::TEXT_MUTED : Theme::TEXT_PRIMARY);
    sf::FloatRect ref = makeText("Ag").getLocalBounds();
    float ty = Theme::centeredTextY(m_font, CHAR_SIZE, m_pos.y, m_size.y);
    txt.setPosition({std::round(m_pos.x + PAD_L), ty});
    window.draw(txt);
    if (m_focused) {
        bool cursorOn = (m_cursorClock.getElapsedTime().asMilliseconds() / 530) % 2 == 0;
        if (cursorOn) {
            float caretH = ref.size.y + 6.f;
            float cx     = std::round(txt.getPosition().x + caretOffset(txt, m_caret));
            float cy     = m_pos.y + (m_size.y - caretH) * 0.5f;
            Theme::fillRoundedRect(window, Theme::px(cx, cy), {2.f, caretH},
                                   1.f, Theme::ACCENT_HOVER);
        }
    }
}
void TextBox::handleEvent(const sf::Event& event) {
    if (const auto* mb = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mb->button == sf::Mouse::Button::Left) {
            sf::Vector2f m{static_cast<float>(mb->position.x),
                           static_cast<float>(mb->position.y)};
            if (m_passwordMode) {
                float eyeLeft = m_pos.x + m_size.x - EYE_AREA_W;
                sf::FloatRect eyeArea({eyeLeft, m_pos.y}, {EYE_AREA_W, m_size.y});
                if (eyeArea.contains(m)) {
                    m_showText = !m_showText;
                    return; 
                }
            }

            if (getBounds().contains(m)) {
                m_caret = caretIndexAt(m.x);
                m_cursorClock.restart();
            }
            return;
        }
    }
    if (!m_focused) return;

    if (const auto* kp = event.getIf<sf::Event::KeyPressed>()) {
        using Key = sf::Keyboard::Key;
        switch (kp->code) {
            case Key::Left:
                m_caret = kp->control ? prevWord(m_caret)
                                      : (m_caret > 0 ? m_caret - 1 : 0);
                break;
            case Key::Right:
                m_caret = kp->control ? nextWord(m_caret)
                                      : std::min(m_caret + 1, m_value.size());
                break;
            case Key::Home:
            case Key::Up:
                m_caret = 0;
                break;
            case Key::End:
            case Key::Down:
                m_caret = m_value.size();
                break;
            case Key::Delete:
                if (kp->control) {                 
                    std::size_t to = nextWord(m_caret);
                    m_value.erase(m_caret, to - m_caret);
                } else if (m_caret < m_value.size()) {
                    m_value.erase(m_caret, 1);
                }
                break;
            case Key::V:
                if (kp->control) insertText(sf::Clipboard::getString().toAnsiString());
                break;
            default:
                return;                             
        }
        m_cursorClock.restart();                    
        return;
    }

    if (const auto* te = event.getIf<sf::Event::TextEntered>()) {
        uint32_t u = te->unicode;
        if (u == 8) {                                  
            if (m_caret > 0) { m_value.erase(m_caret - 1, 1); --m_caret; }
        } else if (u == 127) {                         
            std::size_t from = prevWord(m_caret);
            m_value.erase(from, m_caret - from);
            m_caret = from;
        } else if (u >= 32 && u <= 126) {              
            insertText(std::string(1, static_cast<char>(u)));
        } else {
            return;
        }
        m_cursorClock.restart();
    }
}

void TextBox::setFocused(bool f) {
    if (f != m_focused) {          
        m_cursorClock.restart();    
        if (f) m_caret = std::min(m_caret, m_value.size());
    }                               
    m_focused = f;
}

std::string TextBox::getText() const { return m_value; }

void TextBox::setPlaceholder(const std::string& ph) { m_placeholder = ph; }
void TextBox::setPasswordMode(bool pm)               { m_passwordMode = pm; if (!pm) m_showText = false; }
void TextBox::clear()                                { m_value = ""; m_showText = false; m_caret = 0; }
void TextBox::setText(const std::string& t)          { m_value = t; m_caret = m_value.size(); }
void TextBox::setPosition(sf::Vector2f p)            { m_pos   = p; }
sf::FloatRect TextBox::getBounds() const             { return {m_pos, m_size}; }
