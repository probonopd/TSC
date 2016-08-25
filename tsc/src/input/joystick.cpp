/***************************************************************************
 * joystick.cpp  -  Joystick handling class
 *
 * Copyright © 2003 - 2011 Florian Richter
 * Copyright © 2013 - 2014 The TSC Contributors
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../core/global_basic.hpp"
#include "../input/keyboard.hpp"
#include "../input/joystick.hpp"
#include "../user/preferences.hpp"
#include "../core/game_core.hpp"
#include "../level/level_player.hpp"
#include "../gui/hud.hpp"
#include "../gui/menu.hpp"
#include "../level/level.hpp"
#include "../overworld/overworld.hpp"

using namespace std;

namespace TSC {

const float cJoystick::m_joystick_neutral_bound = 20.0f;

/* *** *** *** *** *** *** cJoystick *** *** *** *** *** *** *** *** *** *** *** */

cJoystick::cJoystick(void)
{
    m_current_joystick = 999; // SFML only supports 8, so this is guaranteed to be invalid input to SFML
    m_num_joysticks = 0;
    m_num_buttons = 0;

    m_debug = 0;

    Reset_keys();

    Init();
}

cJoystick::~cJoystick(void)
{
    Close();
}

int cJoystick::Init(void)
{
    // if not enabled
    if (!pPreferences->m_joy_enabled) {
        return 0;
    }

    // Ensure all joysticks are found now
    sf::Joystick::update();

    for(int i=0; i < sf::Joystick::Count; i++) {
        if (sf::Joystick::isConnected(i)) {
            m_num_joysticks++;
        }
    }

    // no joystick available
    if (m_num_joysticks == 0) {
        cout << "No joysticks available" << endl;
        pPreferences->m_joy_enabled = 0;
        return 0;
    }

    if (m_debug) {
        cout << "Joysticks found : " << m_num_joysticks << endl << endl;
    }

    unsigned int default_joy = 0;

    // if default joy name is given
    if (!pPreferences->m_joy_name.empty()) {
        vector<std::string> joy_names = Get_Names();

        for (unsigned int i = 0; i < joy_names.size(); i++) {
            // found default joy
            if (joy_names[i].compare(pPreferences->m_joy_name) == 0) {
                default_joy = i;
                break;
            }
        }
    }

    // Apply preferences threshold
    pVideo->mp_window->setJoystickThreshold(pPreferences->m_joy_axis_threshold);

    // setup
    Stick_Open(default_joy);

    verticalAxes[0] = pPreferences->m_joy_axis_ver;
    verticalAxes[1] = static_cast<sf::Joystick::Axis>(7);

    horizontalAxes[0] = pPreferences->m_joy_axis_hor;
    horizontalAxes[1] = static_cast<sf::Joystick::Axis>(6);

    if (m_debug) {
        cout << "Joypad System Initialized" << endl;
    }

    return 1;
}

void cJoystick::Close(void)
{
    Stick_Close();
}

void cJoystick::Update(void)
{
    /*
     * It is necessary to examine the joystick state and simulate keyboard
     * input continually in order to allow other portions of the TSC code
     * that only look for keyboard input to function correctly.  Making
     * joystick input be interpreted as keyboard input is not the best
     * architectural design, but we may have to live with it.
     */
    sf::Event newevt;
    if (m_up) {
        newevt.type = sf::Event::KeyPressed;
        newevt.key.code = pPreferences->m_key_up;
        pKeyboard->Key_Down(newevt);
    }
    if (m_down) {
        newevt.type = sf::Event::KeyPressed;
        newevt.key.code = pPreferences->m_key_down;
        pKeyboard->Key_Down(newevt);
    }
    if (m_left) {
        newevt.type = sf::Event::KeyPressed;
        newevt.key.code = pPreferences->m_key_left;
        pKeyboard->Key_Down(newevt);
    }
    if (m_right) {
        newevt.type = sf::Event::KeyPressed;
        newevt.key.code = pPreferences->m_key_right;
        pKeyboard->Key_Down(newevt);
    }
}

bool cJoystick::Stick_Open(unsigned int index)
{
    m_num_buttons = sf::Joystick::getButtonCount(index);

    if (m_num_buttons == 0) {
        cout << "Failed to retrieve button count from joystick " << index << ". Disconnected?" << endl;
        return false;
    }

    m_current_joystick = index;
    if (m_debug) {
        cout << "Switched to Joystick " << m_current_joystick << endl;
        cout << "Name: " << Get_Name() << endl;
        cout << "Number of Buttons: " << m_num_buttons << endl;
    }

    return true;
}

