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
        bool IsVisible() const;
        void Update();

        void Reset();
        void Append_Text(CEGUI::String text);
        void Append_Text(std::string text);
    private:
        CEGUI::Window* mp_console_root;
        CEGUI::Editbox* mp_input_edit;
        CEGUI::MultiLineEditbox* mp_output_edit;
        CEGUI::Window* mp_lino_text;
        unsigned long m_lino;

        void print_preamble();
        bool on_input_accepted(const CEGUI::EventArgs& evt);
    };

    extern cGame_Console* gp_game_console;
}

#endif /* TSC_GAME_CONSOLE_HPP */
