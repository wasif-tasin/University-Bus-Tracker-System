#include "UserDashboardGUI.h"
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
using std::vector;

namespace {

constexpr float SW = 210.f;
constexpr float HH = 60.f;

constexpr float CARD_H   = 108.f;
constexpr float CARD_GAP = 12.f;

constexpr float TOAST_HOLD = 3.f;

constexpr float SECTION_SECONDS = 0.42f;
constexpr float ROW_STAGGER     = 0.045f;
constexpr float RISE_PX         = 18.f;

struct NavItem { const char* label; int state; float y; };

const NavItem NAV[] = {
    {"Universities",   1,  82.f},
    {"All Buses",      3, 132.f},
    {"By University",  2, 182.f},
    {"Search Bus ID",  4, 232.f},
    {"Search by Stop", 5, 282.f},
};

constexpr int NAV_COUNT = static_cast<int>(sizeof(NAV) / sizeof(NAV[0]));

string wrapRoute(const string& route, float maxW, const sf::Font& font,
                 unsigned sz, const string& prefix = "Route: ")
{
    string wrapped = prefix;
    string indent(prefix.length(), ' ');
    string cur;
    sf::Text tmp(font);
    tmp.setCharacterSize(sz);

    vector<string> stops;
    string s;
    for (char c : route)
    {
        if (c == ',') { if (!s.empty()) { stops.push_back(s); s.clear(); } }
        else s += c;
    }
    if (!s.empty()) stops.push_back(s);

    for (size_t i = 0; i < stops.size(); ++i)
    {
        string st = stops[i];
        while (!st.empty() && st.front() == ' ') st = st.substr(1);
        while (!st.empty() && st.back()  == ' ') st.pop_back();

        string candidate = st + (i < stops.size() - 1 ? ", " : "");
        tmp.setString(cur + candidate);
        if (tmp.getLocalBounds().size.x > maxW && !cur.empty())
        {
            wrapped += cur + "\n" + indent;
            cur = candidate;
        }
        else
        {
            cur += candidate;
        }
    }
    wrapped += cur;
    return wrapped;
}

}

UserDashboardScreen::UserDashboardScreen(sf::Font& font)
    : m_font(font),
      m_uniCodeBox(font, {400.f, 44.f}, {0.f, 0.f}),
      m_busIdBox  (font, {400.f, 44.f}, {0.f, 0.f}),
      m_stopBox   (font, {400.f, 44.f}, {0.f, 0.f})
{
    m_uniCodeBox.setPlaceholder("Enter university code (e.g. BUET)");
    m_busIdBox.setPlaceholder  ("Enter Bus ID");
    m_stopBox.setPlaceholder   ("Enter stop name");
}

void UserDashboardScreen::onEnter()
{

    if (m_state == USER_VIEW_UNIVERSITIES) m_unis  = m_user.getUniversities();
    if (m_state == USER_VIEW_BUSES)        m_buses = m_user.getBuses();
}

void UserDashboardScreen::setInfo(const string& msg, bool err)
{
    m_infoText = msg;
    m_infoErr  = err;
    m_showInfo = true;
    m_infoAge  = 0.f;
}

void UserDashboardScreen::goTo(State next)
{
    m_state     = next;
    m_scrollOff = 0.f;
    m_buses.clear();

    if (next == USER_VIEW_UNIVERSITIES) m_unis  = m_user.getUniversities();
    if (next == USER_VIEW_BUSES)        m_buses = m_user.getBuses();
}

void UserDashboardScreen::runSearch()
{
    m_scrollOff = 0.f;
    m_resultsT  = 0.f;  

    if (m_state == USER_SELECT_UNIVERSITY)
    {
        m_buses = m_user.getBusesForUniversity(m_uniCodeBox.getText());
        if (m_buses.empty())
            setInfo("No buses found for: " + m_uniCodeBox.getText(), true);
    }
    else if (m_state == USER_SEARCH_BUS)
    {
        m_buses = m_user.searchBus(m_busIdBox.getText());
        if (m_buses.empty())
            setInfo("Bus not found: " + m_busIdBox.getText(), true);
    }
    else if (m_state == USER_SEARCH_BY_STOP)
    {
        m_buses = m_user.searchByStop(m_stopBox.getText());
        if (m_buses.empty())
            setInfo("No buses pass through: " + m_stopBox.getText(), true);
    }
}

