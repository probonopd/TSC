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
    m_left = 0;
    m_right = 0;
    m_up = 0;
    m_down = 0;
}

void cJoystick::Handle_Motion(const sf::Event& evt)
{
    sf::Event newevt;

    if (evt.joystickMove.joystickId != m_current_joystick)
        return;

    // Vertical Axis
    if (evt.joystickMove.axis == pPreferences->m_joy_axis_ver) {
        // Up
        if (evt.joystickMove.position < -m_joystick_neutral_bound) {
            if (m_debug) {
                cout << "Joystick " << m_current_joystick << " : Up Button pressed" << endl;
            }

            if (!m_up) {
                newevt.type = sf::Event::KeyPressed;
                newevt.key.code = pPreferences->m_key_up;
                pKeyboard->Key_Down(newevt);
                m_up = 1;
            }

            if (m_down) {
                newevt.type = sf::Event::KeyReleased;
                newevt.key.code = pPreferences->m_key_down;
                pKeyboard->Key_Up(newevt);
                m_down = 0;
            }
        }
        // Down
        else if (evt.joystickMove.position > m_joystick_neutral_bound) {
            if (m_debug) {
                cout << "Joystick " << m_current_joystick << " : Down Button pressed" << endl;
            }

            if (!m_down) {
                newevt.type = sf::Event::KeyPressed;
                newevt.key.code = pPreferences->m_key_down;
                pKeyboard->Key_Down(newevt);
                m_down = 1;
            }

            if (m_up) {
                newevt.type = sf::Event::KeyReleased;
                newevt.key.code = pPreferences->m_key_up;
                pKeyboard->Key_Up(newevt);
                m_up = 0;
            }
        }
        // No Down/Left
        else {
            if (m_down) {
                newevt.type = sf::Event::KeyReleased;
                newevt.key.code = pPreferences->m_key_down;
                pKeyboard->Key_Up(newevt);
                m_down = 0;
            }

            if (m_up) {
                newevt.type = sf::Event::KeyReleased;
                newevt.key.code = pPreferences->m_key_up;
                pKeyboard->Key_Up(newevt);
                m_up = 0;
            }
        }
    }
    // Horizontal Axis
    else if (evt.joystickMove.axis == pPreferences->m_joy_axis_hor) {
        // Left
        if (evt.joystickMove.position < -m_joystick_neutral_bound) {
            if (m_debug) {
                cout << "Joystick " << m_current_joystick << " : Left Button pressed" << endl;
            }

            if (!m_left) {
                newevt.type = sf::Event::KeyPressed;
                newevt.key.code = pPreferences->m_key_left;
                pKeyboard->Key_Down(newevt);
                m_left = 1;
            }

            if (m_right) {
                newevt.type = sf::Event::KeyReleased;
                newevt.key.code = pPreferences->m_key_right;
                pKeyboard->Key_Up(newevt);
                m_right = 0;
            }
        }
        // Right
        else if (evt.joystickMove.position > m_joystick_neutral_bound) {
            if (m_debug) {
                cout << "Joystick " << m_current_joystick << " : Right Button pressed" << endl;
            }

            if (!m_right) {
                newevt.type = sf::Event::KeyPressed;
                newevt.key.code = pPreferences->m_key_right;
                pKeyboard->Key_Down(newevt);
                m_right = 1;
            }

            if (m_left) {
                newevt.type = sf::Event::KeyReleased;
                newevt.key.code = pPreferences->m_key_left;
                pKeyboard->Key_Up(newevt);
                m_left = 0;
            }
        }
        // No Left/Right
        else {
            if (m_left) {
                newevt.type = sf::Event::KeyReleased;
                newevt.key.code = pPreferences->m_key_left;
                pKeyboard->Key_Up(newevt);
                m_left = 0;
            }

            if (m_right) {
                newevt.type = sf::Event::KeyReleased;
                newevt.key.code = pPreferences->m_key_right;
                pKeyboard->Key_Up(newevt);
                m_right = 0;
            }
        }
    }
}

bool cJoystick::Handle_Button_Down_Event(const sf::Event& evt)
{
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
