#include "AdminDashboardGUI.h"
#include "Button.h"
#include "ScreenManager.h"
#include "Theme.h"

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

using std::string;
using std::to_string;

namespace {

constexpr float SW = 210.f;
constexpr float HH = 60.f;

constexpr int UNI_FIELDS = 2, UNI_SAVE = 2, UNI_CANCEL = 3, UNI_COUNT = 4;
constexpr int BUS_FIELDS = 5, BUS_SAVE = 5, BUS_CANCEL = 6, BUS_COUNT = 7;

constexpr float CARD_H   = 104.f;
constexpr float UCARD_H  = 76.f;
constexpr float CARD_GAP = 12.f;

constexpr float FIELD_H  = 58.f;

constexpr float TOAST_HOLD = 3.f;

struct NavItem { const char* label; int state; float y; };

const NavItem NAV[] = {
    {"Dashboard",    0 ,         82.f},
    {"Universities", 1 , 132.f},
    {"Buses",        3 ,        182.f},
};

}

AdminDashboardScreen::AdminDashboardScreen(sf::Font& font)
    : m_font(font),
      m_uniCodeBox (font, {100.f, 46.f}, {0.f, 0.f}),
      m_uniNameBox (font, {100.f, 46.f}, {0.f, 0.f}),
      m_busIdBox   (font, {100.f, 46.f}, {0.f, 0.f}),
      m_busNameBox (font, {100.f, 46.f}, {0.f, 0.f}),
      m_busUniBox  (font, {100.f, 46.f}, {0.f, 0.f}),
      m_busSeatsBox(font, {100.f, 46.f}, {0.f, 0.f}),
      m_busRouteBox(font, {100.f, 46.f}, {0.f, 0.f})
{
    m_uniCodeBox.setPlaceholder("e.g.  BUET");
    m_uniNameBox.setPlaceholder("e.g.  Bangladesh University of Engineering");

    m_busIdBox.setPlaceholder   ("e.g.  BUS-001");
    m_busNameBox.setPlaceholder ("e.g.  City Express");
    m_busUniBox.setPlaceholder  ("University code");
    m_busSeatsBox.setPlaceholder("e.g.  40");
    m_busRouteBox.setPlaceholder("Stop1, Stop2, Stop3...");
}

void AdminDashboardScreen::onEnter()
{

    m_unis  = m_admin.getUniversities();
    m_buses = m_admin.getBuses();
}

void AdminDashboardScreen::setInfo(const string& msg, bool err)
{
    m_infoText = msg;
    m_infoErr  = err;
    m_showInfo = true;
    m_infoAge  = 0.f;
}

void AdminDashboardScreen::goTo(State next)
{
    m_state     = next;
    m_selIdx    = -1;
    m_scrollOff = 0.f;

    if (next == VIEW_UNIVERSITIES) m_unis  = m_admin.getUniversities();
    if (next == VIEW_BUSES)        m_buses = m_admin.getBuses();
}

void AdminDashboardScreen::saveUniversity()
{
    string err;
    if (m_admin.addUniversity(m_uniCodeBox.getText(), m_uniNameBox.getText(), err))
    {
        setInfo("University added.", false);
        m_state  = VIEW_UNIVERSITIES;
        m_unis   = m_admin.getUniversities();
        m_selIdx = -1;
        m_uniCodeBox.clear();
        m_uniNameBox.clear();
        m_focusField = 0;
    }
    else
    {
        setInfo("Error: " + err, true);
    }
}

void AdminDashboardScreen::cancelUniversity()
{
    m_state = VIEW_UNIVERSITIES;
    m_unis  = m_admin.getUniversities();
    m_uniCodeBox.clear();
    m_uniNameBox.clear();
    m_focusField = 0;
}

