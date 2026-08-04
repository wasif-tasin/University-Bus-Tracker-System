#include "UserDashboardGUI.h"
#include "Button.h"
#include "TextBox.h"
#include "Theme.h"
#include "User.h"
#include "Bus.h"

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

static string wrapRouteU(const string& route, float maxW,
                          const sf::Font& font, unsigned sz,
                          const string& prefix = "Route: ")
{
    string wrapped = prefix;
    string indent(prefix.length(), ' ');
    string cur;
    sf::Text tmp(font);
    tmp.setCharacterSize(sz);

    vector<string> stops;
    string s;
    for (char c : route) {
        if (c == ',') { if (!s.empty()) { stops.push_back(s); s=""; } }
        else s += c;
    }
    if (!s.empty()) stops.push_back(s);

    for (size_t i = 0; i < stops.size(); ++i) {
        string st = stops[i];
        while (!st.empty() && st.front() == ' ') st = st.substr(1);
        while (!st.empty() && st.back()  == ' ') st.pop_back();
        string candidate = st + (i < stops.size() - 1 ? ", " : "");
        tmp.setString(cur + candidate);
        if (tmp.getLocalBounds().size.x > maxW && !cur.empty()) {
            wrapped += cur + "\n" + indent;
            cur = candidate;
        } else {
            cur += candidate;
        }
    }
    wrapped += cur;
    return wrapped;
}

enum UserState {
    USER_DASHBOARD,
    USER_VIEW_UNIVERSITIES,
    USER_SELECT_UNIVERSITY,
    USER_VIEW_BUSES,
    USER_SEARCH_BUS,
    USER_SEARCH_BY_STOP
};

