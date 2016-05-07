/***************************************************************************
 * editor.hpp
 *
 * Copyright © 2006 - 2011 Florian Richter
 * Copyright © 2013 - 2016 The TSC Contributors
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef TSC_EDITOR_HPP
#define TSC_EDITOR_HPP
#ifdef ENABLE_NEW_EDITOR

namespace TSC {
    class cEditor {
    public:
        cEditor();
        virtual ~cEditor(void);

        virtual void Init(void);
        virtual void Unload(void);

        void Toggle(void);
        virtual void Enable(void);
        virtual void Disable(void);

        void Add_Editor_Item(boost::filesystem::path pixmap_path);

        virtual void Update(void);
        virtual void Draw(void);

        virtual bool Handle_Event(const sf::Event& evt);

    private:
        CEGUI::Window* mp_editor_tabpane;
        bool m_enabled;
        float m_visibility_timer;
        CEGUI::UDim m_target_x_position;
        bool m_rested;
        bool m_mouse_inside;
        std::vector<CEGUI::Window*> m_editor_items;
        float m_element_y;

        bool on_mouse_enter(const CEGUI::EventArgs& event);
        bool on_mouse_leave(const CEGUI::EventArgs& event);
    };
}

#endif
#endif