void AdminDashboardScreen::saveBus()
{
    string err;
    int seats = 0;
    try { seats = std::stoi(m_busSeatsBox.getText()); } catch (...) { seats = -1; }

    if (m_admin.addBus(m_busIdBox.getText(), m_busNameBox.getText(),
                       m_busUniBox.getText(), seats,
                       m_busRouteBox.getText(), err))
    {
        setInfo("Bus added.", false);
        m_state  = VIEW_BUSES;
        m_buses  = m_admin.getBuses();
        m_selIdx = -1;
        m_busIdBox.clear();
        m_busNameBox.clear();
        m_busUniBox.clear();
        m_busSeatsBox.clear();
        m_busRouteBox.clear();
        m_focusField = 0;
    }
    else
    {
        setInfo("Error: " + err, true);
    }
}

void AdminDashboardScreen::cancelBus()
{
    m_state = VIEW_BUSES;
    m_buses = m_admin.getBuses();
    m_busIdBox.clear();
    m_busNameBox.clear();
    m_busUniBox.clear();
    m_busSeatsBox.clear();
    m_busRouteBox.clear();
    m_focusField = 0;
}

void AdminDashboardScreen::prepare(sf::Vector2f size, sf::Vector2f mouse)
{
    Screen::prepare(size, mouse);

    m_contentW = size.x - SW;
    m_contentH = size.y - HH;

    m_fX  = SW + 32.f;
    m_fW  = m_contentW - 64.f;
    m_fW2 = (m_fW - 24.f) * 0.5f;
    m_c1X = m_fX;
    m_c2X = m_fX + m_fW2 + 24.f;

    m_uniCodeBox.setPosition({m_fX, HH + 110.f});
    m_uniCodeBox.setSize    ({m_fW, FIELD_H});
    m_uniNameBox.setPosition({m_fX, HH + 214.f});
    m_uniNameBox.setSize    ({m_fW, FIELD_H});

    m_busIdBox.setPosition   ({m_c1X, HH + 110.f});
    m_busIdBox.setSize       ({m_fW2, FIELD_H});
    m_busNameBox.setPosition ({m_c1X, HH + 214.f});
    m_busNameBox.setSize     ({m_fW2, FIELD_H});
    m_busUniBox.setPosition  ({m_c1X, HH + 318.f});
    m_busUniBox.setSize      ({m_fW2, FIELD_H});
    m_busSeatsBox.setPosition({m_c2X, HH + 214.f});
    m_busSeatsBox.setSize    ({m_fW2, FIELD_H});
    // Route: full-width row below the two-column fields
    m_busRouteBox.setPosition({m_fX,  HH + 422.f});
    m_busRouteBox.setSize    ({m_fW,  FIELD_H});

    m_uniCodeBox.setFocused(m_state == ADD_UNIVERSITY && m_focusField == 0);
    m_uniNameBox.setFocused(m_state == ADD_UNIVERSITY && m_focusField == 1);

    m_busIdBox.setFocused   (m_state == ADD_BUS && m_focusField == 0);
    m_busNameBox.setFocused (m_state == ADD_BUS && m_focusField == 1);
    m_busUniBox.setFocused  (m_state == ADD_BUS && m_focusField == 2);
    m_busSeatsBox.setFocused(m_state == ADD_BUS && m_focusField == 3);
    m_busRouteBox.setFocused(m_state == ADD_BUS && m_focusField == 4);
}