void cJoystick::Stick_Close(void)
{
    Reset_keys();

    m_num_buttons = 0;
    m_current_joystick = 999;

    if (m_debug) {
        cout << "Joystick " << m_current_joystick << " disabled." << endl;
    }
}

void cJoystick::Reset_keys(void)
{
    for (int i = 0; i < NUM_AXIS_TYPES; i++) {
        m_is_axis_left[i] = false;
        m_is_axis_right[i] = false;
        m_is_axis_down[i] = false;
        m_is_axis_up[i] = false;
    }

    m_left = false;
    m_right = false;
    m_up = false;
    m_down = false;
}

void cJoystick::Handle_Motion(const sf::Event& evt)
{
    sf::Event newevt;

    if (evt.joystickMove.joystickId != m_current_joystick)
        return;

    for (int i = 0; i < NUM_AXIS_TYPES; i++) {
    // Vertical Axis
    if (evt.joystickMove.axis == verticalAxes[i]) {

        // Up
        if (evt.joystickMove.position < -m_joystick_neutral_bound) {
            if (m_debug) {
                //cout << "Joystick " << m_current_joystick << " : Up Button pressed" << endl;
            }

            if (!m_is_axis_up[i]) {
                newevt.type = sf::Event::KeyPressed;
                newevt.key.code = pPreferences->m_key_up;
                //pKeyboard->Key_Down(newevt);
                m_is_axis_up[i] = true;
            }

            if (m_is_axis_down[i]) {
                newevt.type = sf::Event::KeyReleased;
                newevt.key.code = pPreferences->m_key_down;
                //pKeyboard->Key_Up(newevt);
                m_is_axis_down[i] = 0;
            }
        }
        // Down
        else if (evt.joystickMove.position > m_joystick_neutral_bound) {
            if (m_debug) {
                cout << "Joystick " << m_current_joystick << " : Down Button pressed" << endl;
            }

            if (!m_is_axis_down[i]) {
                newevt.type = sf::Event::KeyPressed;
                newevt.key.code = pPreferences->m_key_down;
                //pKeyboard->Key_Down(newevt);
                m_is_axis_down[i] = true;
            }

            if (m_is_axis_up[i]) {
                newevt.type = sf::Event::KeyReleased;
                newevt.key.code = pPreferences->m_key_up;
                //pKeyboard->Key_Up(newevt);
                m_is_axis_up[i] = false;
            }
        }
        // No Down/Left
        else {
            //cout << "No up or down - clearing out vertical values" << endl;
            if (m_is_axis_down[i]) {
                newevt.type = sf::Event::KeyReleased;
                newevt.key.code = pPreferences->m_key_down;
                //pKeyboard->Key_Up(newevt);
                m_is_axis_down[i] = false;
            }

            if (m_is_axis_up[i]) {
                newevt.type = sf::Event::KeyReleased;
                newevt.key.code = pPreferences->m_key_up;
                //pKeyboard->Key_Up(newevt);
                m_is_axis_up[i] = false;
            }
        }
    }
    // Horizontal Axis
    else if (evt.joystickMove.axis == horizontalAxes[i]) {

        //if (evt.joystickMove.axis == 6)
        //    cout << "Horizontal movement of: " << evt.joystickMove.position << endl;

        // Left
        if (evt.joystickMove.position < -m_joystick_neutral_bound) {
            if (m_debug) {
                cout << "Joystick " << m_current_joystick << " : Left Button pressed" << endl;
            }

            if (!m_is_axis_left[i]) {
                newevt.type = sf::Event::KeyPressed;
                newevt.key.code = pPreferences->m_key_left;

                //if (evt.joystickMove.axis == 6)
                //    cout << "Generating a keydown for horizontal movement" << endl;

                //pKeyboard->Key_Down(newevt);
                m_is_axis_left[i] = true;
            }

            if (m_is_axis_right[i]) {
                newevt.type = sf::Event::KeyReleased;
                newevt.key.code = pPreferences->m_key_right;
                //pKeyboard->Key_Up(newevt);
                m_is_axis_right[i] = false;
            }
        }
        // Right
        else if (evt.joystickMove.position > m_joystick_neutral_bound) {
            if (m_debug) {
                cout << "Joystick " << m_current_joystick << " : Right Button pressed" << endl;
            }

            if (!m_is_axis_right[i]) {
                newevt.type = sf::Event::KeyPressed;
                newevt.key.code = pPreferences->m_key_right;

                //if (evt.joystickMove.axis == 6)
                //    cout << "Generating a keydown for horizontal movement" << endl;

                //pKeyboard->Key_Down(newevt);
                m_is_axis_right[i] = true;
            }

            if (m_is_axis_left[i]) {
                newevt.type = sf::Event::KeyReleased;
                newevt.key.code = pPreferences->m_key_left;
                //pKeyboard->Key_Up(newevt);
                m_is_axis_left[i] = 0;
            }
        }
        // No Left/Right
        else {
            //cout << "No left or right - clearing out horizontal values" << endl;
            if (m_is_axis_left[i]) {
                newevt.type = sf::Event::KeyReleased;
                newevt.key.code = pPreferences->m_key_left;
                //pKeyboard->Key_Up(newevt);
                m_is_axis_left[i] = 0;
            }

            if (m_is_axis_right[i]) {
                newevt.type = sf::Event::KeyReleased;
                newevt.key.code = pPreferences->m_key_right;
                //pKeyboard->Key_Up(newevt);
                m_is_axis_right[i] = 0;
            }
        }
    }
    else
    {
        //if (evt.joystickMove.axis == 6 || evt.joystickMove.axis == 7)
        //{
        //if (evt.joystickMove.position < -m_joystick_neutral_bound || evt.joystickMove.position < -m_joystick_neutral_bound)
        //{
            //cout << "New axis, axis value of: " << evt.joystickMove.axis << endl;
            //cout << "New axis, position value of: " << evt.joystickMove.position << endl;
        //}
        //}

    }
    } //for



    bool wasRight = m_right;
    bool wasLeft = m_left;
    bool wasUp = m_up;
    bool wasDown = m_down;

    m_right = m_left = m_up = m_down = false;

    //Go backwards so as to give the axis number listed first the highest priority
    for (int i = NUM_AXIS_TYPES - 1; i >= 0; i--)
    {
        if (m_is_axis_right[i])
        {
            m_right = true;
            m_left = false;
        }
        else if (m_is_axis_left[i])
        {
            m_right = false;
            m_left = true;
        }

        if (m_is_axis_up[i])
        {
            m_up = true;
            m_down = false;
        }
        else if (m_is_axis_down[i])
        {
            m_up = false;
            m_down = true;
        }
    }

    if (m_right != wasRight)
    {
        newevt.type = sf::Event::KeyPressed;
        newevt.key.code = pPreferences->m_key_right;
        if (m_right)
        {
            pKeyboard->Key_Down(newevt);
        }
        else
        {
            pKeyboard->Key_Up(newevt);
        }
    }

    if (m_left != wasLeft)
    {
        newevt.type = sf::Event::KeyPressed;
        newevt.key.code = pPreferences->m_key_left;
        if (m_left)
        {
            pKeyboard->Key_Down(newevt);
        }
        else
        {
            pKeyboard->Key_Up(newevt);
        }
    }

    if (m_up != wasUp)
    {
        newevt.type = sf::Event::KeyPressed;
        newevt.key.code = pPreferences->m_key_up;
        if (m_up)
        {
            pKeyboard->Key_Down(newevt);
        }
        else
        {
            pKeyboard->Key_Up(newevt);
        }
    }

    if (m_down != wasDown)
    {
        newevt.type = sf::Event::KeyPressed;
        newevt.key.code = pPreferences->m_key_down;
        if (m_down)
        {
            pKeyboard->Key_Down(newevt);
        }
        else
        {
            cout << "key up for down" << endl;
            pKeyboard->Key_Up(newevt);
        }
    }

}

