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

// ── Route word-wrap helper ───────────────────────────────────────────────────
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

// ── State ────────────────────────────────────────────────────────────────────
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
    if (!font.openFromFile("assets/Inter-Regular.ttf")) return;
    Theme::configureFont(font);

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

    // Sidebar layout constants
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

    // ── Pre-compute search-box layout (fixed window 1100x720) ───────────────
    const float CW_FIXED = 1100.f - SW;   // 890
    const float fX_FIX   = SW + 32.f;     // 242
    const float fW_FIX   = CW_FIXED - 64.f; // 826
    const float srchW    = fW_FIX * 0.55f;
    const float srchBtnX = fX_FIX + srchW + 12.f;

    // ── Search TextBox objects OUTSIDE the game loop so text persists ───
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



        // Sync focus each frame (no re-construction)
        uniCodeBox.setFocused(state == USER_SELECT_UNIVERSITY);
        busIdBox.setFocused  (state == USER_SEARCH_BUS);
        stopBox.setFocused   (state == USER_SEARCH_BY_STOP);

        Button srchUniBtn (font, "Search", {110.f, 44.f}, {srchBtnX, HH + 28.f});
        Button srchBusBtn (font, "Search", {110.f, 44.f}, {srchBtnX, HH + 28.f});
        Button srchStopBtn(font, "Search", {110.f, 44.f}, {srchBtnX, HH + 28.f});


        // Card geometry
        const float CARD_H   = 90.f;
        const float CARD_GAP = 10.f;
        const float CARD_X   = SW + 20.f;
        const float CARD_W   = CW - 40.f;
        const float LIST_TOP = HH + 20.f;

        // Mouse
        auto mp  = sf::Mouse::getPosition(window);
        float mx = static_cast<float>(mp.x);
        float my = static_cast<float>(mp.y);
        bool inContent = mx >= SW && my >= HH;

        // ── Events ───────────────────────────────────────────────────────
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>()) window.close();

            if (event->is<sf::Event::MouseButtonPressed>())
            {
                // Sidebar nav
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
                    // Search triggers
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

            if (event->is<sf::Event::TextEntered>()) {
                if (state == USER_SELECT_UNIVERSITY) uniCodeBox.handleEvent(*event);
                if (state == USER_SEARCH_BUS)        busIdBox.handleEvent(*event);
                if (state == USER_SEARCH_BY_STOP)    stopBox.handleEvent(*event);
            }
        }

        srchUniBtn.update(window);
        srchBusBtn.update(window);
        srchStopBtn.update(window);

        // ═══════════════════════════════════════════════════════════════
        //  DRAW
        // ═══════════════════════════════════════════════════════════════
        window.clear(Theme::BG_DARK);

        // ── Helper: draw a bus card ───────────────────────────────────────
        auto drawBusCard = [&](Bus b, float cx, float cy,
                               float cardW, bool hov, bool includeUni)
        {
            sf::Color bg = hov ? Theme::ITEM_HOVER : Theme::ITEM_BG;
            Theme::drawCard(window, {cx, cy}, {cardW, CARD_H}, bg, 8.f);
            Theme::drawAccentBar(window, cx, cy, CARD_H, Theme::PURPLE);

            // ID badge — high contrast
            Theme::drawBadge(window, font, b.getBusID(),
                             {cx + 14.f, cy + 8.f},
                             Theme::withAlpha(Theme::PURPLE, 90), sf::Color(220, 200, 255));

            // Bus name — primary, large
            sf::Text nm(font); nm.setString(b.getBusName());
            nm.setCharacterSize(16); nm.setFillColor(Theme::TEXT_PRIMARY);
            nm.setPosition(Theme::px(cx + 14.f, cy + 30.f));
            window.draw(nm);

            // Secondary: uni code + seats
            string secondary = includeUni ? b.getUniversityCode() + "  ·  " : "";
            secondary += to_string(b.getTotalSeats()) + " seats";
            sf::Text sec(font); sec.setString(secondary);
            sec.setCharacterSize(12); sec.setFillColor(Theme::TEXT_SECONDARY);
            sec.setPosition(Theme::px(cx + 14.f, cy + 50.f));
            window.draw(sec);

            // Route — brighter gray for readability
            string wrappedRoute = wrapRouteU(b.getRoute(), cardW - 30.f, font, 12, "");
            sf::Text rt(font); rt.setString(wrappedRoute);
            rt.setCharacterSize(12); rt.setFillColor(sf::Color(140, 155, 175));
            rt.setPosition(Theme::px(cx + 14.f, cy + 66.f));
            window.draw(rt);
        };

        // ── Content ──────────────────────────────────────────────────────
        if (state == USER_DASHBOARD) {
            auto allUnis  = user.getUniversities();
            auto allBuses = user.getBuses();

            float scy = HH + 30.f;
            float scW = (CW - 60.f) * 0.5f;
            float sc1X = SW + 20.f, sc2X = SW + 30.f + scW;

            Theme::drawCard(window, {sc1X, scy}, {scW, 108.f}, Theme::BG_CARD, 12.f);
            Theme::drawAccentBar(window, sc1X, scy, 108.f, Theme::ACCENT, 5.f);
            sf::Text n1(font); n1.setString(to_string(allUnis.size()));
            n1.setCharacterSize(44); n1.setFillColor(Theme::ACCENT);
            sf::FloatRect b1 = n1.getLocalBounds();
            n1.setPosition(Theme::px(sc1X + 22.f, scy + 14.f - b1.position.y));
            window.draw(n1);
            sf::Text l1(font); l1.setString("UNIVERSITIES");
            l1.setCharacterSize(11); l1.setFillColor(Theme::TEXT_SECONDARY);
            l1.setPosition(Theme::px(sc1X + 22.f, scy + 78.f));
            window.draw(l1);

            Theme::drawCard(window, {sc2X, scy}, {scW, 108.f}, Theme::BG_CARD, 12.f);
            Theme::drawAccentBar(window, sc2X, scy, 108.f, Theme::PURPLE, 5.f);
            sf::Text n2(font); n2.setString(to_string(allBuses.size()));
            n2.setCharacterSize(44); n2.setFillColor(Theme::PURPLE);
            sf::FloatRect b2 = n2.getLocalBounds();
            n2.setPosition(Theme::px(sc2X + 22.f, scy + 14.f - b2.position.y));
            window.draw(n2);
            sf::Text l2(font); l2.setString("BUSES");
            l2.setCharacterSize(11); l2.setFillColor(Theme::TEXT_SECONDARY);
            l2.setPosition(Theme::px(sc2X + 22.f, scy + 78.f));
            window.draw(l2);

            sf::Text hint(font);
            hint.setString("Use the sidebar to browse universities and bus routes.");
            hint.setCharacterSize(13); hint.setFillColor(Theme::TEXT_MUTED);
            hint.setPosition(Theme::px(SW + 22.f, scy + 132.f));
            window.draw(hint);
        }
        else if (state == USER_VIEW_UNIVERSITIES) {
            float totalH = unis.size() * (65.f + CARD_GAP);
            maxScroll = max(0.f, totalH - CH + 40.f);
            scrollOff = clamp(scrollOff, 0.f, maxScroll);

            for (int i = 0; i < (int)unis.size(); ++i) {
                float cy = LIST_TOP + i * (65.f + CARD_GAP) - scrollOff;
                if (cy + 65.f < HH || cy > wh) continue;
                bool hov = inContent && my >= cy && my < cy + 65.f;
                sf::Color bg = hov ? Theme::ITEM_HOVER : Theme::ITEM_BG;
                Theme::drawCard(window, {CARD_X, cy}, {CARD_W, 65.f}, bg, 8.f);
                Theme::drawAccentBar(window, CARD_X, cy, 65.f, Theme::ACCENT);

                // Badge — high contrast
                Theme::drawBadge(window, font, unis[i].first,
                                 {CARD_X + 14.f, cy + (65.f - 28.f) * 0.5f},
                                 Theme::withAlpha(Theme::ACCENT, 90), sf::Color(220, 235, 255));

                float badgeW = static_cast<float>(unis[i].first.size()) * 8.5f + 32.f;
                sf::Text nm(font); nm.setString(unis[i].second);
                nm.setCharacterSize(15); nm.setFillColor(Theme::TEXT_PRIMARY);
                sf::FloatRect bn = nm.getLocalBounds();
                nm.setPosition(Theme::px(CARD_X + 14.f + badgeW + 14.f,
                                         cy + (65.f - bn.size.y) * 0.5f - bn.position.y));
                window.draw(nm);
            }
            if (unis.empty()) {
                Theme::drawCenteredText(window, font, "No universities registered.",
                                        16, Theme::TEXT_MUTED,
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
                                        16, Theme::TEXT_MUTED,
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
                // Show full detail card for the matched bus
                auto b = buses[0];
                float cy = HH + 92.f;
                Theme::drawCard(window, {CARD_X, cy}, {CARD_W, 200.f}, Theme::ITEM_BG, 10.f);
                Theme::drawAccentBar(window, CARD_X, cy, 200.f, Theme::PURPLE, 4.f);

                auto row = [&](const string& lbl, const string& val, float ry) {
                    sf::Text lt(font); lt.setString(lbl);
                    lt.setCharacterSize(12); lt.setFillColor(Theme::TEXT_MUTED);
                    lt.setPosition({CARD_X + 18.f, cy + ry}); window.draw(lt);
                    sf::Text vt(font); vt.setString(val);
                    vt.setCharacterSize(15); vt.setFillColor(Theme::TEXT_PRIMARY);
                    vt.setPosition({CARD_X + 148.f, cy + ry - 2.f}); window.draw(vt);
                };
                row("Bus ID",     b.getBusID(),                          14.f);
                row("Name",       b.getBusName(),                        42.f);
                row("University", b.getUniversityCode(),                 70.f);
                row("Seats",      to_string(b.getTotalSeats()),          98.f);

                string wrappedRoute = wrapRouteU(b.getRoute(), CARD_W - 170.f, font, 14);
                row("Route", wrappedRoute, 126.f);
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

        // ── SIDEBAR ───────────────────────────────────────────────────────
        {
            sf::RectangleShape sbg({SW, wh});
            sbg.setFillColor(Theme::BG_SIDEBAR); window.draw(sbg);
            sf::RectangleShape sbd({1.f, wh});
            sbd.setPosition({SW - 1.f, 0.f});
            sbd.setFillColor(Theme::BORDER_IDLE); window.draw(sbd);

            sf::Text an(font); an.setString("Bus Tracker");
            an.setCharacterSize(16); an.setFillColor(Theme::ACCENT);
            sf::FloatRect ab = an.getLocalBounds();
            an.setOrigin({ab.position.x + ab.size.x * 0.5f, 0.f});
            an.setPosition({SW * 0.5f, 16.f}); window.draw(an);
            sf::Text ro(font); ro.setString("User");
            ro.setCharacterSize(11); ro.setFillColor(Theme::TEXT_MUTED);
            sf::FloatRect rb = ro.getLocalBounds();
            ro.setOrigin({rb.position.x + rb.size.x * 0.5f, 0.f});
            ro.setPosition({SW * 0.5f, 36.f}); window.draw(ro);

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

                sf::Text nt(font); nt.setString(nav.label);
                nt.setCharacterSize(13);
                nt.setFillColor(active ? Theme::TEXT_PRIMARY : Theme::TEXT_SECONDARY);
                sf::FloatRect nb = nt.getLocalBounds();
                nt.setPosition({16.f, nav.y + (42.f - nb.size.y) * 0.5f - nb.position.y});
                window.draw(nt);
            }

            // Logout
            bool lh = mx < SW && my >= wh - 52.f && my < wh - 12.f;
            if (lh) {
                sf::RectangleShape lb({SW, 40.f});
                lb.setPosition({0.f, wh - 52.f}); lb.setFillColor(Theme::SIDEBAR_HOVER);
                window.draw(lb);
            }
            Theme::drawSeparator(window, 0.f, wh - 56.f, SW);
            sf::Text lt(font); lt.setString("Logout");
            lt.setCharacterSize(14); lt.setFillColor(Theme::DANGER);
            sf::FloatRect ltb = lt.getLocalBounds();
            lt.setPosition({16.f, wh - 52.f + (40.f - ltb.size.y) * 0.5f - ltb.position.y});
            window.draw(lt);
        }

        // ── HEADER BAR ────────────────────────────────────────────────────
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

            sf::Text pt(font); pt.setString(pageTitle);
            pt.setCharacterSize(17); pt.setFillColor(Theme::TEXT_PRIMARY);
            sf::FloatRect pb = pt.getLocalBounds();
            pt.setPosition({SW + 22.f, (HH - pb.size.y) * 0.5f - pb.position.y});
            window.draw(pt);
        }

        // ── Toast ─────────────────────────────────────────────────────────
        if (showInfo) {
            float elapsed = infoTimer.getElapsedTime().asSeconds();
            float alpha   = elapsed > 2.f ? 1.f - (elapsed - 2.f) : 1.f;
            Theme::drawInfoToast(window, font, infoText, infoErr, ww, wh, alpha);
        }

        window.display();
    }
}