void AdminDashboardScreen::handleEvent(const sf::Event& event)
{
    const float ww = m_size.x, wh = m_size.y;
    const float mx = m_mouse.x, my = m_mouse.y;
    const bool  inContent = mx >= SW && my >= HH;

    const float CARD_X   = SW + 20.f;
    const float CARD_W   = m_contentW - 40.f;
    const float LIST_TOP = HH + 20.f;

    const sf::FloatRect addRect{{ww - 240.f, (HH - 36.f) * 0.5f}, {110.f, 36.f}};
    const sf::FloatRect delRect{{ww - 120.f, (HH - 36.f) * 0.5f}, {110.f, 36.f}};

    const sf::FloatRect saveUniRect  {{m_fX,         HH + 320.f}, {160.f, 44.f}};
    const sf::FloatRect cancelUniRect{{m_fX + 170.f, HH + 320.f}, {110.f, 44.f}};
    const sf::FloatRect saveBusRect  {{m_fX,         HH + 510.f}, {160.f, 44.f}};
    const sf::FloatRect cancelBusRect{{m_fX + 170.f, HH + 510.f}, {110.f, 44.f}};

    if (event.is<sf::Event::MouseButtonPressed>())
    {

        if (mx < SW)
        {
            for (const auto& nav : NAV)
            {
                if (my >= nav.y && my < nav.y + 42.f)
                    goTo(static_cast<State>(nav.state));
            }

            if (my >= wh - 52.f && my < wh - 12.f)
            {
                m_app->pop();
                return;
            }
        }

        if (my < HH && mx >= SW)
        {
            if ((m_state == VIEW_UNIVERSITIES || m_state == ADD_UNIVERSITY) &&
                addRect.contains(m_mouse))
            {
                m_state = ADD_UNIVERSITY;
                m_focusField = 0;
                m_selIdx = -1;
            }
            if ((m_state == VIEW_BUSES || m_state == ADD_BUS) &&
                addRect.contains(m_mouse))
            {
                m_state = ADD_BUS;
                m_focusField = 0;
                m_selIdx = -1;
            }
            if (m_state == VIEW_UNIVERSITIES && delRect.contains(m_mouse) &&
                m_selIdx >= 0 && m_selIdx < static_cast<int>(m_unis.size()))
            {
                string err;
                if (m_admin.deleteUniversity(m_unis[m_selIdx].first, err))
                {
                    m_unis   = m_admin.getUniversities();
                    m_selIdx = -1;
                    setInfo("University deleted.", false);
                }
                else setInfo("Error: " + err, true);
            }
            if (m_state == VIEW_BUSES && delRect.contains(m_mouse) &&
                m_selIdx >= 0 && m_selIdx < static_cast<int>(m_buses.size()))
            {
                string err;
                if (m_admin.deleteBus(m_buses[m_selIdx].getBusID(), err))
                {
                    m_buses  = m_admin.getBuses();
                    m_selIdx = -1;
                    setInfo("Bus deleted.", false);
                }
                else setInfo("Error: " + err, true);
            }
        }

        if (inContent)
        {
            if (m_state == VIEW_UNIVERSITIES)
            {
                for (int i = 0; i < static_cast<int>(m_unis.size()); ++i)
                {
                    float cy = LIST_TOP + i * (UCARD_H + CARD_GAP) - m_scrollOff;
                    if (my >= cy && my < cy + UCARD_H) { m_selIdx = (m_selIdx == i) ? -1 : i; break; }
                }
            }
            if (m_state == VIEW_BUSES)
            {
                for (int i = 0; i < static_cast<int>(m_buses.size()); ++i)
                {
                    float cy = LIST_TOP + i * (CARD_H + CARD_GAP) - m_scrollOff;
                    if (my >= cy && my < cy + CARD_H) { m_selIdx = (m_selIdx == i) ? -1 : i; break; }
                }
            }

            if (m_state == ADD_UNIVERSITY)
            {
                if (m_uniCodeBox.getBounds().contains(m_mouse)) m_focusField = 0;
                if (m_uniNameBox.getBounds().contains(m_mouse)) m_focusField = 1;
                if (saveUniRect.contains(m_mouse))   { saveUniversity();   return; }
                if (cancelUniRect.contains(m_mouse)) { cancelUniversity(); return; }
            }
            if (m_state == ADD_BUS)
            {
                if (m_busIdBox.getBounds().contains(m_mouse))    m_focusField = 0;
                if (m_busNameBox.getBounds().contains(m_mouse))  m_focusField = 1;
                if (m_busUniBox.getBounds().contains(m_mouse))   m_focusField = 2;
                if (m_busSeatsBox.getBounds().contains(m_mouse)) m_focusField = 3;
                if (m_busRouteBox.getBounds().contains(m_mouse)) m_focusField = 4;
                if (saveBusRect.contains(m_mouse))   { saveBus();   return; }
                if (cancelBusRect.contains(m_mouse)) { cancelBus(); return; }
            }
        }
    }

    if (const auto* mw = event.getIf<sf::Event::MouseWheelScrolled>())
    {
        if (inContent && (m_state == VIEW_UNIVERSITIES || m_state == VIEW_BUSES))
        {
            m_scrollOff -= mw->delta * 36.f;
            m_scrollOff  = std::clamp(m_scrollOff, 0.f, m_maxScroll);
        }
    }

    if (const auto* kp = event.getIf<sf::Event::KeyPressed>())
    {
        using Key = sf::Keyboard::Key;

        if (m_state == ADD_UNIVERSITY || m_state == ADD_BUS)
        {
            const bool uni    = (m_state == ADD_UNIVERSITY);
            const int  fields = uni ? UNI_FIELDS : BUS_FIELDS;
            const int  count  = uni ? UNI_COUNT  : BUS_COUNT;
            const int  saveAt = uni ? UNI_SAVE   : BUS_SAVE;
            const int  cancAt = uni ? UNI_CANCEL : BUS_CANCEL;
            bool consumed = true;

            switch (kp->code)
            {
                case Key::Enter:

                    if      (m_focusField <  fields - 1) ++m_focusField;
                    else if (m_focusField == cancAt)     { if (uni) cancelUniversity(); else cancelBus(); }
                    else                                 { if (uni) saveUniversity();   else saveBus();   }
                    break;

                case Key::Tab:
                    m_focusField = kp->shift ? (m_focusField + count - 1) % count
                                             : (m_focusField + 1) % count;
                    break;

                case Key::Down:
                    m_focusField = (m_focusField + 1) % count;
                    break;

                case Key::Up:
                    m_focusField = (m_focusField + count - 1) % count;
                    break;

                case Key::Left:

                    if (m_focusField == cancAt) m_focusField = saveAt;
                    else                        consumed = false;
                    break;

                case Key::Right:
                    if (m_focusField == saveAt) m_focusField = cancAt;
                    else                        consumed = false;
                    break;

                case Key::Escape:
                    if (uni) cancelUniversity(); else cancelBus();
                    break;

                default:
                    consumed = false;
                    break;
            }

            if (consumed) return;
        }
        else
        {
            if (kp->code == Key::Enter)
            {

                if (m_state == VIEW_UNIVERSITIES) { m_state = ADD_UNIVERSITY; m_focusField = 0; m_selIdx = -1; }
                if (m_state == VIEW_BUSES)        { m_state = ADD_BUS;        m_focusField = 0; m_selIdx = -1; }
                return;
            }
            if (kp->code == Key::Escape)
            {

                if (m_state == DASHBOARD) m_app->pop();
                else                      goTo(DASHBOARD);
                return;
            }
        }
    }

    if (event.is<sf::Event::TextEntered>() ||
        event.is<sf::Event::KeyPressed>()  ||
        event.is<sf::Event::MouseButtonPressed>())
    {
        if (m_state == ADD_UNIVERSITY)
        {
            if (m_focusField == 0) m_uniCodeBox.handleEvent(event);
            else                   m_uniNameBox.handleEvent(event);
        }
        if (m_state == ADD_BUS)
        {
            switch (m_focusField)
            {
                case 0: m_busIdBox.handleEvent(event);    break;
                case 1: m_busNameBox.handleEvent(event);  break;
                case 2: m_busUniBox.handleEvent(event);   break;
                case 3: m_busSeatsBox.handleEvent(event); break;
                case 4: m_busRouteBox.handleEvent(event); break;
                default: break;
            }
        }
    }
}