bool cJoystick::Handle_Button_Down_Event(const sf::Event& evt)
{
    cout << "Button down: " << evt.joystickButton.button << endl;

    // not enabled or opened
    if (!pPreferences->m_joy_enabled || evt.joystickButton.joystickId != m_current_joystick) {
        return 0;
    }

    // handle button in the current mode
    if (Game_Mode == MODE_LEVEL) {
        // processed by the level
        if (pActive_Level->Joy_Button_Down(evt.joystickButton.button)) {
            return 1;
        }
    }
    else if (Game_Mode == MODE_OVERWORLD) {
        // processed by the overworld
        if (pActive_Overworld->Joy_Button_Down(evt.joystickButton.button)) {
            return 1;
        }
    }
    else if (Game_Mode == MODE_MENU) {
        // processed by the menu
        if (pMenuCore->Joy_Button_Down(evt.joystickButton.button)) {
            return 1;
        }
    }

    // Jump
    if (evt.joystickButton.button == pPreferences->m_joy_button_jump) {
        //
    }
    // Shoot
    else if (evt.joystickButton.button == pPreferences->m_joy_button_shoot) {
        sf::Event newevt;
        newevt.type = sf::Event::KeyPressed;
        newevt.key.code = pPreferences->m_key_shoot;
        pKeyboard->Key_Down(newevt);
        return 1;
    }
    // Request Itembox Item
    else if (evt.joystickButton.button == pPreferences->m_joy_button_item) {
        // not handled
        return 1;
    }
    // Interaction
    else if (evt.joystickButton.button == pPreferences->m_joy_button_action) {
        sf::Event newevt;
        newevt.type = sf::Event::KeyPressed;
        newevt.key.code = pPreferences->m_key_action;
        pKeyboard->Key_Down(newevt);
        return 1;
    }
    // Exit
    else if (evt.joystickButton.button == pPreferences->m_joy_button_exit) {
        sf::Event newevt;
        newevt.type = sf::Event::KeyPressed;
        newevt.key.code = sf::Keyboard::Escape;
        pKeyboard->Key_Down(newevt);
        return 1;
    }
    // Pause
    else if (evt.joystickButton.button == 9) {
        sf::Event newevt;
        newevt.type = sf::Event::KeyPressed;
        newevt.key.code = sf::Keyboard::Pause;
        pKeyboard->Key_Down(newevt);
        return 1;
    }

    return 0;
}