void UserDashboardScreen::prepare(sf::Vector2f size, sf::Vector2f mouse)
{
    Screen::prepare(size, mouse);

    m_contentW = size.x - SW;
    m_contentH = size.y - HH;

    m_fX       = SW + 32.f;
    const float fW = m_contentW - 64.f;
    m_srchW    = fW * 0.55f;
    m_srchBtnX = m_fX + m_srchW + 12.f;

    const sf::Vector2f fieldSize{m_srchW, 44.f};
    const float fdy = (1.f - Theme::smoothstep01(m_stateT)) * RISE_PX;
    const sf::Vector2f fieldPos {m_fX, HH + 28.f + fdy};

    m_uniCodeBox.setSize(fieldSize);
    m_busIdBox.setSize(fieldSize);
    m_stopBox.setSize(fieldSize);

    m_uniCodeBox.setPosition(fieldPos);
    m_busIdBox.setPosition(fieldPos);
    m_stopBox.setPosition(fieldPos);

    m_uniCodeBox.setFocused(m_state == USER_SELECT_UNIVERSITY);
    m_busIdBox.setFocused  (m_state == USER_SEARCH_BUS);
    m_stopBox.setFocused   (m_state == USER_SEARCH_BY_STOP);
}

void UserDashboardScreen::handleEvent(const sf::Event& event)
{
    const float wh = m_size.y;
    const float mx = m_mouse.x, my = m_mouse.y;
    const bool  inContent = mx >= SW && my >= HH;

    const sf::FloatRect searchRect{{m_srchBtnX, HH + 28.f}, {110.f, 44.f}};

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

        if (inContent && searchRect.contains(m_mouse) &&
            (m_state == USER_SELECT_UNIVERSITY ||
             m_state == USER_SEARCH_BUS        ||
             m_state == USER_SEARCH_BY_STOP))
        {
            runSearch();
        }
    }

    if (const auto* mw = event.getIf<sf::Event::MouseWheelScrolled>())
    {
        if (inContent)
        {
            m_scrollOff -= mw->delta * 36.f;
            m_scrollOff  = std::clamp(m_scrollOff, 0.f, m_maxScroll);
        }
    }

    if (const auto* kp = event.getIf<sf::Event::KeyPressed>())
    {
        using Key = sf::Keyboard::Key;

        const bool searching = (m_state == USER_SELECT_UNIVERSITY ||
                                m_state == USER_SEARCH_BUS        ||
                                m_state == USER_SEARCH_BY_STOP);

        if (kp->code == Key::Enter)
        {
            if (searching) runSearch();
            return;
        }
        if (kp->code == Key::Escape)
        {

            if (m_state == USER_DASHBOARD) m_app->pop();
            else                           goTo(USER_DASHBOARD);
            return;
        }
    }

    if (event.is<sf::Event::TextEntered>() ||
        event.is<sf::Event::KeyPressed>()  ||
        event.is<sf::Event::MouseButtonPressed>())
    {
        if (m_state == USER_SELECT_UNIVERSITY) m_uniCodeBox.handleEvent(event);
        if (m_state == USER_SEARCH_BUS)        m_busIdBox.handleEvent(event);
        if (m_state == USER_SEARCH_BY_STOP)    m_stopBox.handleEvent(event);
    }
}

