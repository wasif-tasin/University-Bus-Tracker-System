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
    if (!Theme::loadUIFont(font)) return;

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

        // Card geometry for list — taller than before so the larger type has
        // room; the grid itself is unchanged. University rows are a single
        // line of content, so they get their own (shorter) height.
        const float CARD_H  = 104.f;
        const float UCARD_H = 76.f;
        const float CARD_GAP= 12.f;
        const float CARD_X  = SW + 20.f;
        const float CARD_W  = CW - 40.f;
        const float LIST_TOP= HH + 20.f;

        // ── Events ───────────────────────────────────────────────────────
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>()) window.close();
            Theme::syncViewToWindow(window, *event);
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
                            float cy = LIST_TOP + i * (UCARD_H + CARD_GAP) - scrollOff;
                            if (my >= cy && my < cy + UCARD_H) {
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

            // Text input routing (keys included, for caret movement)
            if (event->is<sf::Event::TextEntered>() ||
                event->is<sf::Event::KeyPressed>()  ||
                event->is<sf::Event::MouseButtonPressed>()) {
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

            // Stat card: big bold number, generous gap down to its label
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

            // Quick action hint
            Theme::drawText(window, font,
                            "Use the sidebar to manage Universities and Buses.",
                            Theme::Type::META, Theme::TEXT_SECONDARY,
                            {SW + 22.f, scy + 148.f});
        }
        else if (state == VIEW_UNIVERSITIES) {
            float totalH = unis.size() * (UCARD_H + CARD_GAP);
            maxScroll = max(0.f, totalH - CH + 40.f);
            scrollOff = clamp(scrollOff, 0.f, maxScroll);

            const float badgeH = Theme::badgeHeight(font, Theme::Type::BADGE_UNI);

            for (int i = 0; i < (int)unis.size(); ++i) {
                float cy = LIST_TOP + i * (UCARD_H + CARD_GAP) - scrollOff;
                if (cy + UCARD_H < HH || cy > wh) continue;

                bool sel = (i == selIdx);
                bool hov = inContent && mx >= CARD_X && mx < CARD_X + CARD_W &&
                           my >= cy && my < cy + UCARD_H;
                sf::Color bg = sel ? Theme::ITEM_SELECTED
                             : hov ? Theme::ITEM_HOVER : Theme::ITEM_BG;

                Theme::drawCard(window, {CARD_X, cy}, {CARD_W, UCARD_H}, bg, 8.f);
                Theme::drawAccentBar(window, CARD_X, cy, UCARD_H,
                                     sel ? Theme::ACCENT_HOVER : Theme::ACCENT);

                // Short title chip — bold, centred, vertically centred in card
                float badgeW = Theme::drawBadge(
                    window, font, unis[i].first,
                    {CARD_X + 16.f, std::round(cy + (UCARD_H - badgeH) * 0.5f)},
                    Theme::BADGE_UNI_BG, Theme::BADGE_UNI_TEXT,
                    Theme::Type::BADGE_UNI, Theme::BADGE_UNI_EDGE);

                // University full name — measured off the chip, not estimated
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
        else if (state == ADD_UNIVERSITY) {
            // Form card
            Theme::drawCard(window, {SW + 20.f, HH + 20.f}, {CW - 40.f, 440.f},
                            Theme::BG_CARD, 12.f);
            sf::RectangleShape formAccent({CW - 40.f, 4.f});
            formAccent.setPosition({SW + 20.f, HH + 20.f});
            formAccent.setFillColor(Theme::ACCENT); window.draw(formAccent);

            auto drawLbl = [&](const string& s, float x, float y) {
                Theme::drawText(window, font, s, Theme::Type::LABEL,
                                Theme::TEXT_MUTED, {x, y}, sf::Text::Bold);
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

            const float badgeH = Theme::badgeHeight(font, Theme::Type::BADGE_BUS);

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

                const float padL = 16.f;
                const float rowY = cy + 14.f;

                // Bus ID chip — bold, high contrast
                float badgeW = Theme::drawBadge(window, font, buses[i].getBusID(),
                                                {CARD_X + padL, rowY},
                                                Theme::BADGE_BUS_BG, Theme::BADGE_BUS_TEXT,
                                                Theme::Type::BADGE_BUS, Theme::BADGE_BUS_EDGE);

                // Bus name — largest and brightest element on the card
                Theme::drawTextVCentered(window, font, buses[i].getBusName(),
                                         Theme::Type::BUS_NAME, Theme::TEXT_PRIMARY,
                                         CARD_X + padL + badgeW + 16.f,
                                         rowY, badgeH, sf::Text::Bold);

                // University + seat count
                string secondary = buses[i].getUniversityCode()
                                 + "  ·  " + to_string(buses[i].getTotalSeats()) + " seats";
                const float metaY = rowY + badgeH + 12.f;
                Theme::drawText(window, font, secondary, Theme::Type::META,
                                Theme::TEXT_SECONDARY, {CARD_X + padL, metaY});

                // Route — trimmed to the measured width rather than a fixed
                // character count, so it never clips mid-glyph
                string route = Theme::ellipsize(font, buses[i].getRoute(),
                                                Theme::Type::ROUTE, CARD_W - padL - 24.f);
                Theme::drawText(window, font, route, Theme::Type::ROUTE,
                                Theme::TEXT_ROUTE, {CARD_X + padL, metaY + 24.f});
            }
            if (buses.empty()) {
                Theme::drawCenteredText(window, font, "No buses registered.",
                                        Theme::Type::BODY, Theme::TEXT_MUTED,
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
                Theme::drawText(window, font, s, Theme::Type::LABEL,
                                Theme::TEXT_MUTED, {x, y}, sf::Text::Bold);
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
            Theme::drawTextHCentered(window, font, "Bus Tracker", Theme::Type::SUBTITLE,
                                     Theme::ACCENT, SW * 0.5f, 14.f, sf::Text::Bold);
            Theme::drawTextHCentered(window, font, "ADMIN", Theme::Type::CAPTION,
                                     Theme::TEXT_MUTED, SW * 0.5f, 38.f, sf::Text::Bold);

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

                // Weight, not just colour, carries the active state
                Theme::drawTextVCentered(window, font, nav.label, Theme::Type::META,
                                         active ? Theme::TEXT_PRIMARY : Theme::TEXT_SECONDARY,
                                         18.f, nav.y, 42.f,
                                         active ? sf::Text::Bold : sf::Text::Regular);
            }

            // Logout
            bool logHov = mx < SW && my >= wh - 52.f && my < wh - 12.f;
            if (logHov) {
                sf::RectangleShape lb({SW, 40.f});
                lb.setPosition({0.f, wh - 52.f}); lb.setFillColor(Theme::SIDEBAR_HOVER);
                window.draw(lb);
            }
            Theme::drawTextVCentered(window, font, "Logout", Theme::Type::META,
                                     Theme::DANGER_HOVER, 18.f, wh - 52.f, 40.f,
                                     sf::Text::Bold);
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

            Theme::drawTextVCentered(window, font, pageTitle, Theme::Type::HEADING,
                                     Theme::TEXT_PRIMARY, SW + 24.f, 0.f, HH,
                                     sf::Text::Bold);

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