void UserDashboardGUI::run()
{
    sf::RenderWindow window(sf::VideoMode({1100, 720}),
                            "University Bus Tracker - User Dashboard");
    window.setFramerateLimit(60);

    sf::Font font;
    if (!Theme::loadUIFont(font)) return;

    User user;

    UserState state     = USER_DASHBOARD;
    float     scrollOff = 0.f, maxScroll = 0.f;

    vector<pair<string,string>> unis;
    vector<Bus>                 buses;

    string infoText;
    bool   infoErr  = false, showInfo = false;
    sf::Clock infoTimer;
    auto setInfo = [&](const string& msg, bool err) {
        infoText = msg; infoErr = err; showInfo = true; infoTimer.restart();
    };

    const float SW = 210.f;
    const float HH = 60.f;

    struct NavItem { string label; UserState st; float y; };
    const vector<NavItem> navItems = {
        {"Universities",    USER_VIEW_UNIVERSITIES, 82.f},
        {"All Buses",       USER_VIEW_BUSES,        132.f},
        {"By University",   USER_SELECT_UNIVERSITY, 182.f},
        {"Search Bus ID",   USER_SEARCH_BUS,        232.f},
        {"Search by Stop",  USER_SEARCH_BY_STOP,    282.f},
    };

    const float CW_FIXED = 1100.f - SW;   
    const float fX_FIX   = SW + 32.f;     
    const float fW_FIX   = CW_FIXED - 64.f; 
    const float srchW    = fW_FIX * 0.55f;
    const float srchBtnX = fX_FIX + srchW + 12.f;

    TextBox uniCodeBox(font, {srchW, 44.f}, {fX_FIX, HH + 28.f});
    TextBox busIdBox  (font, {srchW, 44.f}, {fX_FIX, HH + 28.f});
    TextBox stopBox   (font, {srchW, 44.f}, {fX_FIX, HH + 28.f});
    uniCodeBox.setPlaceholder("Enter university code (e.g. BUET)");
    busIdBox.setPlaceholder  ("Enter Bus ID");
    stopBox.setPlaceholder   ("Enter stop name");


    while (window.isOpen())
    {
        auto sz  = window.getSize();
        float ww = static_cast<float>(sz.x);
        float wh = static_cast<float>(sz.y);
        float CW = ww - SW;
        float CH = wh - HH;

        if (showInfo && infoTimer.getElapsedTime().asSeconds() > 3.f)
            showInfo = false;



        uniCodeBox.setFocused(state == USER_SELECT_UNIVERSITY);
        busIdBox.setFocused  (state == USER_SEARCH_BUS);
        stopBox.setFocused   (state == USER_SEARCH_BY_STOP);

        Button srchUniBtn (font, "Search", {110.f, 44.f}, {srchBtnX, HH + 28.f});
        Button srchBusBtn (font, "Search", {110.f, 44.f}, {srchBtnX, HH + 28.f});
        Button srchStopBtn(font, "Search", {110.f, 44.f}, {srchBtnX, HH + 28.f});

        const float CARD_H   = 108.f;
        const float CARD_GAP = 12.f;
        const float CARD_X   = SW + 20.f;
        const float CARD_W   = CW - 40.f;
        const float LIST_TOP = HH + 20.f;

        auto mp  = sf::Mouse::getPosition(window);
        float mx = static_cast<float>(mp.x);
        float my = static_cast<float>(mp.y);
        bool inContent = mx >= SW && my >= HH;

        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>()) window.close();
            Theme::syncViewToWindow(window, *event);
            if (event->is<sf::Event::MouseButtonPressed>())
            {
                if (mx < SW) {
                    for (auto& nav : navItems) {
                        if (my >= nav.y && my < nav.y + 42.f) {
                            state     = nav.st;
                            scrollOff = 0.f;
                            buses.clear();
                            if (state == USER_VIEW_UNIVERSITIES)
                                unis  = user.getUniversities();
                            if (state == USER_VIEW_BUSES)
                                buses = user.getBuses();
                        }
                    }
                    if (my >= wh - 52.f && my < wh - 12.f)
                        window.close();
                }

                if (inContent) {
                    if (state == USER_SELECT_UNIVERSITY && srchUniBtn.isClicked(window)) {
                        buses = user.getBusesForUniversity(uniCodeBox.getText());
                        scrollOff = 0.f;
                        if (buses.empty()) setInfo("No buses found for: " + uniCodeBox.getText(), true);
                    }
                    if (state == USER_SEARCH_BUS && srchBusBtn.isClicked(window)) {
                        buses = user.searchBus(busIdBox.getText());
                        scrollOff = 0.f;
                        if (buses.empty()) setInfo("Bus not found: " + busIdBox.getText(), true);
                    }
                    if (state == USER_SEARCH_BY_STOP && srchStopBtn.isClicked(window)) {
                        buses = user.searchByStop(stopBox.getText());
                        scrollOff = 0.f;
                        if (buses.empty()) setInfo("No buses pass through: " + stopBox.getText(), true);
                    }
                }
            }

            if (const auto* mw = event->getIf<sf::Event::MouseWheelScrolled>()) {
                if (inContent) {
                    scrollOff -= mw->delta * 36.f;
                    scrollOff  = std::clamp(scrollOff, 0.f, maxScroll);
                }
            }

            if (event->is<sf::Event::TextEntered>() ||
                event->is<sf::Event::KeyPressed>()  ||
                event->is<sf::Event::MouseButtonPressed>()) {
                if (state == USER_SELECT_UNIVERSITY) uniCodeBox.handleEvent(*event);
                if (state == USER_SEARCH_BUS)        busIdBox.handleEvent(*event);
                if (state == USER_SEARCH_BY_STOP)    stopBox.handleEvent(*event);
            }
        }

        srchUniBtn.update(window);
        srchBusBtn.update(window);
        srchStopBtn.update(window);

        window.clear(Theme::BG_DARK);

        auto drawBusCard = [&](Bus b, float cx, float cy,
                               float cardW, bool hov, bool includeUni)
        {
            sf::Color bg = hov ? Theme::ITEM_HOVER : Theme::ITEM_BG;
            Theme::drawCard(window, {cx, cy}, {cardW, CARD_H}, bg, 8.f);
            Theme::drawAccentBar(window, cx, cy, CARD_H, Theme::PURPLE);

            const float padL   = 16.f;
            const float badgeH = Theme::badgeHeight(font, Theme::Type::BADGE_BUS);
            const float rowY   = cy + 14.f;              

            float badgeW = Theme::drawBadge(window, font, b.getBusID(),
                                            {cx + padL, rowY},
                                            Theme::BADGE_BUS_BG, Theme::BADGE_BUS_TEXT,
                                            Theme::Type::BADGE_BUS, Theme::BADGE_BUS_EDGE);

            const float nameX = cx + padL + badgeW + 16.f;
            Theme::drawTextVCentered(window, font, b.getBusName(),
                                     Theme::Type::BUS_NAME, Theme::TEXT_PRIMARY,
                                     nameX, rowY, badgeH, sf::Text::Bold);

            string secondary = includeUni ? b.getUniversityCode() + "  ·  " : "";
            secondary += to_string(b.getTotalSeats()) + " seats";
            const float metaY = rowY + badgeH + 12.f;
            Theme::drawText(window, font, secondary, Theme::Type::META,
                            Theme::TEXT_SECONDARY, {cx + padL, metaY});

            string route = Theme::ellipsize(font, b.getRoute(), Theme::Type::ROUTE,
                                            cardW - padL - 24.f);
            Theme::drawText(window, font, route, Theme::Type::ROUTE,
                            Theme::TEXT_ROUTE, {cx + padL, metaY + 24.f});
        };

        if (state == USER_DASHBOARD) {
            auto allUnis  = user.getUniversities();
            auto allBuses = user.getBuses();

            float scy = HH + 30.f;
            float scW = (CW - 60.f) * 0.5f;
            float sc1X = SW + 20.f, sc2X = SW + 30.f + scW;

            auto statCard = [&](float x, sf::Color accent, const string& value,
                                const string& label) {
                Theme::drawCard(window, {x, scy}, {scW, 116.f}, Theme::BG_CARD, 12.f);
                Theme::drawAccentBar(window, x, scy, 116.f, accent, 5.f);
                Theme::drawText(window, font, value, 46, accent,
                                {x + 24.f, scy + 12.f}, sf::Text::Bold);
                Theme::drawText(window, font, label, Theme::Type::LABEL,
                                Theme::TEXT_MUTED, {x + 24.f, scy + 84.f},
                                sf::Text::Bold);
            };
            statCard(sc1X, Theme::ACCENT, to_string(allUnis.size()),  "UNIVERSITIES");
            statCard(sc2X, Theme::PURPLE, to_string(allBuses.size()), "BUSES");

            Theme::drawText(window, font,
                            "Use the sidebar to browse universities and bus routes.",
                            Theme::Type::META, Theme::TEXT_SECONDARY,
                            {SW + 22.f, scy + 148.f});
        }
        else if (state == USER_VIEW_UNIVERSITIES) {
            const float UCARD_H = 76.f;
            float totalH = unis.size() * (UCARD_H + CARD_GAP);
            maxScroll = max(0.f, totalH - CH + 40.f);
            scrollOff = clamp(scrollOff, 0.f, maxScroll);

            const float badgeH = Theme::badgeHeight(font, Theme::Type::BADGE_UNI);

            for (int i = 0; i < (int)unis.size(); ++i) {
                float cy = LIST_TOP + i * (UCARD_H + CARD_GAP) - scrollOff;
                if (cy + UCARD_H < HH || cy > wh) continue;
                bool hov = inContent && my >= cy && my < cy + UCARD_H;
                sf::Color bg = hov ? Theme::ITEM_HOVER : Theme::ITEM_BG;
                Theme::drawCard(window, {CARD_X, cy}, {CARD_W, UCARD_H}, bg, 8.f);
                Theme::drawAccentBar(window, CARD_X, cy, UCARD_H, Theme::ACCENT);

                float badgeW = Theme::drawBadge(
                    window, font, unis[i].first,
                    {CARD_X + 16.f, std::round(cy + (UCARD_H - badgeH) * 0.5f)},
                    Theme::BADGE_UNI_BG, Theme::BADGE_UNI_TEXT,
                    Theme::Type::BADGE_UNI, Theme::BADGE_UNI_EDGE);

                Theme::drawTextVCentered(window, font, unis[i].second,
                                         Theme::Type::SUBTITLE, Theme::TEXT_PRIMARY,
                                         CARD_X + 16.f + badgeW + 16.f,
                                         cy, UCARD_H, sf::Text::Bold);
            }
            if (unis.empty()) {
                Theme::drawCenteredText(window, font, "No universities registered.",
                                        Theme::Type::BODY, Theme::TEXT_MUTED,
                                        {{SW + 20.f, HH + 80.f}, {CW - 40.f, 60.f}});
            }
        }
        else if (state == USER_VIEW_BUSES) {
            float totalH = buses.size() * (CARD_H + CARD_GAP);
            maxScroll = max(0.f, totalH - CH + 40.f);
            scrollOff = clamp(scrollOff, 0.f, maxScroll);

            for (int i = 0; i < (int)buses.size(); ++i) {
                float cy = LIST_TOP + i * (CARD_H + CARD_GAP) - scrollOff;
                if (cy + CARD_H < HH || cy > wh) continue;
                bool hov = inContent && my >= cy && my < cy + CARD_H;
                drawBusCard(buses[i], CARD_X, cy, CARD_W, hov, true);
            }
            if (buses.empty()) {
                Theme::drawCenteredText(window, font, "No buses registered.",
                                        Theme::Type::BODY, Theme::TEXT_MUTED,
                                        {{SW + 20.f, HH + 80.f}, {CW - 40.f, 60.f}});
            }
        }
        else if (state == USER_SELECT_UNIVERSITY) {
            uniCodeBox.draw(window);
            srchUniBtn.draw(window);

            float listTop = HH + 92.f;
            float totalH  = buses.size() * (CARD_H + CARD_GAP);
            maxScroll = max(0.f, totalH - (CH - 90.f) + 40.f);
            scrollOff = clamp(scrollOff, 0.f, maxScroll);

            for (int i = 0; i < (int)buses.size(); ++i) {
                float cy = listTop + i * (CARD_H + CARD_GAP) - scrollOff;
                if (cy + CARD_H < HH || cy > wh) continue;
                bool hov = inContent && my >= cy && my < cy + CARD_H;
                drawBusCard(buses[i], CARD_X, cy, CARD_W, hov, false);
            }
        }
        else if (state == USER_SEARCH_BUS) {
            busIdBox.draw(window);
            srchBusBtn.draw(window);

            if (!buses.empty()) {
                auto b = buses[0];
                float cy = HH + 92.f;
                Theme::drawCard(window, {CARD_X, cy}, {CARD_W, 230.f}, Theme::ITEM_BG, 10.f);
                Theme::drawAccentBar(window, CARD_X, cy, 230.f, Theme::PURPLE, 4.f);

                auto row = [&](const string& lbl, const string& val, float ry,
                               unsigned valSize = Theme::Type::BODY,
                               std::uint32_t valStyle = sf::Text::Regular,
                               sf::Color valColor = Theme::TEXT_SECONDARY) {
                    Theme::drawText(window, font, lbl, Theme::Type::LABEL,
                                    Theme::TEXT_MUTED, {CARD_X + 20.f, cy + ry + 3.f},
                                    sf::Text::Bold);
                    Theme::drawText(window, font, val, valSize, valColor,
                                    {CARD_X + 160.f, cy + ry}, valStyle,
                                    Theme::Type::LEADING_BODY);
                };
                row("BUS ID",     b.getBusID(),   16.f, Theme::Type::BADGE_BUS,
                    sf::Text::Bold, Theme::TEXT_PRIMARY);
                row("NAME",       b.getBusName(), 54.f, Theme::Type::BUS_NAME,
                    sf::Text::Bold, Theme::TEXT_PRIMARY);
                row("UNIVERSITY", b.getUniversityCode(),        96.f);
                row("SEATS",      to_string(b.getTotalSeats()), 126.f);

                string wrappedRoute = wrapRouteU(b.getRoute(), CARD_W - 190.f,
                                                 font, Theme::Type::ROUTE, "");
                Theme::drawText(window, font, "ROUTE", Theme::Type::LABEL,
                                Theme::TEXT_MUTED, {CARD_X + 20.f, cy + 159.f},
                                sf::Text::Bold);
                Theme::drawText(window, font, wrappedRoute, Theme::Type::ROUTE,
                                Theme::TEXT_ROUTE, {CARD_X + 160.f, cy + 156.f},
                                sf::Text::Regular, Theme::Type::LEADING_BODY);
            }
        }
        else if (state == USER_SEARCH_BY_STOP) {
            stopBox.draw(window);
            srchStopBtn.draw(window);

            float listTop = HH + 92.f;
            float totalH  = buses.size() * (CARD_H + CARD_GAP);
            maxScroll = max(0.f, totalH - (CH - 90.f) + 40.f);
            scrollOff = clamp(scrollOff, 0.f, maxScroll);

            for (int i = 0; i < (int)buses.size(); ++i) {
                float cy = listTop + i * (CARD_H + CARD_GAP) - scrollOff;
                if (cy + CARD_H < HH || cy > wh) continue;
                bool hov = inContent && my >= cy && my < cy + CARD_H;
                drawBusCard(buses[i], CARD_X, cy, CARD_W, hov, true);
            }
        }

        {
            sf::RectangleShape sbg({SW, wh});
            sbg.setFillColor(Theme::BG_SIDEBAR); window.draw(sbg);
            sf::RectangleShape sbd({1.f, wh});
            sbd.setPosition({SW - 1.f, 0.f});
            sbd.setFillColor(Theme::BORDER_IDLE); window.draw(sbd);

            Theme::drawTextHCentered(window, font, "Bus Tracker", Theme::Type::SUBTITLE,
                                     Theme::ACCENT, SW * 0.5f, 14.f, sf::Text::Bold);
            Theme::drawTextHCentered(window, font, "USER", Theme::Type::CAPTION,
                                     Theme::TEXT_MUTED, SW * 0.5f, 38.f, sf::Text::Bold);

            Theme::drawSeparator(window, 0.f, 70.f, SW);

            for (auto& nav : navItems) {
                bool active = (state == nav.st);
                bool hov    = mx < SW && my >= nav.y && my < nav.y + 42.f;
                sf::Color ibg = active ? Theme::SIDEBAR_SELECTED
                              : hov   ? Theme::SIDEBAR_HOVER
                              : sf::Color(0,0,0,0);
                if (active || hov) {
                    sf::RectangleShape ib({SW, 42.f});
                    ib.setPosition({0.f, nav.y}); ib.setFillColor(ibg);
                    window.draw(ib);
                }
                if (active) Theme::drawAccentBar(window, 0.f, nav.y, 42.f, Theme::ACCENT);

                Theme::drawTextVCentered(window, font, nav.label, Theme::Type::META,
                                         active ? Theme::TEXT_PRIMARY : Theme::TEXT_SECONDARY,
                                         18.f, nav.y, 42.f,
                                         active ? sf::Text::Bold : sf::Text::Regular);
            }

            bool lh = mx < SW && my >= wh - 52.f && my < wh - 12.f;
            if (lh) {
                sf::RectangleShape lb({SW, 40.f});
                lb.setPosition({0.f, wh - 52.f}); lb.setFillColor(Theme::SIDEBAR_HOVER);
                window.draw(lb);
            }
            Theme::drawSeparator(window, 0.f, wh - 56.f, SW);
            Theme::drawTextVCentered(window, font, "Logout", Theme::Type::META,
                                     Theme::DANGER_HOVER, 18.f, wh - 52.f, 40.f,
                                     sf::Text::Bold);
        }

        {
            sf::RectangleShape hbg({ww - SW, HH});
            hbg.setPosition({SW, 0.f}); hbg.setFillColor(Theme::BG_HEADER);
            window.draw(hbg);
            sf::RectangleShape hl({ww - SW, 1.5f});
            hl.setPosition({SW, HH - 1.5f}); hl.setFillColor(Theme::ACCENT);
            window.draw(hl);

            string pageTitle = "Dashboard";
            if (state == USER_VIEW_UNIVERSITIES) pageTitle = "Universities";
            else if (state == USER_VIEW_BUSES)    pageTitle = "All Buses";
            else if (state == USER_SELECT_UNIVERSITY) pageTitle = "Buses by University";
            else if (state == USER_SEARCH_BUS)    pageTitle = "Search Bus";
            else if (state == USER_SEARCH_BY_STOP) pageTitle = "Search by Stop";

            Theme::drawTextVCentered(window, font, pageTitle, Theme::Type::HEADING,
                                     Theme::TEXT_PRIMARY, SW + 24.f, 0.f, HH,
                                     sf::Text::Bold);
        }

        if (showInfo) {
            float elapsed = infoTimer.getElapsedTime().asSeconds();
            float alpha   = elapsed > 2.f ? 1.f - (elapsed - 2.f) : 1.f;
            Theme::drawInfoToast(window, font, infoText, infoErr, ww, wh, alpha);
        }

        window.display();
    }
}