bool cJoystick::Handle_Button_Up_Event(const sf::Event& evt)
{
    // not enabled or opened
    if (!pPreferences->m_joy_enabled || evt.joystickButton.joystickId != m_current_joystick) {
        return 0;
    }

    // handle button in the current mode
    if (Game_Mode == MODE_LEVEL) {
        // processed by the level
        if (pActive_Level->Joy_Button_Up(evt.joystickButton.button)) {
            return 1;
        }
    }
    else if (Game_Mode == MODE_OVERWORLD) {
        // processed by the overworld
        if (pActive_Overworld->Joy_Button_Up(evt.joystickButton.button)) {
            return 1;
        }
    }
    else if (Game_Mode == MODE_MENU) {
        // processed by the menu
        if (pMenuCore->Joy_Button_Up(evt.joystickButton.button)) {
            return 1;
        }
    }

    if (evt.joystickButton.button == pPreferences->m_joy_button_jump) {
        sf::Event newevt;
        newevt.type = sf::Event::KeyReleased;
        newevt.key.code = pPreferences->m_key_jump;
        pKeyboard->Key_Up(newevt);
        return 1;
    }
    else if (evt.joystickButton.button == pPreferences->m_joy_button_shoot) {
        sf::Event newevt;
        newevt.type = sf::Event::KeyReleased;
        newevt.key.code = pPreferences->m_key_shoot;
        pKeyboard->Key_Up(newevt);
        return 1;
    }
    else if (evt.joystickButton.button == pPreferences->m_joy_button_item) {
        // not handled
    }
    else if (evt.joystickButton.button == pPreferences->m_joy_button_action) {
        sf::Event newevt;
        newevt.type = sf::Event::KeyReleased;
        newevt.key.code = pPreferences->m_key_action;
        pKeyboard->Key_Up(newevt);
        return 1;
    }
    else if (evt.joystickButton.button == pPreferences->m_joy_button_exit) {
        sf::Event newevt;
        newevt.type = sf::Event::KeyReleased;
        newevt.key.code = sf::Keyboard::Escape;
        pKeyboard->Key_Up(newevt);
        return 1;
    }

    return 0;
}

std::string cJoystick::Get_Name(void) const
{
    return sf::Joystick::getIdentification(m_current_joystick).name;
}

vector<std::string> cJoystick::Get_Names(void) const
{
    vector<std::string> names;

    // joystick names
    for (unsigned int i = 0; i < m_num_joysticks; i++) {
        names.push_back(sf::Joystick::getIdentification(i).name);
    }

    return names;
}

bool cJoystick::Left(void) const
{
    if (pPreferences->m_joy_enabled && sf::Joystick::getAxisPosition(m_current_joystick, pPreferences->m_joy_axis_hor) < 0) {
        return 1;
    }

    return 0;
}

bool cJoystick::Right(void) const
{
    if (pPreferences->m_joy_enabled && sf::Joystick::getAxisPosition(m_current_joystick, pPreferences->m_joy_axis_hor) > 0) {
        return 1;
    }

    return 0;
}

bool cJoystick::Up(void) const
{
    if (pPreferences->m_joy_enabled && sf::Joystick::getAxisPosition(m_current_joystick, pPreferences->m_joy_axis_ver) < 0) {
        return 1;
    }

    return 0;
}

bool cJoystick::Down(void) const
{
    if (pPreferences->m_joy_enabled && sf::Joystick::getAxisPosition(m_current_joystick, pPreferences->m_joy_axis_ver) > 0) {
        return 1;
    }

    return 0;
}

bool cJoystick::Button(unsigned int num)
{
    if (pPreferences->m_joy_enabled && sf::Joystick::isButtonPressed(m_current_joystick, num)) {
        return 1;
    }

    return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cJoystick* pJoystick = NULL;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace TSC