void UserDashboardScreen::update(float dt)
{
    if (m_state != m_shownState)
    {
        m_shownState = m_state;
        m_stateT     = 0.f;
        m_resultsT   = 0.f;
    }
    else if (m_stateT < 1.f)
    {
        m_stateT = std::min(1.f, m_stateT + dt / SECTION_SECONDS);
    }

    if (m_resultsT < 1.f)
        m_resultsT = std::min(1.f, m_resultsT + dt / SECTION_SECONDS);

    if (m_showInfo)
    {
        m_infoAge += dt;
        if (m_infoAge > TOAST_HOLD) m_showInfo = false;
    }

    m_uniCodeBox.update(dt);
    m_busIdBox.update(dt);
    m_stopBox.update(dt);

    const bool searchHot =
        sf::FloatRect{{m_srchBtnX, HH + 28.f}, {110.f, 44.f}}.contains(m_mouse);
    m_searchHoverT = Theme::approachHover(m_searchHoverT, searchHot, dt);

    // Sidebar hover easing.
    for (int i = 0; i < NAV_COUNT; ++i)
    {
        const bool hot = m_mouse.x < SW && m_mouse.y >= NAV[i].y &&
                         m_mouse.y < NAV[i].y + 42.f;
        m_navT[i] = Theme::approachHover(m_navT[i], hot, dt);
    }
    const bool logHot = m_mouse.x < SW && m_mouse.y >= m_size.y - 52.f &&
                        m_mouse.y < m_size.y - 12.f;
    m_logoutT = Theme::approachHover(m_logoutT, logHot, dt);
}

void UserDashboardScreen::skipAnimations()
{
    m_stateT   = 1.f;
    m_resultsT = 1.f;
    m_uniCodeBox.settle();
    m_busIdBox.settle();
    m_stopBox.settle();
}

void UserDashboardScreen::drawBusCard(sf::RenderTarget& target, const Bus& b,
                                      float cx, float cy, float cardW,
                                      bool hov, bool includeUni)
{
    sf::Color bg = hov ? Theme::ITEM_HOVER : Theme::ITEM_BG;
    Theme::drawCard(target, {cx, cy}, {cardW, CARD_H}, bg, 8.f);
    Theme::drawAccentBar(target, cx, cy, CARD_H, Theme::PURPLE);

    const float padL   = 16.f;
    const float badgeH = Theme::badgeHeight(m_font, Theme::Type::BADGE_BUS);
    const float rowY   = cy + 14.f;

    float badgeW = Theme::drawBadge(target, m_font, b.getBusID(),
                                    {cx + padL, rowY},
                                    Theme::BADGE_BUS_BG, Theme::BADGE_BUS_TEXT,
                                    Theme::Type::BADGE_BUS, Theme::BADGE_BUS_EDGE);

    Theme::drawTextVCentered(target, m_font, b.getBusName(),
                             Theme::Type::BUS_NAME, Theme::TEXT_PRIMARY,
                             cx + padL + badgeW + 16.f, rowY, badgeH, sf::Text::Bold);

    string secondary = includeUni ? b.getUniversityCode() + "  ·  " : "";
    secondary += to_string(b.getTotalSeats()) + " seats";
    const float metaY = rowY + badgeH + 12.f;
    Theme::drawText(target, m_font, secondary, Theme::Type::META,
                    Theme::TEXT_SECONDARY, {cx + padL, metaY});

    string route = Theme::ellipsize(m_font, b.getRoute(), Theme::Type::ROUTE,
                                    cardW - padL - 24.f);
    Theme::drawText(target, m_font, route, Theme::Type::ROUTE,
                    Theme::TEXT_ROUTE, {cx + padL, metaY + 24.f});
}