void AdminDashboardScreen::update(float dt)
{
    if (m_showInfo)
    {
        m_infoAge += dt;
        if (m_infoAge > TOAST_HOLD) m_showInfo = false;
    }

    m_uniCodeBox.update(dt);
    m_uniNameBox.update(dt);
    m_busIdBox.update(dt);
    m_busNameBox.update(dt);
    m_busUniBox.update(dt);
    m_busSeatsBox.update(dt);
    m_busRouteBox.update(dt);

    const float ww = m_size.x;

    const bool addHot = sf::FloatRect{{ww - 240.f, (HH - 36.f) * 0.5f}, {110.f, 36.f}}.contains(m_mouse);
    const bool delHot = sf::FloatRect{{ww - 120.f, (HH - 36.f) * 0.5f}, {110.f, 36.f}}.contains(m_mouse);
    m_addHoverT = Theme::approachHover(m_addHoverT, addHot, dt);
    m_delHoverT = Theme::approachHover(m_delHoverT, delHot, dt);

    const float btnY = (m_state == ADD_BUS) ? HH + 510.f : HH + 320.f;
    const bool saveHot   = sf::FloatRect{{m_fX,         btnY}, {160.f, 44.f}}.contains(m_mouse);
    const bool cancelHot = sf::FloatRect{{m_fX + 170.f, btnY}, {110.f, 44.f}}.contains(m_mouse);
    m_saveHoverT   = Theme::approachHover(m_saveHoverT,   saveHot,   dt);
    m_cancelHoverT = Theme::approachHover(m_cancelHoverT, cancelHot, dt);
}

