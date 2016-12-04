/***************************************************************************
 * hud.h
 *
 * Copyright © 2016 The TSC Contributors
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef TSC_GAME_CONSOLE_HPP
#define TSC_GAME_CONSOLE_HPP

namespace TSC {

    class cGame_Console
    {
    public:
        cGame_Console();
        ~cGame_Console();

        void Show();
        void Hide();
        void Toggle();
        void Update();

        void Clear();
        void Append_Line(std::string line);
    private:
        CEGUI::Window* mp_console_root;
    };

    extern cGame_Console* gp_game_console;
}

#endif /* TSC_GAME_CONSOLE_HPP */