void UserDashboardScreen::drawSidebar(sf::RenderTarget& target)
{
    const float wh = m_size.y;

    Theme::drawSidebarBackdrop(target, SW, wh, Theme::ACCENT);

    Theme::drawTextHCentered(target, m_font, "Bus Tracker", Theme::Type::SUBTITLE,
                             Theme::ACCENT_HOVER, SW * 0.5f, 14.f, sf::Text::Bold);
    Theme::drawTextHCentered(target, m_font, "USER", Theme::Type::CAPTION,
                             Theme::TEXT_MUTED, SW * 0.5f, 38.f, sf::Text::Bold);

    Theme::drawSeparatorSoft(target, 10.f, 70.f, SW - 20.f);

    for (int i = 0; i < NAV_COUNT; ++i)
    {
        const NavItem& nav = NAV[i];
        bool active = (m_state == static_cast<State>(nav.state));

        Theme::drawNavItem(target, SW, nav.y, 42.f, active, m_navT[i], Theme::ACCENT);

        const float tx = 22.f + (active ? 0.f : 3.f * m_navT[i]);
        Theme::drawTextVCentered(target, m_font, nav.label, Theme::Type::META,
                                 active ? Theme::TEXT_PRIMARY
                                        : Theme::lerp(Theme::TEXT_SECONDARY,
                                                      Theme::TEXT_PRIMARY, m_navT[i]),
                                 tx, nav.y, 42.f,
                                 active ? sf::Text::Bold : sf::Text::Regular);
    }

    if (m_logoutT > 0.004f)
        Theme::fillRoundedRect(target, {8.f, wh - 52.f}, {SW - 18.f, 40.f}, 10.f,
                               Theme::withAlpha(Theme::DANGER,
                                                static_cast<std::uint8_t>(45 * m_logoutT)));
    Theme::drawSeparatorSoft(target, 10.f, wh - 56.f, SW - 20.f);
    Theme::drawTextVCentered(target, m_font, "Logout", Theme::Type::META,
                             Theme::lerp(Theme::DANGER_HOVER, sf::Color::White,
                                         0.35f * m_logoutT),
                             22.f + 3.f * m_logoutT, wh - 52.f, 40.f, sf::Text::Bold);
}

void UserDashboardScreen::drawHeader(sf::RenderTarget& target)
{
    const float ww = m_size.x;

    Theme::drawHeaderBar(target, SW, ww - SW, HH, Theme::ACCENT, Theme::PURPLE);

    string pageTitle = "Dashboard";
    if      (m_state == USER_VIEW_UNIVERSITIES) pageTitle = "Universities";
    else if (m_state == USER_VIEW_BUSES)        pageTitle = "All Buses";
    else if (m_state == USER_SELECT_UNIVERSITY) pageTitle = "Buses by University";
    else if (m_state == USER_SEARCH_BUS)        pageTitle = "Search Bus";
    else if (m_state == USER_SEARCH_BY_STOP)    pageTitle = "Search by Stop";

    Theme::drawTextVCentered(target, m_font, pageTitle, Theme::Type::HEADING,
                             Theme::TEXT_PRIMARY, SW + 24.f, 0.f, HH, sf::Text::Bold);
}