void AdminDashboardScreen::skipAnimations()
{
    m_uniCodeBox.settle();
    m_uniNameBox.settle();
    m_busIdBox.settle();
    m_busNameBox.settle();
    m_busUniBox.settle();
    m_busSeatsBox.settle();
    m_busRouteBox.settle();
}

void AdminDashboardScreen::drawSidebar(sf::RenderTarget& target)
{
    const float wh = m_size.y;
    const float mx = m_mouse.x, my = m_mouse.y;

    Theme::drawSidebarBackdrop(target, SW, wh, Theme::ACCENT);

    Theme::drawTextHCentered(target, m_font, "Bus Tracker", Theme::Type::SUBTITLE,
                             Theme::ACCENT_HOVER, SW * 0.5f, 14.f, sf::Text::Bold);
    Theme::drawTextHCentered(target, m_font, "ADMIN", Theme::Type::CAPTION,
                             Theme::TEXT_MUTED, SW * 0.5f, 38.f, sf::Text::Bold);

    Theme::drawSeparatorSoft(target, 10.f, 70.f, SW - 20.f);

    for (const auto& nav : NAV)
    {
        const State st = static_cast<State>(nav.state);
        bool active = (m_state == st) ||
                      (st == VIEW_UNIVERSITIES && m_state == ADD_UNIVERSITY) ||
                      (st == VIEW_BUSES        && m_state == ADD_BUS);
        bool hov    = mx < SW && my >= nav.y && my < nav.y + 42.f;

        Theme::drawNavItem(target, SW, nav.y, 42.f, active, hov, Theme::ACCENT);
        Theme::drawTextVCentered(target, m_font, nav.label, Theme::Type::META,
                                 active ? Theme::TEXT_PRIMARY : Theme::TEXT_SECONDARY,
                                 22.f, nav.y, 42.f,
                                 active ? sf::Text::Bold : sf::Text::Regular);
    }

    bool logHov = mx < SW && my >= wh - 52.f && my < wh - 12.f;
    if (logHov)
        Theme::fillRoundedRect(target, {8.f, wh - 52.f}, {SW - 18.f, 40.f}, 10.f,
                               Theme::withAlpha(Theme::DANGER, 45));
    Theme::drawTextVCentered(target, m_font, "Logout", Theme::Type::META,
                             Theme::DANGER_HOVER, 22.f, wh - 52.f, 40.f, sf::Text::Bold);
    Theme::drawSeparatorSoft(target, 10.f, wh - 56.f, SW - 20.f);
}

