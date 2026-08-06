#ifndef ADMINDASHBOARDGUI_H
#define ADMINDASHBOARDGUI_H

#include "Admin.h"
#include "Bus.h"
#include "Screen.h"
#include "TextBox.h"

#include <SFML/Graphics.hpp>
#include <string>
#include <utility>
#include <vector>

class AdminDashboardScreen : public Screen
{
public:
    explicit AdminDashboardScreen(sf::Font& font);

    std::string title() const override { return "University Bus Tracker - Admin Dashboard"; }

    void onEnter() override;
    void prepare(sf::Vector2f size, sf::Vector2f mouse) override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void draw(sf::RenderTarget& target) override;
    void skipAnimations() override;

private:
    enum State { DASHBOARD, VIEW_UNIVERSITIES, ADD_UNIVERSITY, VIEW_BUSES, ADD_BUS };

    void setInfo(const std::string& msg, bool err);
    void saveUniversity();
    void cancelUniversity();
    void saveBus();
    void cancelBus();
    void goTo(State next);

    void drawSidebar(sf::RenderTarget& target);
    void drawHeader(sf::RenderTarget& target);

    sf::Font& m_font;
    Admin     m_admin;

    State m_state     = DASHBOARD;
    int   m_selIdx    = -1;
    float m_scrollOff = 0.f;
    float m_maxScroll = 0.f;
    int   m_focusField = 0;

    std::vector<std::pair<std::string, std::string>> m_unis;
    std::vector<Bus>                                 m_buses;

    std::string m_infoText;
    bool        m_infoErr  = false;
    bool        m_showInfo = false;
    float       m_infoAge  = 0.f;

    TextBox m_uniCodeBox, m_uniNameBox;
    TextBox m_busIdBox, m_busNameBox, m_busUniBox, m_busSeatsBox, m_busRouteBox;

    float m_addHoverT = 0.f, m_delHoverT = 0.f;
    float m_saveHoverT = 0.f, m_cancelHoverT = 0.f;

    float m_contentW = 0.f, m_contentH = 0.f;
    float m_fX = 0.f, m_fW = 0.f, m_fW2 = 0.f, m_c1X = 0.f, m_c2X = 0.f;
};

#endif