void UserDashboardScreen::draw(sf::RenderTarget& target)
{
    const float ww = m_size.x, wh = m_size.y;
    const float CW = m_contentW, CH = m_contentH;
    const float mx = m_mouse.x, my = m_mouse.y;
    const bool  inContent = mx >= SW && my >= HH;

    const float CARD_X   = SW + 20.f;
    const float CARD_W   = CW - 40.f;
    const float LIST_TOP = HH + 20.f;

    const float sectionRise = (1.f - Theme::smoothstep01(m_stateT)) * RISE_PX;
    auto rowRise = [&](int i) {
        const float t = Theme::smoothstep01(
            std::clamp((m_resultsT - ROW_STAGGER * i) / 0.55f, 0.f, 1.f));
        return (1.f - t) * RISE_PX;
    };

    Theme::drawAppBase(target, {ww, wh});
    if (!Theme::backgroundReady())
    {
        Theme::drawRadialGlow(target, {ww * 0.78f, HH}, std::max(ww, wh) * 0.55f, Theme::ACCENT, 18);
        Theme::drawRadialGlow(target, {ww * 0.35f, wh}, std::max(ww, wh) * 0.45f, Theme::PURPLE, 16);
    }

    auto drawSearchBar = [&](TextBox& box) {
        box.draw(target);
        Button btn(m_font, "Search", {110.f, 44.f},
                   {m_srchBtnX, HH + 28.f + sectionRise});
        btn.setHoverT(m_searchHoverT);
        btn.draw(target);
    };

    if (m_state == USER_DASHBOARD)
    {
        auto allUnis  = m_user.getUniversities();
        auto allBuses = m_user.getBuses();

        float scy  = HH + 30.f;
        float scW  = (CW - 60.f) * 0.5f;
        float sc1X = SW + 20.f, sc2X = SW + 30.f + scW;

        auto statCard = [&](float x, sf::Color accent, const string& value,
                            const string& label, int idx) {
            const float sy = scy + rowRise(idx);
            Theme::drawCard(target, {x, sy}, {scW, 116.f}, Theme::BG_CARD, 12.f);
            Theme::drawAccentBar(target, x, sy, 116.f, accent, 5.f);
            Theme::drawText(target, m_font, value, 46, accent,
                            {x + 24.f, sy + 12.f}, sf::Text::Bold);
            Theme::drawText(target, m_font, label, Theme::Type::LABEL,
                            Theme::TEXT_MUTED, {x + 24.f, sy + 84.f}, sf::Text::Bold);
        };
        statCard(sc1X, Theme::ACCENT, to_string(allUnis.size()),  "UNIVERSITIES", 0);
        statCard(sc2X, Theme::PURPLE, to_string(allBuses.size()), "BUSES",        1);

        Theme::drawText(target, m_font,
                        "Use the sidebar to browse universities and bus routes.",
                        Theme::Type::META, Theme::TEXT_SECONDARY,
                        {SW + 22.f, scy + 148.f + rowRise(2)});
    }
    else if (m_state == USER_VIEW_UNIVERSITIES)
    {
        const float UCARD_H = 76.f;
        float totalH = m_unis.size() * (UCARD_H + CARD_GAP);
        m_maxScroll  = std::max(0.f, totalH - CH + 40.f);
        m_scrollOff  = std::clamp(m_scrollOff, 0.f, m_maxScroll);

        const float badgeH = Theme::badgeHeight(m_font, Theme::Type::BADGE_UNI);

        for (int i = 0; i < static_cast<int>(m_unis.size()); ++i)
        {
            float cy = LIST_TOP + i * (UCARD_H + CARD_GAP) - m_scrollOff + rowRise(i);
            if (cy + UCARD_H < HH || cy > wh) continue;

            bool hov = inContent && my >= cy && my < cy + UCARD_H;
            sf::Color bg = hov ? Theme::ITEM_HOVER : Theme::ITEM_BG;

            Theme::drawCard(target, {CARD_X, cy}, {CARD_W, UCARD_H}, bg, 8.f);
            Theme::drawAccentBar(target, CARD_X, cy, UCARD_H, Theme::ACCENT);

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
    else if (m_state == USER_VIEW_BUSES)
    {
        float totalH = m_buses.size() * (CARD_H + CARD_GAP);
        m_maxScroll  = std::max(0.f, totalH - CH + 40.f);
        m_scrollOff  = std::clamp(m_scrollOff, 0.f, m_maxScroll);

        for (int i = 0; i < static_cast<int>(m_buses.size()); ++i)
        {
            float cy = LIST_TOP + i * (CARD_H + CARD_GAP) - m_scrollOff + rowRise(i);
            if (cy + CARD_H < HH || cy > wh) continue;
            bool hov = inContent && my >= cy && my < cy + CARD_H;
            drawBusCard(target, m_buses[i], CARD_X, cy, CARD_W, hov, true);
        }
        if (m_buses.empty())
            Theme::drawCenteredText(target, m_font, "No buses registered.",
                                    Theme::Type::BODY, Theme::TEXT_MUTED,
                                    {{SW + 20.f, HH + 80.f}, {CW - 40.f, 60.f}});
    }
    else if (m_state == USER_SELECT_UNIVERSITY)
    {
        drawSearchBar(m_uniCodeBox);

        float listTop = HH + 92.f;
        float totalH  = m_buses.size() * (CARD_H + CARD_GAP);
        m_maxScroll   = std::max(0.f, totalH - (CH - 90.f) + 40.f);
        m_scrollOff   = std::clamp(m_scrollOff, 0.f, m_maxScroll);

        for (int i = 0; i < static_cast<int>(m_buses.size()); ++i)
        {
            float cy = listTop + i * (CARD_H + CARD_GAP) - m_scrollOff + rowRise(i);
            if (cy + CARD_H < HH || cy > wh) continue;
            bool hov = inContent && my >= cy && my < cy + CARD_H;
            drawBusCard(target, m_buses[i], CARD_X, cy, CARD_W, hov, false);
        }
    }
    else if (m_state == USER_SEARCH_BUS)
    {
        drawSearchBar(m_busIdBox);

        if (!m_buses.empty())
        {
            const Bus& b = m_buses[0];
            float cy = HH + 92.f + rowRise(0);
            Theme::drawCard(target, {CARD_X, cy}, {CARD_W, 230.f}, Theme::ITEM_BG, 10.f);
            Theme::drawAccentBar(target, CARD_X, cy, 230.f, Theme::PURPLE, 4.f);

            auto row = [&](const string& lbl, const string& val, float ry,
                           unsigned valSize = Theme::Type::BODY,
                           std::uint32_t valStyle = sf::Text::Regular,
                           sf::Color valColor = Theme::TEXT_SECONDARY) {
                Theme::drawText(target, m_font, lbl, Theme::Type::LABEL,
                                Theme::TEXT_MUTED, {CARD_X + 20.f, cy + ry + 3.f},
                                sf::Text::Bold);
                Theme::drawText(target, m_font, val, valSize, valColor,
                                {CARD_X + 160.f, cy + ry}, valStyle,
                                Theme::Type::LEADING_BODY);
            };
            row("BUS ID",     b.getBusID(),   16.f, Theme::Type::BADGE_BUS,
                sf::Text::Bold, Theme::TEXT_PRIMARY);
            row("NAME",       b.getBusName(), 54.f, Theme::Type::BUS_NAME,
                sf::Text::Bold, Theme::TEXT_PRIMARY);
            row("UNIVERSITY", b.getUniversityCode(),        96.f);
            row("SEATS",      to_string(b.getTotalSeats()), 126.f);

            string wrappedRoute = wrapRoute(b.getRoute(), CARD_W - 190.f,
                                            m_font, Theme::Type::ROUTE, "");
            Theme::drawText(target, m_font, "ROUTE", Theme::Type::LABEL,
                            Theme::TEXT_MUTED, {CARD_X + 20.f, cy + 159.f},
                            sf::Text::Bold);
            Theme::drawText(target, m_font, wrappedRoute, Theme::Type::ROUTE,
                            Theme::TEXT_ROUTE, {CARD_X + 160.f, cy + 156.f},
                            sf::Text::Regular, Theme::Type::LEADING_BODY);
        }
    }
    else if (m_state == USER_SEARCH_BY_STOP)
    {
        drawSearchBar(m_stopBox);

        float listTop = HH + 92.f;
        float totalH  = m_buses.size() * (CARD_H + CARD_GAP);
        m_maxScroll   = std::max(0.f, totalH - (CH - 90.f) + 40.f);
        m_scrollOff   = std::clamp(m_scrollOff, 0.f, m_maxScroll);

        for (int i = 0; i < static_cast<int>(m_buses.size()); ++i)
        {
            float cy = listTop + i * (CARD_H + CARD_GAP) - m_scrollOff;
            if (cy + CARD_H < HH || cy > wh) continue;
            bool hov = inContent && my >= cy && my < cy + CARD_H;
            drawBusCard(target, m_buses[i], CARD_X, cy, CARD_W, hov, true);
        }
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