void AdminDashboardScreen::drawHeader(sf::RenderTarget& target)
{
    const float ww = m_size.x;

    Theme::drawHeaderBar(target, SW, ww - SW, HH, Theme::ACCENT, Theme::PURPLE);

    string pageTitle = "Dashboard";
    if (m_state == VIEW_UNIVERSITIES || m_state == ADD_UNIVERSITY) pageTitle = "Universities";
    else if (m_state == VIEW_BUSES   || m_state == ADD_BUS)        pageTitle = "Buses";

    Theme::drawTextVCentered(target, m_font, pageTitle, Theme::Type::HEADING,
                             Theme::TEXT_PRIMARY, SW + 24.f, 0.f, HH, sf::Text::Bold);

    if (m_state == VIEW_UNIVERSITIES || m_state == VIEW_BUSES ||
        m_state == ADD_UNIVERSITY    || m_state == ADD_BUS)
    {
        Button addBtn(m_font, "+ Add", {110.f, 36.f}, {ww - 240.f, (HH - 36.f) * 0.5f});
        addBtn.setHoverT(m_addHoverT);
        addBtn.draw(target);

        if (m_state == VIEW_UNIVERSITIES || m_state == VIEW_BUSES)
        {
            Button delBtn(m_font, "Delete", {110.f, 36.f}, {ww - 120.f, (HH - 36.f) * 0.5f},
                          m_selIdx >= 0 ? ButtonStyle::DANGER : ButtonStyle::SECONDARY);
            delBtn.setHoverT(m_delHoverT);
            delBtn.draw(target);
        }
    }
}

