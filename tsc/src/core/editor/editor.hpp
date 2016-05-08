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
    class cEditor_Menu_Entry {
    public:
        cEditor_Menu_Entry(std::string name);
        ~cEditor_Menu_Entry();

        void Add_Image_Item(std::string pixmap_path, const cImage_Settings_Data& settings);
        void Activate(CEGUI::TabControl* p_tabcontrol);

        inline void Set_Color(Color color){ m_color = color; }
        inline Color Get_Color(){ return m_color; }

        inline std::string Get_Name(){ return m_name; }

        inline void Set_Header(bool is_header){ m_is_header = is_header; }
        inline bool Is_Header() { return m_is_header; }

        inline void Set_Required_Tags(std::vector<std::string> tags) { m_required_tags = tags; }
        inline std::vector<std::string>& Get_Required_Tags() { return m_required_tags; }

        inline CEGUI::ScrollablePane* Get_CEGUI_Pane() { return mp_tab_pane; }

    private:
        std::string m_name;
        Color m_color;
        bool m_is_header;
        std::vector<std::string> m_required_tags;
        CEGUI::ScrollablePane* mp_tab_pane;
        int m_element_y;
    };

    class cEditor {
    public:
        cEditor();
        virtual ~cEditor(void);

        virtual void Init(void);
        virtual void Unload(void);

        void Toggle(void);
        virtual void Enable(void);
        virtual void Disable(void);

        bool Try_Add_Editor_Item(boost::filesystem::path pixmap_path);

        virtual void Update(void);
        virtual void Draw(void);

        virtual bool Handle_Event(const sf::Event& evt);

    protected:
        std::string m_editor_item_tag;
        boost::filesystem::path m_menu_filename;

    private:
        CEGUI::TabControl* mp_editor_tabpane;
        CEGUI::Listbox* mp_menu_listbox;
        bool m_enabled;
        float m_visibility_timer;
        CEGUI::UDim m_target_x_position;
        bool m_rested;
        bool m_mouse_inside;
        std::vector<CEGUI::Window*> m_editor_items;
        std::vector<cEditor_Menu_Entry*> m_menu_entries;

        void parse_menu_file();
        void populate_menu();
        void load_image_items();
        cEditor_Menu_Entry* get_menu_entry(const std::string& name);
        std::vector<cEditor_Menu_Entry*> find_target_menu_entries_for(const cImage_Settings_Data& settings);
        bool on_mouse_enter(const CEGUI::EventArgs& event);
        bool on_mouse_leave(const CEGUI::EventArgs& event);
        bool on_menu_selection_changed(const CEGUI::EventArgs& event);
    };
}

#endif
#endif
