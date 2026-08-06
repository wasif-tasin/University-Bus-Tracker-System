#ifndef USERDASHBOARDGUI_H
#define USERDASHBOARDGUI_H

#include "Bus.h"
#include "Screen.h"
#include "TextBox.h"
#include "User.h"

#include <SFML/Graphics.hpp>
#include <string>
#include <utility>
#include <vector>

class UserDashboardScreen : public Screen
{
public:
    explicit UserDashboardScreen(sf::Font& font);

    std::string title() const override { return "University Bus Tracker - User Dashboard"; }

    void onEnter() override;
    void prepare(sf::Vector2f size, sf::Vector2f mouse) override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void draw(sf::RenderTarget& target) override;
    void skipAnimations() override;

private:
    enum State {
        USER_DASHBOARD,
        USER_VIEW_UNIVERSITIES,
        USER_SELECT_UNIVERSITY,
        USER_VIEW_BUSES,
        USER_SEARCH_BUS,
        USER_SEARCH_BY_STOP
    };

    void setInfo(const std::string& msg, bool err);
    void runSearch();
    void goTo(State next);

    void drawBusCard(sf::RenderTarget& target, const Bus& b,
                     float cx, float cy, float cardW, bool hov, bool includeUni);
    void drawSidebar(sf::RenderTarget& target);
    void drawHeader(sf::RenderTarget& target);

    sf::Font& m_font;
    User      m_user;

    State m_state     = USER_DASHBOARD;
    float m_scrollOff = 0.f;
    float m_maxScroll = 0.f;

    std::vector<std::pair<std::string, std::string>> m_unis;
    std::vector<Bus>                                 m_buses;

    std::string m_infoText;
    bool        m_infoErr  = false;
    bool        m_showInfo = false;
    float       m_infoAge  = 0.f;

    TextBox m_uniCodeBox, m_busIdBox, m_stopBox;

    float m_searchHoverT = 0.f;
    State m_shownState = USER_DASHBOARD;
    float m_stateT     = 1.f;
    float m_resultsT   = 1.f;

    float m_navT[5] = {0.f, 0.f, 0.f, 0.f, 0.f};
    float m_logoutT = 0.f;

    float m_contentW = 0.f, m_contentH = 0.f;
    float m_fX = 0.f, m_srchW = 0.f, m_srchBtnX = 0.f;
};

#endif