void AdminDashboardScreen::draw(sf::RenderTarget& target)
{
    const float ww = m_size.x, wh = m_size.y;
    const float CW = m_contentW, CH = m_contentH;
    const float mx = m_mouse.x, my = m_mouse.y;
    const bool  inContent = mx >= SW && my >= HH;

    const float CARD_X   = SW + 20.f;
    const float CARD_W   = CW - 40.f;
    const float LIST_TOP = HH + 20.f;

    Theme::drawAppBase(target, {ww, wh});
    if (!Theme::backgroundReady())
    {
        Theme::drawRadialGlow(target, {ww * 0.78f, HH}, std::max(ww, wh) * 0.55f, Theme::ACCENT, 18);
        Theme::drawRadialGlow(target, {ww * 0.35f, wh}, std::max(ww, wh) * 0.45f, Theme::PURPLE, 16);
    }

    if (m_state == DASHBOARD)
    {
        auto allUnis  = m_admin.getUniversities();
        auto allBuses = m_admin.getBuses();

        float scy  = HH + 30.f;
        float scW  = (CW - 60.f) * 0.5f;
        float sc1X = SW + 20.f, sc2X = SW + 30.f + scW;

        auto statCard = [&](float x, sf::Color accent, const string& value, const string& label) {
            Theme::drawCard(target, {x, scy}, {scW, 116.f}, Theme::BG_CARD, 12.f);
            Theme::drawAccentBar(target, x, scy, 116.f, accent, 5.f);
            Theme::drawText(target, m_font, value, 46, accent,
                            {x + 24.f, scy + 12.f}, sf::Text::Bold);
            Theme::drawText(target, m_font, label, Theme::Type::LABEL,
                            Theme::TEXT_MUTED, {x + 24.f, scy + 84.f}, sf::Text::Bold);
        };
        statCard(sc1X, Theme::ACCENT, to_string(allUnis.size()),  "UNIVERSITIES");
        statCard(sc2X, Theme::PURPLE, to_string(allBuses.size()), "BUSES");

        Theme::drawText(target, m_font,
                        "Use the sidebar to manage Universities and Buses.",
                        Theme::Type::META, Theme::TEXT_SECONDARY,
                        {SW + 22.f, scy + 148.f});
    }
    else if (m_state == VIEW_UNIVERSITIES)
    {
        float totalH = m_unis.size() * (UCARD_H + CARD_GAP);
        m_maxScroll  = std::max(0.f, totalH - CH + 40.f);
        m_scrollOff  = std::clamp(m_scrollOff, 0.f, m_maxScroll);

        const float badgeH = Theme::badgeHeight(m_font, Theme::Type::BADGE_UNI);

        for (int i = 0; i < static_cast<int>(m_unis.size()); ++i)
        {
            float cy = LIST_TOP + i * (UCARD_H + CARD_GAP) - m_scrollOff;
            if (cy + UCARD_H < HH || cy > wh) continue;

            bool sel = (i == m_selIdx);
            bool hov = inContent && mx >= CARD_X && mx < CARD_X + CARD_W &&
                       my >= cy && my < cy + UCARD_H;
            sf::Color bg = sel ? Theme::ITEM_SELECTED : hov ? Theme::ITEM_HOVER : Theme::ITEM_BG;

            Theme::drawCard(target, {CARD_X, cy}, {CARD_W, UCARD_H}, bg, 8.f);
            Theme::drawAccentBar(target, CARD_X, cy, UCARD_H,
                                 sel ? Theme::ACCENT_HOVER : Theme::ACCENT);

            float badgeW = Theme::drawBadge(
                target, m_font, m_unis[i].first,
                {CARD_X + 16.f, std::round(cy + (UCARD_H - badgeH) * 0.5f)},
                Theme::BADGE_UNI_BG, Theme::BADGE_UNI_TEXT,
                Theme::Type::BADGE_UNI, Theme::BADGE_UNI_EDGE);

            Theme::drawTextVCentered(target, m_font, m_unis[i].second,
                                     Theme::Type::SUBTITLE, Theme::TEXT_PRIMARY,
                                     CARD_X + 16.f + badgeW + 16.f, cy, UCARD_H,
                                     sf::Text::Bold);
        }
        if (m_unis.empty())
            Theme::drawCenteredText(target, m_font, "No universities registered.",
                                    Theme::Type::BODY, Theme::TEXT_MUTED,
                                    {{SW + 20.f, HH + 80.f}, {CW - 40.f, 60.f}});
    }
    else if (m_state == ADD_UNIVERSITY)
    {
        Theme::drawCardElevated(target, {SW + 20.f, HH + 20.f}, {CW - 40.f, 380.f},
                                Theme::BG_CARD, 14.f, 20.f, 14);
        Theme::fillRoundedRectV(target, {SW + 38.f, HH + 21.f}, {CW - 76.f, 3.f}, 1.5f,
                                Theme::ACCENT_CYAN, Theme::ACCENT);

        auto drawLbl = [&](const string& s, float x, float y) {
            Theme::drawText(target, m_font, s, Theme::Type::LABEL,
                            Theme::TEXT_MUTED, {x, y}, sf::Text::Bold);
        };
        drawLbl("UNIVERSITY CODE", m_fX, HH + 94.f);
        drawLbl("UNIVERSITY NAME", m_fX, HH + 198.f);

        m_uniCodeBox.draw(target);
        m_uniNameBox.draw(target);

        Button saveBtn  (m_font, "Save",   {160.f, 44.f}, {m_fX,         HH + 320.f});
        Button cancelBtn(m_font, "Cancel", {110.f, 44.f}, {m_fX + 170.f, HH + 320.f},
                         ButtonStyle::SECONDARY);
        saveBtn.setFocused  (m_focusField == UNI_SAVE);
        cancelBtn.setFocused(m_focusField == UNI_CANCEL);
        saveBtn.setHoverT(m_saveHoverT);
        cancelBtn.setHoverT(m_cancelHoverT);
        saveBtn.draw(target);
        cancelBtn.draw(target);
    }
    else if (m_state == VIEW_BUSES)
    {
        float totalH = m_buses.size() * (CARD_H + CARD_GAP);
        m_maxScroll  = std::max(0.f, totalH - CH + 40.f);
        m_scrollOff  = std::clamp(m_scrollOff, 0.f, m_maxScroll);

        const float badgeH = Theme::badgeHeight(m_font, Theme::Type::BADGE_BUS);

        for (int i = 0; i < static_cast<int>(m_buses.size()); ++i)
        {
            float cy = LIST_TOP + i * (CARD_H + CARD_GAP) - m_scrollOff;
            if (cy + CARD_H < HH || cy > wh) continue;

            bool sel = (i == m_selIdx);
            bool hov = inContent && mx >= CARD_X && mx < CARD_X + CARD_W &&
                       my >= cy && my < cy + CARD_H;
            sf::Color bg = sel ? Theme::ITEM_SELECTED : hov ? Theme::ITEM_HOVER : Theme::ITEM_BG;

            Theme::drawCard(target, {CARD_X, cy}, {CARD_W, CARD_H}, bg, 8.f);
            Theme::drawAccentBar(target, CARD_X, cy, CARD_H,
                                 sel ? Theme::ACCENT_HOVER : Theme::PURPLE);

            const float padL = 16.f;
            const float rowY = cy + 14.f;

            float badgeW = Theme::drawBadge(target, m_font, m_buses[i].getBusID(),
                                            {CARD_X + padL, rowY},
                                            Theme::BADGE_BUS_BG, Theme::BADGE_BUS_TEXT,
                                            Theme::Type::BADGE_BUS, Theme::BADGE_BUS_EDGE);
            Theme::drawTextVCentered(target, m_font, m_buses[i].getBusName(),
                                     Theme::Type::BUS_NAME, Theme::TEXT_PRIMARY,
                                     CARD_X + padL + badgeW + 16.f, rowY, badgeH,
                                     sf::Text::Bold);

            string secondary = m_buses[i].getUniversityCode()
                             + "  ·  " + to_string(m_buses[i].getTotalSeats()) + " seats";
            const float metaY = rowY + badgeH + 12.f;
            Theme::drawText(target, m_font, secondary, Theme::Type::META,
                            Theme::TEXT_SECONDARY, {CARD_X + padL, metaY});

            string route = Theme::ellipsize(m_font, m_buses[i].getRoute(),
                                            Theme::Type::ROUTE, CARD_W - padL - 24.f);
            Theme::drawText(target, m_font, route, Theme::Type::ROUTE,
                            Theme::TEXT_ROUTE, {CARD_X + padL, metaY + 24.f});
        }
        if (m_buses.empty())
            Theme::drawCenteredText(target, m_font, "No buses registered.",
                                    Theme::Type::BODY, Theme::TEXT_MUTED,
                                    {{SW + 20.f, HH + 80.f}, {CW - 40.f, 60.f}});
    }
    else if (m_state == ADD_BUS)
    {
        Theme::drawCardElevated(target, {SW + 20.f, HH + 20.f}, {CW - 40.f, 580.f},
                                Theme::BG_CARD, 14.f, 20.f, 14);
        Theme::fillRoundedRectV(target, {SW + 38.f, HH + 21.f}, {CW - 76.f, 3.f}, 1.5f,
                                Theme::PURPLE_HOVER, Theme::PURPLE);

        auto drawLbl = [&](const string& s, float x, float y) {
            Theme::drawText(target, m_font, s, Theme::Type::LABEL,
                            Theme::TEXT_MUTED, {x, y}, sf::Text::Bold);
        };
        drawLbl("BUS ID",                     m_c1X, HH + 94.f);
        drawLbl("BUS NAME",                   m_c1X, HH + 198.f);
        drawLbl("UNIVERSITY CODE",            m_c1X, HH + 302.f);
        drawLbl("TOTAL SEATS",                m_c2X, HH + 198.f);
        drawLbl("ROUTE STOPS (comma-sep)",    m_fX,  HH + 406.f);

        m_busIdBox.draw(target);
        m_busNameBox.draw(target);
        m_busUniBox.draw(target);
        m_busSeatsBox.draw(target);
        m_busRouteBox.draw(target);

        Button saveBusBtn  (m_font, "Save Bus", {160.f, 44.f}, {m_fX,         HH + 510.f});
        Button cancelBusBtn(m_font, "Cancel",   {110.f, 44.f}, {m_fX + 170.f, HH + 510.f},
                            ButtonStyle::SECONDARY);
        saveBusBtn.setFocused  (m_focusField == BUS_SAVE);
        cancelBusBtn.setFocused(m_focusField == BUS_CANCEL);
        saveBusBtn.setHoverT(m_saveHoverT);
        cancelBusBtn.setHoverT(m_cancelHoverT);
        saveBusBtn.draw(target);
        cancelBusBtn.draw(target);
    }

    drawSidebar(target);
    drawHeader(target);

    if (m_showInfo)
    {
        float alpha = m_infoAge > 2.f ? 1.f - (m_infoAge - 2.f) : 1.f;
        Theme::drawInfoToast(target, m_font, m_infoText, m_infoErr, ww, wh,
                             std::clamp(alpha, 0.f, 1.f));
    }
}
