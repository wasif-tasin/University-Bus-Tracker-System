#include "AdminDashboardGUI.h"
#include "Button.h"
#include "TextBox.h"
#include "Theme.h"
#include "Admin.h"

#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>

using namespace std;

// (wrapRoute not needed in this file)


// ── State enum ───────────────────────────────────────────────────────────────
enum AdminState { DASHBOARD, VIEW_UNIVERSITIES, ADD_UNIVERSITY, VIEW_BUSES, ADD_BUS };

void AdminDashboardGUI::run()
{
    sf::RenderWindow window(sf::VideoMode({1100, 720}),
                            "University Bus Tracker - Admin Dashboard");
    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.openFromFile("assets/Inter-Regular.ttf")) return;
    Theme::configureFont(font);

    Admin admin;

    // ── State ────────────────────────────────────────────────────────────
    AdminState state    = DASHBOARD;
    int        selIdx   = -1;
    float      scrollOff = 0.f, maxScroll = 0.f;

    vector<pair<string,string>> unis;
    vector<Bus>                 buses;

    // ── Info toast ───────────────────────────────────────────────────────
    string    infoText;
    bool      infoErr  = false;
    bool      showInfo = false;
    sf::Clock infoTimer;
    auto setInfo = [&](const string& msg, bool err) {
        infoText  = msg; infoErr = err;
        showInfo  = true; infoTimer.restart();
    };

    // ── Focus tracking for form fields ───────────────────────────────────
    int focusField = 0;

    // ── Sidebar nav constants ─────────────────────────────────────────────
    const float SW   = 210.f; // sidebar width
    const float HH   = 60.f;  // header height
    struct NavItem { string label; AdminState st; float y; };
    const vector<NavItem> navItems = {
        {"Dashboard",    DASHBOARD,         82.f},
        {"Universities", VIEW_UNIVERSITIES, 132.f},
        {"Buses",        VIEW_BUSES,        182.f},
    };

    // ── Pre-compute form layout (window is fixed 1100x720) ───────────────
    const float CW_FIXED = 1100.f - SW;   // 890
    const float fX  = SW + 32.f;          // 242
    const float fW  = CW_FIXED - 64.f;    // 826
    const float fW2 = (fW - 24.f) * 0.5f; // half-col width
    const float c1X = fX;
    const float c2X = fX + fW2 + 24.f;

    // ── ALL TextBox objects live OUTSIDE the game loop ───────────────────
    // Add-University form
    TextBox uniCodeBox(font, {fW,  46.f}, {fX, HH + 108.f});
    TextBox uniNameBox(font, {fW,  46.f}, {fX, HH + 200.f});
    uniCodeBox.setPlaceholder("e.g.  BUET");
    uniNameBox.setPlaceholder("e.g.  Bangladesh University of Engineering");

    // Add-Bus form (two columns)
    TextBox busIdBox   (font, {fW2, 46.f}, {c1X, HH + 108.f});
    TextBox busNameBox (font, {fW2, 46.f}, {c1X, HH + 200.f});
    TextBox busUniBox  (font, {fW2, 46.f}, {c1X, HH + 292.f});
    TextBox busSeatsBox(font, {fW2, 46.f}, {c2X, HH + 108.f});
    TextBox busRouteBox(font, {fW2, 46.f}, {c2X, HH + 200.f});
    busIdBox.setPlaceholder   ("e.g.  BUS-001");
    busNameBox.setPlaceholder ("e.g.  City Express");
    busUniBox.setPlaceholder  ("University code");
    busSeatsBox.setPlaceholder("e.g.  40");
    busRouteBox.setPlaceholder("Stop1, Stop2, Stop3...");

    while (window.isOpen())
    {
        auto sz  = window.getSize();
        float ww = static_cast<float>(sz.x);
        float wh = static_cast<float>(sz.y);
        float CW = ww - SW;   // content area width
        float CH = wh - HH;   // content area height

        // Info auto-hide
        if (showInfo && infoTimer.getElapsedTime().asSeconds() > 3.f)
            showInfo = false;

        // Sync TextBox focus each frame (no re-construction!)
        uniCodeBox.setFocused(state == ADD_UNIVERSITY && focusField == 0);
        uniNameBox.setFocused(state == ADD_UNIVERSITY && focusField == 1);
        busIdBox.setFocused   (state == ADD_BUS && focusField == 0);
        busNameBox.setFocused (state == ADD_BUS && focusField == 1);
        busUniBox.setFocused  (state == ADD_BUS && focusField == 2);
        busSeatsBox.setFocused(state == ADD_BUS && focusField == 3);
        busRouteBox.setFocused(state == ADD_BUS && focusField == 4);

        // Form action buttons
        Button saveBtn  (font, "Save",   {160.f, 44.f}, {fX,         HH + 400.f});
        Button cancelBtn(font, "Cancel", {110.f, 44.f}, {fX + 170.f, HH + 400.f}, ButtonStyle::SECONDARY);
        Button saveBusBtn(font, "Save Bus", {160.f, 44.f}, {fX,        HH + 480.f});
        Button cancelBusBtn(font, "Cancel", {110.f, 44.f},{fX + 170.f, HH + 480.f}, ButtonStyle::SECONDARY);

        // Header toolbar buttons
        Button addBtn(font, "+ Add",  {110.f, 36.f}, {ww - 240.f, (HH - 36.f) * 0.5f});
        Button delBtn(font, "Delete", {110.f, 36.f}, {ww - 120.f, (HH - 36.f) * 0.5f},
                      selIdx >= 0 ? ButtonStyle::DANGER : ButtonStyle::SECONDARY);

        // ── Mouse info ────────────────────────────────────────────────────
        auto mp  = sf::Mouse::getPosition(window);
        float mx = static_cast<float>(mp.x);
        float my = static_cast<float>(mp.y);
        bool inContent = mx >= SW && my >= HH;

        // Card geometry for list
        const float CARD_H  = 80.f;
        const float CARD_GAP= 10.f;
        const float CARD_X  = SW + 20.f;
        const float CARD_W  = CW - 40.f;
        const float LIST_TOP= HH + 20.f;

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
                            state = nav.st;
                            selIdx = -1; scrollOff = 0.f;
                            if (state == VIEW_UNIVERSITIES)
                                unis  = admin.getUniversities();
                            if (state == VIEW_BUSES)
                                buses = admin.getBuses();
                        }
                    }
                    if (my >= wh - 52.f && my < wh - 12.f)
                        window.close();
                }

                // Header toolbar (must be before content check)
                if (my < HH && mx >= SW) {
                    if ((state == VIEW_UNIVERSITIES || state == ADD_UNIVERSITY) &&
                        addBtn.isClicked(window)) {
                        state = ADD_UNIVERSITY; focusField = 0;
                        selIdx = -1;
                    }
                    if ((state == VIEW_BUSES || state == ADD_BUS) &&
                        addBtn.isClicked(window)) {
                        state = ADD_BUS; focusField = 0;
                        selIdx = -1;
                    }
                    if (state == VIEW_UNIVERSITIES && delBtn.isClicked(window) &&
                        selIdx >= 0 && selIdx < (int)unis.size()) {
                        string err;
                        if (admin.deleteUniversity(unis[selIdx].first, err)) {
                            unis = admin.getUniversities();
                            selIdx = -1;
                            setInfo("University deleted.", false);
                        } else {
                            setInfo("Error: " + err, true);
                        }
                    }
                    if (state == VIEW_BUSES && delBtn.isClicked(window) &&
                        selIdx >= 0 && selIdx < (int)buses.size()) {
                        string err;
                        if (admin.deleteBus(buses[selIdx].getBusID(), err)) {
                            buses = admin.getBuses();
                            selIdx = -1;
                            setInfo("Bus deleted.", false);
                        } else {
                            setInfo("Error: " + err, true);
                        }
                    }
                }

                // Content area
                if (inContent) {
                    if (state == VIEW_UNIVERSITIES) {
                        for (int i = 0; i < (int)unis.size(); ++i) {
                            float cy = LIST_TOP + i * (CARD_H + CARD_GAP) - scrollOff;
                            if (my >= cy && my < cy + CARD_H) {
                                selIdx = (selIdx == i) ? -1 : i;
                                break;
                            }
                        }
                    }
                    if (state == VIEW_BUSES) {
                        for (int i = 0; i < (int)buses.size(); ++i) {
                            float cy = LIST_TOP + i * (CARD_H + CARD_GAP) - scrollOff;
                            if (my >= cy && my < cy + CARD_H) {
                                selIdx = (selIdx == i) ? -1 : i;
                                break;
                            }
                        }
                    }

                    // ADD_UNIVERSITY form
                    if (state == ADD_UNIVERSITY) {
                        if (uniCodeBox.getBounds().contains({mx, my})) { focusField = 0; }
                        if (uniNameBox.getBounds().contains({mx, my})) { focusField = 1; }
                        if (saveBtn.isClicked(window)) {
                            string err;
                            if (admin.addUniversity(uniCodeBox.getText(), uniNameBox.getText(), err)) {
                                setInfo("University added.", false);
                                state = VIEW_UNIVERSITIES;
                                unis  = admin.getUniversities();
                                selIdx = -1;
                                uniCodeBox.clear(); uniNameBox.clear();
                            } else { setInfo("Error: " + err, true); }
                        }
                        if (cancelBtn.isClicked(window)) {
                            state = VIEW_UNIVERSITIES;
                            unis  = admin.getUniversities();
                            uniCodeBox.clear(); uniNameBox.clear();
                        }
                    }

                    // ADD_BUS form
                    if (state == ADD_BUS) {
                        if (busIdBox.getBounds().contains({mx,my}))    focusField = 0;
                        if (busNameBox.getBounds().contains({mx,my}))  focusField = 1;
                        if (busUniBox.getBounds().contains({mx,my}))   focusField = 2;
                        if (busSeatsBox.getBounds().contains({mx,my})) focusField = 3;
                        if (busRouteBox.getBounds().contains({mx,my})) focusField = 4;

                        if (saveBusBtn.isClicked(window)) {
                            string err;
                            int seats = 0;
                            try { seats = stoi(busSeatsBox.getText()); } catch(...) { seats = -1; }
                            if (admin.addBus(busIdBox.getText(), busNameBox.getText(),
                                            busUniBox.getText(), seats,
                                            busRouteBox.getText(), err)) {
                                setInfo("Bus added.", false);
                                state  = VIEW_BUSES;
                                buses  = admin.getBuses();
                                selIdx = -1;
                                busIdBox.clear(); busNameBox.clear(); busUniBox.clear();
                                busSeatsBox.clear(); busRouteBox.clear();
                            } else { setInfo("Error: " + err, true); }
                        }
                        if (cancelBusBtn.isClicked(window)) {
                            state = VIEW_BUSES;
                            buses = admin.getBuses();
                            busIdBox.clear(); busNameBox.clear(); busUniBox.clear();
                            busSeatsBox.clear(); busRouteBox.clear();
                        }
                    }
                }
            } // MouseButtonPressed

            // Mouse wheel scroll
            if (const auto* mw = event->getIf<sf::Event::MouseWheelScrolled>()) {
                if (inContent && (state == VIEW_UNIVERSITIES || state == VIEW_BUSES)) {
                    scrollOff -= mw->delta * 36.f;
                    scrollOff  = std::clamp(scrollOff, 0.f, maxScroll);
                }
            }

            // Text input routing
            if (event->is<sf::Event::TextEntered>()) {
                if (state == ADD_UNIVERSITY) {
                    if (focusField == 0) uniCodeBox.handleEvent(*event);
                    else                 uniNameBox.handleEvent(*event);
                }
                if (state == ADD_BUS) {
                    switch (focusField) {
                        case 0: busIdBox.handleEvent(*event);    break;
                        case 1: busNameBox.handleEvent(*event);  break;
                        case 2: busUniBox.handleEvent(*event);   break;
                        case 3: busSeatsBox.handleEvent(*event); break;
                        case 4: busRouteBox.handleEvent(*event); break;
                    }
                }
            }
        } // pollEvent

        // Update toolbar buttons
        addBtn.update(window);
        delBtn.update(window);
        if (state == ADD_UNIVERSITY) { saveBtn.update(window); cancelBtn.update(window); }
        if (state == ADD_BUS)        { saveBusBtn.update(window); cancelBusBtn.update(window); }

        // ═══════════════════════════════════════════════════════════════
        //  DRAW
        // ═══════════════════════════════════════════════════════════════
        window.clear(Theme::BG_DARK);

        // ── CONTENT AREA ─────────────────────────────────────────────────
        if (state == DASHBOARD) {
            // Stat cards
            auto allUnis  = admin.getUniversities();
            auto allBuses = admin.getBuses();

            float scy = HH + 30.f;
            float scW = (CW - 60.f) * 0.5f;
            float sc1X = SW + 20.f, sc2X = SW + 30.f + scW;

            // ── Universities stat card ────────────────────────────────────
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

            // ── Buses stat card ───────────────────────────────────────────
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

            // Quick action hint
            sf::Text hint(font);
            hint.setString("Use the sidebar to manage Universities and Buses.");
            hint.setCharacterSize(13); hint.setFillColor(Theme::TEXT_MUTED);
            hint.setPosition(Theme::px(SW + 22.f, scy + 132.f));
            window.draw(hint);
        }
        else if (state == VIEW_UNIVERSITIES) {
            float totalH = unis.size() * (CARD_H + CARD_GAP);
            maxScroll = max(0.f, totalH - CH + 40.f);
            scrollOff = clamp(scrollOff, 0.f, maxScroll);

            for (int i = 0; i < (int)unis.size(); ++i) {
                float cy = LIST_TOP + i * (CARD_H + CARD_GAP) - scrollOff;
                if (cy + CARD_H < HH || cy > wh) continue;

                bool sel = (i == selIdx);
                bool hov = inContent && mx >= CARD_X && mx < CARD_X + CARD_W &&
                           my >= cy && my < cy + CARD_H;
                sf::Color bg = sel ? Theme::ITEM_SELECTED
                             : hov ? Theme::ITEM_HOVER : Theme::ITEM_BG;

                Theme::drawCard(window, {CARD_X, cy}, {CARD_W, CARD_H}, bg, 8.f);
                Theme::drawAccentBar(window, CARD_X, cy, CARD_H,
                                     sel ? Theme::ACCENT_HOVER : Theme::ACCENT);

                // Code badge — larger, higher contrast
                Theme::drawBadge(window, font, unis[i].first,
                                 {CARD_X + 14.f, cy + (CARD_H - 28.f) * 0.5f},
                                 Theme::withAlpha(Theme::ACCENT, 90), sf::Color(220, 235, 255));

                // University full name — prominent
                float badgeW = static_cast<float>(unis[i].first.size()) * 8.5f + 32.f;
                sf::Text nm(font); nm.setString(unis[i].second);
                nm.setCharacterSize(16); nm.setFillColor(Theme::TEXT_PRIMARY);
                sf::FloatRect bn = nm.getLocalBounds();
                nm.setPosition(Theme::px(CARD_X + 14.f + badgeW + 14.f,
                                         cy + (CARD_H - bn.size.y) * 0.5f - bn.position.y));
                window.draw(nm);
            }
            if (unis.empty()) {
                Theme::drawCenteredText(window, font, "No universities registered.",
                                        16, Theme::TEXT_MUTED,
                                        {{SW + 20.f, HH + 80.f}, {CW - 40.f, 60.f}});
            }
        }
        else if (state == ADD_UNIVERSITY) {
            // Form card
            Theme::drawCard(window, {SW + 20.f, HH + 20.f}, {CW - 40.f, 440.f},
                            Theme::BG_CARD, 12.f);
            sf::RectangleShape formAccent({CW - 40.f, 4.f});
            formAccent.setPosition({SW + 20.f, HH + 20.f});
            formAccent.setFillColor(Theme::ACCENT); window.draw(formAccent);

            auto drawLbl = [&](const string& s, float x, float y) {
                sf::Text t(font); t.setString(s);
                t.setCharacterSize(12); t.setFillColor(Theme::TEXT_SECONDARY);
                t.setPosition({x, y}); window.draw(t);
            };
            drawLbl("UNIVERSITY CODE", fX, HH + 92.f);
            drawLbl("UNIVERSITY NAME", fX, HH + 184.f);

            uniCodeBox.draw(window);
            uniNameBox.draw(window);
            saveBtn.draw(window);
            cancelBtn.draw(window);
        }
        else if (state == VIEW_BUSES) {
            float totalH = buses.size() * (CARD_H + CARD_GAP);
            maxScroll = max(0.f, totalH - CH + 40.f);
            scrollOff = clamp(scrollOff, 0.f, maxScroll);

            for (int i = 0; i < (int)buses.size(); ++i) {
                float cy = LIST_TOP + i * (CARD_H + CARD_GAP) - scrollOff;
                if (cy + CARD_H < HH || cy > wh) continue;

                bool sel = (i == selIdx);
                bool hov = inContent && mx >= CARD_X && mx < CARD_X + CARD_W &&
                           my >= cy && my < cy + CARD_H;
                sf::Color bg = sel ? Theme::ITEM_SELECTED
                             : hov ? Theme::ITEM_HOVER : Theme::ITEM_BG;

                Theme::drawCard(window, {CARD_X, cy}, {CARD_W, CARD_H}, bg, 8.f);
                Theme::drawAccentBar(window, CARD_X, cy, CARD_H,
                                     sel ? Theme::ACCENT_HOVER : Theme::PURPLE);

                // ID badge — higher contrast, purple tint
                Theme::drawBadge(window, font, buses[i].getBusID(),
                                 {CARD_X + 14.f, cy + 9.f},
                                 Theme::withAlpha(Theme::PURPLE, 90), sf::Color(220, 200, 255));

                // Bus name — primary, larger
                sf::Text nm(font); nm.setString(buses[i].getBusName());
                nm.setCharacterSize(16); nm.setFillColor(Theme::TEXT_PRIMARY);
                nm.setPosition(Theme::px(CARD_X + 14.f, cy + 31.f));
                window.draw(nm);

                // Secondary info — uni code + seat count
                string secondary = buses[i].getUniversityCode()
                                 + "  ·  " + to_string(buses[i].getTotalSeats()) + " seats";
                sf::Text sec(font); sec.setString(secondary);
                sec.setCharacterSize(12); sec.setFillColor(Theme::TEXT_SECONDARY);
                sec.setPosition(Theme::px(CARD_X + 14.f, cy + 51.f));
                window.draw(sec);

                // Truncated route — brighter muted color
                string route = buses[i].getRoute();
                if (route.size() > 72) route = route.substr(0, 72) + "...";
                sf::Text rt(font); rt.setString(route);
                rt.setCharacterSize(12); rt.setFillColor(sf::Color(140, 155, 175));
                rt.setPosition(Theme::px(CARD_X + 14.f, cy + 67.f));
                window.draw(rt);
            }
            if (buses.empty()) {
                Theme::drawCenteredText(window, font, "No buses registered.",
                                        16, Theme::TEXT_MUTED,
                                        {{SW + 20.f, HH + 80.f}, {CW - 40.f, 60.f}});
            }
        }
        else if (state == ADD_BUS) {
            Theme::drawCard(window, {SW + 20.f, HH + 20.f}, {CW - 40.f, 520.f},
                            Theme::BG_CARD, 12.f);
            sf::RectangleShape formAccent2({CW - 40.f, 4.f});
            formAccent2.setPosition({SW + 20.f, HH + 20.f});
            formAccent2.setFillColor(Theme::PURPLE); window.draw(formAccent2);

            auto drawLbl = [&](const string& s, float x, float y) {
                sf::Text t(font); t.setString(s);
                t.setCharacterSize(12); t.setFillColor(Theme::TEXT_SECONDARY);
                t.setPosition({x, y}); window.draw(t);
            };
            drawLbl("BUS ID",                  c1X, HH + 92.f);
            drawLbl("BUS NAME",                c1X, HH + 184.f);
            drawLbl("UNIVERSITY CODE",         c1X, HH + 276.f);
            drawLbl("TOTAL SEATS",             c2X, HH + 92.f);
            drawLbl("ROUTE STOPS (comma-sep)", c2X, HH + 184.f);

            busIdBox.draw(window);
            busNameBox.draw(window);
            busUniBox.draw(window);
            busSeatsBox.draw(window);
            busRouteBox.draw(window);
            saveBusBtn.draw(window);
            cancelBusBtn.draw(window);
        }

        // ── SIDEBAR (drawn on top to clip content overflow) ───────────────
        {
            sf::RectangleShape sbg({SW, wh});
            sbg.setFillColor(Theme::BG_SIDEBAR);
            window.draw(sbg);

            // Right border
            sf::RectangleShape sborder({1.f, wh});
            sborder.setPosition({SW - 1.f, 0.f});
            sborder.setFillColor(Theme::BORDER_IDLE);
            window.draw(sborder);

            // App name
            sf::Text appName(font); appName.setString("Bus Tracker");
            appName.setCharacterSize(18); appName.setFillColor(Theme::ACCENT);
            sf::FloatRect ab = appName.getLocalBounds();
            appName.setOrigin({ab.position.x + ab.size.x * 0.5f, 0.f});
            appName.setPosition(Theme::px(SW * 0.5f, 14.f));
            window.draw(appName);
            sf::Text role(font); role.setString("ADMIN");
            role.setCharacterSize(10); role.setFillColor(Theme::TEXT_MUTED);
            sf::FloatRect rb = role.getLocalBounds();
            role.setOrigin({rb.position.x + rb.size.x * 0.5f, 0.f});
            role.setPosition(Theme::px(SW * 0.5f, 37.f));
            window.draw(role);

            Theme::drawSeparator(window, 0.f, 70.f, SW);

            // Nav items
            for (auto& nav : navItems) {
                bool active = (state == nav.st) ||
                              (nav.st == VIEW_UNIVERSITIES && state == ADD_UNIVERSITY) ||
                              (nav.st == VIEW_BUSES        && state == ADD_BUS);
                bool hov    = mx < SW && my >= nav.y && my < nav.y + 42.f;

                sf::Color itemBg = active ? Theme::SIDEBAR_SELECTED
                                 : hov   ? Theme::SIDEBAR_HOVER
                                 : sf::Color(0,0,0,0);
                if (active || hov) {
                    sf::RectangleShape ib({SW, 42.f});
                    ib.setPosition({0.f, nav.y});
                    ib.setFillColor(itemBg);
                    window.draw(ib);
                }
                if (active)
                    Theme::drawAccentBar(window, 0.f, nav.y, 42.f, Theme::ACCENT);

                sf::Text nt(font); nt.setString(nav.label);
                nt.setCharacterSize(14);
                nt.setFillColor(active ? Theme::TEXT_PRIMARY : Theme::TEXT_SECONDARY);
                sf::FloatRect nb = nt.getLocalBounds();
                nt.setPosition(Theme::px(16.f, nav.y + (42.f - nb.size.y) * 0.5f - nb.position.y));
                window.draw(nt);
            }

            // Logout
            bool logHov = mx < SW && my >= wh - 52.f && my < wh - 12.f;
            if (logHov) {
                sf::RectangleShape lb({SW, 40.f});
                lb.setPosition({0.f, wh - 52.f}); lb.setFillColor(Theme::SIDEBAR_HOVER);
                window.draw(lb);
            }
            sf::Text logT(font); logT.setString("Logout");
            logT.setCharacterSize(14); logT.setFillColor(Theme::DANGER);
            sf::FloatRect ltb = logT.getLocalBounds();
            logT.setPosition({16.f, wh - 52.f + (40.f - ltb.size.y) * 0.5f - ltb.position.y});
            window.draw(logT);
            Theme::drawSeparator(window, 0.f, wh - 56.f, SW);
        }

        // ── HEADER BAR (drawn on top) ─────────────────────────────────────
        {
            sf::RectangleShape hbg({ww - SW, HH});
            hbg.setPosition({SW, 0.f});
            hbg.setFillColor(Theme::BG_HEADER);
            window.draw(hbg);

            sf::RectangleShape hline({ww - SW, 1.5f});
            hline.setPosition({SW, HH - 1.5f});
            hline.setFillColor(Theme::ACCENT);
            window.draw(hline);

            string pageTitle = "Dashboard";
            if (state == VIEW_UNIVERSITIES || state == ADD_UNIVERSITY) pageTitle = "Universities";
            else if (state == VIEW_BUSES   || state == ADD_BUS)        pageTitle = "Buses";

            sf::Text pt(font); pt.setString(pageTitle);
            pt.setCharacterSize(20); pt.setFillColor(Theme::TEXT_PRIMARY);
            sf::FloatRect pb = pt.getLocalBounds();
            pt.setPosition(Theme::px(SW + 22.f, (HH - pb.size.y) * 0.5f - pb.position.y));
            window.draw(pt);

            // Toolbar buttons (only in list views)
            if (state == VIEW_UNIVERSITIES || state == VIEW_BUSES ||
                state == ADD_UNIVERSITY    || state == ADD_BUS) {
                addBtn.draw(window);
                if (state == VIEW_UNIVERSITIES || state == VIEW_BUSES)
                    delBtn.draw(window);
            }
        }

        // ── Info toast ────────────────────────────────────────────────────
        if (showInfo) {
            float elapsed = infoTimer.getElapsedTime().asSeconds();
            float alpha   = elapsed > 2.f ? 1.f - (elapsed - 2.f) : 1.f;
            Theme::drawInfoToast(window, font, infoText, infoErr, ww, wh, alpha);
        }

        window.display();
    }
}