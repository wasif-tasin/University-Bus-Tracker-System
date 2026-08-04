#include "TextBox.h"
#include "Theme.h"

#include <SFML/Window/Clipboard.hpp>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <limits>

// Eye icon hit-area: right 38px of the box
static constexpr float EYE_AREA_W = 38.f;
// Text metrics shared by drawing, measuring and hit-testing.
// Kept in one place because caret placement measures the same sf::Text that
// gets drawn — a mismatch here shows up as a caret that drifts off the glyphs.
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

// ── Internal helpers ──────────────────────────────────────────────────────

std::string TextBox::displayString() const {
    bool mask = m_passwordMode && !m_showText;
    return mask ? std::string(m_value.size(), '*') : m_value;
}

sf::Text TextBox::makeText(const std::string& s) const {
    return Theme::makeText(m_font, s, CHAR_SIZE, Theme::TEXT_PRIMARY);
}

float TextBox::innerWidth() const {
    // Reserve space for the eye icon when in password mode
    return m_size.x - PAD_L - (m_passwordMode ? EYE_AREA_W + 8.f : PAD_L);
}

bool TextBox::fits(const std::string& candidate) const {
    bool mask = m_passwordMode && !m_showText;
    sf::Text check = makeText(mask ? std::string(candidate.size(), '*') : candidate);
    return check.getLocalBounds().size.x < innerWidth();
}

// Local x offset (from the text's own origin) of the caret gap before index i.
float TextBox::caretOffset(const sf::Text& t, std::size_t i) const {
    const auto& glyphs = t.getShapedGlyphs();
    if (glyphs.empty()) return 0.f;
    // First glyph at or after index i — tolerant of clusters that span
    // several characters, so the caret never silently snaps to the end.
    const sf::Text::ShapedGlyph* best = nullptr;
    for (const auto& g : glyphs) {
        if (g.cluster < static_cast<std::uint32_t>(i)) continue;
        if (!best || g.cluster < best->cluster) best = &g;
    }
    if (best) return best->position.x;
    const auto& last = glyphs.back();          // at/after the end of the string
    return last.position.x + last.glyph.advance;
}

// Nearest character gap to a window x coordinate — used for click-to-place.
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

// Insert at the caret, one char at a time so the width limit still applies.
void TextBox::insertText(const std::string& s) {
    for (char c : s) {
        if (c < 32 || c > 126) continue;            // printable ASCII only
        std::string temp = m_value;
        temp.insert(m_caret, 1, c);
        if (!fits(temp)) break;                     // box is full
        m_value = temp;
        ++m_caret;
    }
}

// ── Drawing ───────────────────────────────────────────────────────────────

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
    std::string display = showPH ? m_placeholder : displayString();

    sf::Text txt = makeText(display);
    txt.setFillColor(showPH ? Theme::TEXT_MUTED : Theme::TEXT_PRIMARY);

    // Vertical centering — measured off a fixed reference so the baseline
    // doesn't jump as the content changes. Snapped to integer pixel.
    sf::FloatRect ref = makeText("Ag").getLocalBounds();
    float ty = Theme::centeredTextY(m_font, CHAR_SIZE, m_pos.y, m_size.y);
    txt.setPosition({std::round(m_pos.x + PAD_L), ty});
    window.draw(txt);

    // ── Caret ─────────────────────────────────────────────────────────────
    if (m_focused) {
        bool cursorOn = (m_cursorClock.getElapsedTime().asMilliseconds() / 530) % 2 == 0;
        if (cursorOn) {
            float caretH = ref.size.y + 6.f;
            float cx     = std::round(txt.getPosition().x + caretOffset(txt, m_caret));
            sf::RectangleShape caret({1.5f, caretH});
            caret.setPosition(Theme::px(cx, m_pos.y + (m_size.y - caretH) * 0.5f));
            caret.setFillColor(Theme::TEXT_PRIMARY);
            window.draw(caret);
        }
    }
}

// ── Input ─────────────────────────────────────────────────────────────────

void TextBox::handleEvent(const sf::Event& event) {
    // ── Mouse — handled even while unfocused, because the owning screen
    //    only applies the new focus on the next frame ────────────────────
    if (const auto* mb = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mb->button == sf::Mouse::Button::Left) {
            sf::Vector2f m{static_cast<float>(mb->position.x),
                           static_cast<float>(mb->position.y)};

            // Eye icon click — works regardless of whether box is focused
            if (m_passwordMode) {
                float eyeLeft = m_pos.x + m_size.x - EYE_AREA_W;
                sf::FloatRect eyeArea({eyeLeft, m_pos.y}, {EYE_AREA_W, m_size.y});
                if (eyeArea.contains(m)) {
                    m_showText = !m_showText;
                    return; // consumed — don't also move the caret
                }
            }

            // Click inside the box places the caret at the nearest gap
            if (getBounds().contains(m)) {
                m_caret = caretIndexAt(m.x);
                m_cursorClock.restart();
            }
            return;
        }
    }

    // ── Keys / text — only when this box has focus ────────────────────────
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
                if (kp->control) {                  // Ctrl+Del: word forward
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
                return;                             // not ours — no blink reset
        }
        m_cursorClock.restart();                    // keep caret visible
        return;
    }

    if (const auto* te = event.getIf<sf::Event::TextEntered>()) {
        uint32_t u = te->unicode;
        if (u == 8) {                                   // Backspace
            if (m_caret > 0) { m_value.erase(m_caret - 1, 1); --m_caret; }
        } else if (u == 127) {                          // Ctrl+Backspace: word
            std::size_t from = prevWord(m_caret);
            m_value.erase(from, m_caret - from);
            m_caret = from;
        } else if (u >= 32 && u <= 126) {               // Printable ASCII
            insertText(std::string(1, static_cast<char>(u)));
        } else {
            return;
        }
        m_cursorClock.restart();
    }
}

void TextBox::setFocused(bool f) {
    if (f != m_focused) {           // only on a real change — this is called
        m_cursorClock.restart();    // every frame, and restarting each frame
        if (f) m_caret = std::min(m_caret, m_value.size());
    }                               // would freeze the blink
    m_focused = f;
}

std::string TextBox::getText() const { return m_value; }

void TextBox::setPlaceholder(const std::string& ph) { m_placeholder = ph; }
void TextBox::setPasswordMode(bool pm)               { m_passwordMode = pm; if (!pm) m_showText = false; }
void TextBox::clear()                                { m_value = ""; m_showText = false; m_caret = 0; }
void TextBox::setText(const std::string& t)          { m_value = t; m_caret = m_value.size(); }
void TextBox::setPosition(sf::Vector2f p)            { m_pos   = p; }
sf::FloatRect TextBox::getBounds() const             { return {m_pos, m_size}; }
