#include "../global_basic.hpp"
#include "../game_core.hpp"
#include "../../gui/generic.hpp"
#include "../framerate.hpp"
#include "../../audio/audio.hpp"
#include "../../video/font.hpp"
#include "../../video/animation.hpp"
#include "../../input/keyboard.hpp"
#include "../../input/mouse.hpp"
#include "../../input/joystick.hpp"
#include "../../user/preferences.hpp"
#include "../../level/level.hpp"
#include "../../level/level_player.hpp"
#include "../../overworld/world_manager.hpp"
#include "../../video/renderer.hpp"
#include "../sprite_manager.hpp"
#include "../../overworld/overworld.hpp"
#include "../i18n.hpp"
#include "../filesystem/filesystem.hpp"
#include "../filesystem/relative.hpp"
#include "../filesystem/resource_manager.hpp"
#include "../../video/img_settings.hpp"
#include "../errors.hpp"
#include "editor.hpp"

#ifdef ENABLE_NEW_EDITOR

using namespace TSC;

cEditor::cEditor()
{
    mp_editor_tabpane = NULL;
    mp_menu_listbox = NULL;
    m_enabled = false;
    m_rested = false;
    m_visibility_timer = 0.0f;
    m_mouse_inside = false;
    m_menu_filename = boost::filesystem::path(path_to_utf8("Needs to be set by subclasses"));
    m_editor_item_tag = "Must be set by subclass";
    m_help_window_visible = false;
    mp_sprite_manager = NULL;
}

cEditor::~cEditor()
{
    Unload();
}

/**
 * Load the CEGUI layout from disk and attach it to the root window.
 * Does not show it, use Enable() for that.
 *
 * Override in subclasses to fill the editor pane with your custom
 * items. Be sure to call this parent method before doing so, though.
 */
void cEditor::Init(void)
{
    mp_editor_tabpane = static_cast<CEGUI::TabControl*>(CEGUI::WindowManager::getSingleton().loadLayoutFromFile(m_editor_item_tag + "_editor.layout"));
    m_target_x_position = mp_editor_tabpane->getXPosition();
    mp_editor_tabpane->hide(); // Do not show for now

    CEGUI::System::getSingleton().getDefaultGUIContext().getRootWindow()->addChild(mp_editor_tabpane);

    mp_menu_listbox = static_cast<CEGUI::Listbox*>(mp_editor_tabpane->getChild("editor_tab_menu/editor_menu"));

    parse_menu_file();
    populate_menu();
    load_image_items();

    mp_editor_tabpane->subscribeEvent(CEGUI::Window::EventMouseEntersArea, CEGUI::Event::Subscriber(&cEditor::on_mouse_enter, this));
    mp_editor_tabpane->subscribeEvent(CEGUI::Window::EventMouseLeavesArea, CEGUI::Event::Subscriber(&cEditor::on_mouse_leave, this));

    mp_menu_listbox->subscribeEvent(CEGUI::Listbox::EventSelectionChanged, CEGUI::Event::Subscriber(&cEditor::on_menu_selection_changed, this));
}

/**
 * Empties the editor panel, detaches it from the CEGUI root window
 * and destroys it. After calling this you need to call Init()
 * again to use the editor.
 */
void cEditor::Unload(void)
{
    std::vector<cEditor_Menu_Entry*>::iterator iter;
    for(iter=m_menu_entries.begin(); iter != m_menu_entries.end(); iter++)
        delete *iter;

    if (mp_editor_tabpane) {
        CEGUI::System::getSingleton().getDefaultGUIContext().getRootWindow()->removeChild(mp_editor_tabpane);
        CEGUI::WindowManager::getSingleton().destroyWindow(mp_editor_tabpane); // destroys child windows
        mp_editor_tabpane = NULL;
    }
}

void cEditor::Toggle(cSprite_Manager* p_sprite_manager)
{
    if (m_enabled)
        Disable();
    else
        Enable(p_sprite_manager);
}

void cEditor::Enable(cSprite_Manager* p_sprite_manager)
{
    if (m_enabled)
        return;

    // TRANS: displayed to the user when opening the editor
    Draw_Static_Text(_("Loading"), &orange, NULL, 0);

    pAudio->Play_Sound("editor/enter.ogg");
    pHud_Debug->Set_Text(_("Editor enabled"));
    pMouseCursor->Set_Active(true);

    pActive_Animation_Manager->Delete_All(); // Stop all animations

    mp_editor_tabpane->show();
    m_enabled = true;
    editor_enabled = true;
    mp_sprite_manager = p_sprite_manager;
}

void cEditor::Disable(void)
{
    if (!m_enabled)
        return;

    pAudio->Play_Sound("editor/leave.ogg");
    pMouseCursor->Reset(0);
    pMouseCursor->Set_Active(false);

    mp_editor_tabpane->hide();
    m_enabled = false;
    editor_enabled = false;
    mp_sprite_manager = NULL;
}

void cEditor::Update(void)
{
    if (!m_enabled)
        return;

    // If the mouse is in the editor panel, do not fade it out.
    if (!m_mouse_inside) {
        // Otherwise, slowly fade the panel out until it is invisible.
        // When it reaches transparency, set to fully visible again
        // and place it on the side.
        if (!m_rested) {
            float timeout = speedfactor_fps * 2;
            if (m_visibility_timer >= timeout) {
                mp_editor_tabpane->setXPosition(CEGUI::UDim(-0.19f, 0.0f));
                mp_editor_tabpane->setAlpha(1.0f);

                m_rested = true;
                m_visibility_timer = 0.0f;
            }
            else {
                m_visibility_timer += pFramerate->m_speed_factor;
                float alpha_max = 1.0f;
                mp_editor_tabpane->setAlpha(alpha_max - ((alpha_max * m_visibility_timer) / timeout));
            }
        }
    }

    pMouseCursor->Editor_Update();
}

void cEditor::Draw(void)
{
    if (!m_enabled) {
        return;
    }

    const float camera_top = pActive_Camera->m_y;
    const float camera_bottom = pActive_Camera->m_y + game_res_h;
    const float camera_left = pActive_Camera->m_x;
    const float camera_right = pActive_Camera->m_x + game_res_w;

    Color color;

    // Camera limit bottom line
    if (camera_bottom > pActive_Camera->m_limit_rect.m_y && camera_top < pActive_Camera->m_limit_rect.m_y) {
        float start_x = 0.0f;

        if (pActive_Camera->m_x < 0.0f) {
            start_x = -pActive_Camera->m_x;
        }

        color = Color(static_cast<uint8_t>(0), 0, 100, 192);
        pVideo->Draw_Line(start_x, -pActive_Camera->m_y, static_cast<float>(game_res_w), -pActive_Camera->m_y, 0.124f, &color);
    }
    // Camera limit top line
    if (camera_bottom > pActive_Camera->m_limit_rect.m_y + pActive_Camera->m_limit_rect.m_h && camera_top < pActive_Camera->m_limit_rect.m_y + pActive_Camera->m_limit_rect.m_h) {
        float start_x = 0.0f;

        if (pActive_Camera->m_x < pActive_Camera->m_limit_rect.m_x) {
            start_x = -pActive_Camera->m_x;
        }

        color = Color(static_cast<uint8_t>(20), 20, 150, 192);
        pVideo->Draw_Line(start_x, pActive_Camera->m_limit_rect.m_y + pActive_Camera->m_limit_rect.m_h - pActive_Camera->m_y, static_cast<float>(game_res_w), pActive_Camera->m_limit_rect.m_y + pActive_Camera->m_limit_rect.m_h - pActive_Camera->m_y, 0.124f, &color);
    }

    // Camera limit left line
    if (camera_left < pActive_Camera->m_limit_rect.m_x && camera_right > pActive_Camera->m_limit_rect.m_x) {
        float start_y = static_cast<float>(game_res_h);

        if (pActive_Camera->m_y < game_res_h) {
            start_y = game_res_h - pActive_Camera->m_y;
        }

        color = Color(static_cast<uint8_t>(0), 100, 0, 192);
        pVideo->Draw_Line(pActive_Camera->m_limit_rect.m_x - pActive_Camera->m_x, start_y, -pActive_Camera->m_x, 0, 0.124f, &color);
    }
    // Camera limit right line
    if (camera_left < pActive_Camera->m_limit_rect.m_x + pActive_Camera->m_limit_rect.m_w && camera_right > pActive_Camera->m_limit_rect.m_x + pActive_Camera->m_limit_rect.m_w) {
        float start_y = static_cast<float>(game_res_h);

        if (pActive_Camera->m_y < game_res_h) {
            start_y = game_res_h - pActive_Camera->m_y;
        }

        color = Color(static_cast<uint8_t>(20), 150, 20, 192);
        pVideo->Draw_Line(pActive_Camera->m_limit_rect.m_x + pActive_Camera->m_limit_rect.m_w - pActive_Camera->m_x, start_y, pActive_Camera->m_limit_rect.m_x + pActive_Camera->m_limit_rect.m_w - pActive_Camera->m_x, 0, 0.124f, &color);
    }
}

bool cEditor::Key_Down(const sf::Event& evt)
{
    if (!m_enabled)
        return false;

    // New level
    if (evt.key.code == sf::Keyboard::N && evt.key.control) {
        Function_New();
    }
    // Save
    else if (evt.key.code == sf::Keyboard::S && evt.key.control) {
        Function_Save();
    }
    // Save as
    else if (evt.key.code == sf::Keyboard::S && evt.key.control && evt.key.shift) {
        Function_Save_as();
    }
    else if (evt.key.code == sf::Keyboard::F1) {
        if (m_help_window_visible) {
            on_help_window_exit_clicked(CEGUI::EventArgs());
        }
        else {
            CEGUI::FrameWindow* p_helpframe = static_cast<CEGUI::FrameWindow*>(CEGUI::WindowManager::getSingleton().createWindow("TaharezLook/FrameWindow", "editor_help"));
            p_helpframe->setPosition(CEGUI::UVector2(CEGUI::UDim(0, (game_res_w * 0.1f) * global_upscalex), CEGUI::UDim(0, (game_res_h * 0.1f) * global_upscalex)));
            p_helpframe->setSize(CEGUI::USize(CEGUI::UDim(0, (game_res_w * 0.8f) * global_upscalex), CEGUI::UDim(0, (game_res_h * 0.8f) * global_upscalex)));
            // TRANS: Title of the editor help window
            p_helpframe->setText(UTF8_("Editor Help"));

            p_helpframe->getCloseButton()->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&cEditor::on_help_window_exit_clicked, this));

            CEGUI::Window* p_helptext = CEGUI::WindowManager::getSingleton().createWindow("TaharezLook/StaticText", "editor_help_text");
            p_helptext->setPosition(CEGUI::UVector2(CEGUI::UDim(0.00f, 0.0f), CEGUI::UDim(0.00f, 0.0f)));
            p_helptext->setSize(CEGUI::USize(CEGUI::UDim(1, 0.0f), CEGUI::UDim(1, 0.0f)));
            p_helptext->setProperty("VertScrollbar", "True");
            p_helptext->setProperty("FrameEnabled", "False");
            p_helptext->setProperty("BackgroundEnabled", "False");
            p_helptext->setProperty("VertFormatting", "TopAligned");

            // TRANS: This is the help window in the editor. Do not translate
            // TRANS: the color codes in the string.
            p_helptext->setText(UTF8_(" \n"
                                      "----- [colour='FFFFCF5F']General[colour='FFFFFFFF'] -----\n"
                                      " \n"
                                      "F1 - Toggle this Help Window\n"
                                      "F8 - Open/Close the Editor\n"
                                      "F10 - Toggle sound effects\n"
                                      "F11 - Toggle music play\n"
                                      "Home - Focus level start\n"
                                      "End - Focus last level exit\n"
                                      "Ctrl + G - Goto Camera position\n"
                                      "N - Step one screen to the right ( Next Screen )\n"
                                      "P - Step one screen to the left ( Previous Screen )\n"
                                      "Ctrl + N - Create a new Level\n"
                                      "Ctrl + L - Load a Level\n"
                                      "Ctrl + W - Load an Overworld\n"
                                      "Ctrl + S - Save the current Level/World\n"
                                      "Ctrl + Shift + S - Save the current Level/World under a new name\n"
                                      "Ctrl + D - Toggle debug mode\n"
                                      "Ctrl + P - Toggle performance mode\n"
                                      " \n"
                                      "----- [colour='FFFFCF5F']Objects[colour='FFFFFFFF'] -----\n"
                                      " \n"
                                      "M - Cycle selected object(s) through massive types\n"
                                      "Massive types with their color:\n"
                                      "   [colour='FFFF2F0F']Massive[colour='FFFFFFFF']->[colour='FFFFAF2F']Halfmassive[colour='FFFFFFFF']->[colour='FFDF00FF']Climbable[colour='FFFFFFFF']->[colour='FF1FFF1F']Passive[colour='FFFFFFFF']->[colour='FF1FFF1F']Front Passive[colour='FFFFFFFF']\n"
                                      "O - Enable snap to object mode\n"
                                      "Ctrl + A - Select all objects\n"
                                      "Ctrl + X - Cut currently selected objects\n"
                                      "Ctrl + C - Copy currently selected objects\n"
                                      "Ctrl + V or Insert - Paste current copied / cutted objects\n"
                                      "Ctrl + R - Replace the selected basic sprite(s) image with another one\n"
                                      "Del - If Mouse is over an object: Delete current object\n"
                                      "Del - If Mouse has nothing selected: Delete selected objects\n"
                                      "Numpad:\n"
                                      " + - Bring object to front\n"
                                      " - - Send object to back\n"
                                      " 2/4/6/8 - Move selected object 1 pixel into the direction\n"
                                      "Mouse:\n"
                                      " Left (Hold) - Drag objects\n"
                                      " Left (Click) - With shift to select / deselect single objects\n"
                                      " Right - Delete intersecting Object\n"
                                      " Middle - Toggle Mover Mode\n"
                                      " Ctrl + Shift + Left (Click) - Select objects with the same type\n"
                                      "Arrow keys:\n"
                                      " Use arrow keys to move around. Press shift for faster movement\n"
                                      " \n")
                );

            CEGUI::Window* p_root = CEGUI::System::getSingleton().getDefaultGUIContext().getRootWindow();
            p_helpframe->addChild(p_helptext);
            p_root->addChild(p_helpframe);

            m_help_window_visible = true;
        }
    }
    // focus level start
    else if (evt.key.code == sf::Keyboard::Home) {
        pActive_Camera->Reset_Pos();
    }
    // move camera one screen to the right
    else if (evt.key.code == sf::Keyboard::N) {
        pActive_Camera->Move(static_cast<float>(game_res_w), 0);
    }
    // move camera one screen to the left
    else if (evt.key.code == sf::Keyboard::P) {
        pActive_Camera->Move(-static_cast<float>(game_res_w), 0);
    }
    // move camera to position
    else if (evt.key.code == sf::Keyboard::G && evt.key.control) {
        int pos_x = string_to_int(Box_Text_Input(int_to_string(static_cast<int>(pActive_Camera->m_x)), "Position X", 1));
        int pos_y = string_to_int(Box_Text_Input(int_to_string(static_cast<int>(pActive_Camera->m_y)), "Position Y", 1));

        pActive_Camera->Set_Pos(static_cast<float>(pos_x), static_cast<float>(pos_y));
    }
    // push selected objects into the front
    else if (evt.key.code == sf::Keyboard::Add) {
        for (SelectedObjectList::iterator itr = pMouseCursor->m_selected_objects.begin(); itr != pMouseCursor->m_selected_objects.end(); ++itr) {
            cSelectedObject* sel_obj = (*itr);

            if (!sel_obj->m_obj->Is_Sprite_Managed()) {
                continue;
            }

            // last object is in front of others
            mp_sprite_manager->Move_To_Back(sel_obj->m_obj);
        }
    }
    // push selected objects into the back
    else if (evt.key.code == sf::Keyboard::Subtract) {
        for (SelectedObjectList::iterator itr = pMouseCursor->m_selected_objects.begin(); itr != pMouseCursor->m_selected_objects.end(); ++itr) {
            cSelectedObject* sel_obj = (*itr);

            if (!sel_obj->m_obj->Is_Sprite_Managed()) {
                continue;
            }

            // first object is behind others
            mp_sprite_manager->Move_To_Front(sel_obj->m_obj);
        }
    }
    // copy into direction
    else if ((evt.key.code == pPreferences->m_key_editor_fast_copy_up || evt.key.code == pPreferences->m_key_editor_fast_copy_down || evt.key.code == pPreferences->m_key_editor_fast_copy_left || evt.key.code == pPreferences->m_key_editor_fast_copy_right) && pMouseCursor->m_hovering_object->m_obj && pMouseCursor->m_fastcopy_mode) {
        ObjectDirection dir = DIR_UNDEFINED;

        if (evt.key.code == pPreferences->m_key_editor_fast_copy_up) {
            dir = DIR_UP;
        }
        else if (evt.key.code == pPreferences->m_key_editor_fast_copy_down) {
            dir = DIR_DOWN;
        }
        else if (evt.key.code == pPreferences->m_key_editor_fast_copy_left) {
            dir = DIR_LEFT;
        }
        else if (evt.key.code == pPreferences->m_key_editor_fast_copy_right) {
            dir = DIR_RIGHT;
        }

        // get currently selected objects
        cSprite_List objects = pMouseCursor->Get_Selected_Objects();
        // copy objects
        cSprite_List new_objects = copy_direction(objects, dir);

        // add new objects
        for (cSprite_List::iterator itr = new_objects.begin(); itr != new_objects.end(); ++itr) {
            cSprite* obj = (*itr);

            pMouseCursor->Add_Selected_Object(obj, 1);
        }

        // deselect old objects
        for (cSprite_List::const_iterator itr = objects.begin(); itr != objects.end(); ++itr) {
            const cSprite* obj = (*itr);

            pMouseCursor->Remove_Selected_Object(obj);
        }
    }
    // Precise Pixel-Positioning
    else if ((evt.key.code == pPreferences->m_key_editor_pixel_move_up || evt.key.code == pPreferences->m_key_editor_pixel_move_down || evt.key.code == pPreferences->m_key_editor_pixel_move_left || evt.key.code == pPreferences->m_key_editor_pixel_move_right)) {
        int x_offset = 0;
        int y_offset = 0;

        if (evt.key.code == pPreferences->m_key_editor_pixel_move_up) {
            y_offset = -1;
        }
        else if (evt.key.code == pPreferences->m_key_editor_pixel_move_down) {
            y_offset = 1;
        }
        else if (evt.key.code == pPreferences->m_key_editor_pixel_move_left) {
            x_offset = -1;
        }
        else if (evt.key.code == pPreferences->m_key_editor_pixel_move_right) {
            x_offset = 1;
        }

        for (SelectedObjectList::iterator itr = pMouseCursor->m_selected_objects.begin(); itr != pMouseCursor->m_selected_objects.end(); ++itr) {
            cSelectedObject* sel_obj = (*itr);
            cSprite* obj = sel_obj->m_obj;

            obj->Set_Pos(obj->m_pos_x + x_offset, obj->m_pos_y + y_offset, true);
        }
    }
    // deselect everything
    else if (evt.key.code == sf::Keyboard::A && evt.key.control && evt.key.shift) {
        pMouseCursor->Clear_Selected_Objects();
    }
    // select everything
    else if (evt.key.code == sf::Keyboard::A && evt.key.control) {
        pMouseCursor->Clear_Selected_Objects();

        // player
        pMouseCursor->Add_Selected_Object(pActive_Player, 1);
        // sprite manager
        for (cSprite_List::iterator itr = mp_sprite_manager->objects.begin(); itr != mp_sprite_manager->objects.end(); ++itr) {
            cSprite* obj = (*itr);

            pMouseCursor->Add_Selected_Object(obj, 1);
        }
    }
    // Paste copy buffer objects
    else if ((evt.key.code == sf::Keyboard::Insert || evt.key.code == sf::Keyboard::V) && evt.key.control) {
        pMouseCursor->Paste_Copy_Objects(static_cast<float>(static_cast<int>(pMouseCursor->m_pos_x)), static_cast<float>(static_cast<int>(pMouseCursor->m_pos_y)));
    }
    // Cut selected Sprites to the copy buffer
    else if (evt.key.code == sf::Keyboard::X && evt.key.control) {
        pMouseCursor->Clear_Copy_Objects();

        for (SelectedObjectList::iterator itr = pMouseCursor->m_selected_objects.begin(); itr != pMouseCursor->m_selected_objects.end(); ++itr) {
            cSelectedObject* sel_obj = (*itr);

            pMouseCursor->Add_Copy_Object(sel_obj->m_obj);
        }

        pMouseCursor->Delete_Selected_Objects();
    }
    // Add selected Sprites to the copy buffer
    else if (evt.key.code == sf::Keyboard::C && evt.key.control) {
        pMouseCursor->Clear_Copy_Objects();

        for (SelectedObjectList::iterator itr = pMouseCursor->m_selected_objects.begin(); itr != pMouseCursor->m_selected_objects.end(); ++itr) {
            cSelectedObject* sel_obj = (*itr);

            pMouseCursor->Add_Copy_Object(sel_obj->m_obj);
        }
    }
    // Replace sprites
    else if (evt.key.code == sf::Keyboard::R && evt.key.control) {
        replace_sprites();
    }
    // Delete mouse object
    else if (evt.key.code == sf::Keyboard::Delete && pMouseCursor->m_hovering_object->m_obj) {
        pMouseCursor->Delete(pMouseCursor->m_hovering_object->m_obj);
    }
    // if shift got pressed remove mouse object for possible mouse selection
    else if (evt.key.shift && pMouseCursor->m_hovering_object->m_obj) {
        pMouseCursor->Clear_Hovered_Object();
    }
    // Delete selected objects
    else if (evt.key.code == sf::Keyboard::Delete) {
        pMouseCursor->Delete_Selected_Objects();
    }
    // Snap to objects mode
    else if (evt.key.code == sf::Keyboard::O) {
        pMouseCursor->Toggle_Snap_Mode();
    }
    else {
        // not processed
        return false;
    }

    // key got processed
    return true;

}

bool cEditor::Mouse_Down(sf::Mouse::Button button)
{
    if (!m_enabled) {
        return 0;
    }

    // left
    if (button == sf::Mouse::Left) {
        pMouseCursor->Left_Click_Down();

        // auto hide if enabled
        if (pMouseCursor->m_hovering_object->m_obj && pPreferences->m_editor_mouse_auto_hide) {
            pMouseCursor->Set_Active(0);
        }
    }
    // middle
    else if (button == sf::Mouse::Middle) {
        // Activate fast copy mode
        if (pMouseCursor->m_hovering_object->m_obj) {
            pMouseCursor->m_fastcopy_mode = 1;
            return 1;
        }
        // Mover mode
        else {
            pMouseCursor->Toggle_Mover_Mode();
            return 1;
        }
    }
    // right
    else if (button == sf::Mouse::Right) {
        if (!pMouseCursor->m_left) {
            pMouseCursor->Delete(pMouseCursor->m_hovering_object->m_obj);
            return 1;
        }
    }
    else {
        // not processed
        return 0;
    }

    // button got processed
    return 1;
}

bool cEditor::Mouse_Up(sf::Mouse::Button button)
{
    if (!m_enabled) {
        return 0;
    }

    // left
    if (button == sf::Mouse::Left) {
        // unhide
        if (pPreferences->m_editor_mouse_auto_hide) {
            pMouseCursor->Set_Active(1);
        }

        pMouseCursor->End_Selection();

        if (pMouseCursor->m_hovering_object->m_obj) {
            for (SelectedObjectList::iterator itr = pMouseCursor->m_selected_objects.begin(); itr != pMouseCursor->m_selected_objects.end(); ++itr) {
                cSelectedObject* object = (*itr);

                // pre-update to keep particles on the correct position
                if (object->m_obj->m_type == TYPE_PARTICLE_EMITTER) {
                    cParticle_Emitter* emitter = static_cast<cParticle_Emitter*>(object->m_obj);
                    emitter->Pre_Update();
                }
            }
        }
    }
    // middle
    else if (button == sf::Mouse::Middle) {
        pMouseCursor->m_fastcopy_mode = 0;
    }
    else {
        // not processed
        return 0;
    }

    // button got processed
    return 1;
}

/**
 * Adds a graphic to the editor menu. The settings file of this
 * graphic will be parsed and it will be placed in the menu
 * accordingly (subclasses have to set the `m_editor_item_tag`
 * member variable the the master tag required for graphics to
 * show up in this editor; these are "level" and "world" for the
 * level and world editor subclasses, respectively. That is,
 * a graphic tagged with "world" will never appear in the level
 * editor, and vice-versa.).
 *
 * \param settings_path
 * Absolute path that refers to the settings file
 * of the graphic to add.
 *
 * \returns false if the item was not added because the master tag
 * was missing, true otherwise.
 */
bool cEditor::Try_Add_Editor_Item(boost::filesystem::path settings_path)
{
    // Parse the image's settings file
    cImage_Settings_Parser parser;
    cImage_Settings_Data* p_settings = parser.Get(settings_path);

    // If the master tag is not in the tag list, do not add this graphic to the
    // editor.
    if (p_settings->m_editor_tags.find(m_editor_item_tag) == std::string::npos)
        return false;

    // Find the menu entries that contain the tags this graphic has set.
    std::vector<cEditor_Menu_Entry*> target_menu_entries = find_target_menu_entries_for(*p_settings);
    std::vector<cEditor_Menu_Entry*>::iterator iter;

    // Add the graphics to the respective menu entries' GUI panels.
    for(iter=target_menu_entries.begin(); iter != target_menu_entries.end(); iter++) {
        (*iter)->Add_Image_Item(settings_path, *p_settings);
    }

    delete p_settings;
    return true;
}

void cEditor::parse_menu_file()
{
    std::string menu_file = path_to_utf8(m_menu_filename);

    // The menu XML file is so dead simple that a SAX parser would
    // simply be overkill. Leightweight XPath queries are enough.
    xmlpp::DomParser parser;
    parser.parse_file(menu_file);

    xmlpp::Element* p_root = parser.get_document()->get_root_node();
    xmlpp::NodeSet items = p_root->find("item");

    xmlpp::NodeSet::const_iterator iter;
    for (iter=items.begin(); iter != items.end(); iter++) {
        xmlpp::Element* p_node = dynamic_cast<xmlpp::Element*>(*iter);

        std::string name   = dynamic_cast<xmlpp::Element*>(p_node->find("property[@name='name']")[0])->get_attribute("value")->get_value();
        std::string tagstr = dynamic_cast<xmlpp::Element*>(p_node->find("property[@name='tags']")[0])->get_attribute("value")->get_value();

        // Set color if available (---header--- elements have no color property)
        std::string colorstr = "FFFFFFFF";
        xmlpp::NodeSet results = p_node->find("property[@name='color']");
        if (!results.empty())
            colorstr = dynamic_cast<xmlpp::Element*>(results[0])->get_attribute("value")->get_value();

        // Burst the tag list into its elements
        std::vector<std::string> tags = string_split(tagstr, ";");

        // Prepare informational menu object
        cEditor_Menu_Entry* p_entry = new cEditor_Menu_Entry(name);
        p_entry->Set_Color(Color(colorstr));
        p_entry->Set_Required_Tags(tags);

        // Mark as header element if the tag "header" is encountered.
        std::vector<std::string>::iterator iter;
        iter = std::find(tags.begin(), tags.end(), std::string("header"));
        p_entry->Set_Header((iter != tags.end()));

        // Mark as a function element (= does something special if clicked)
        // if the tag "function" is encountered.
        iter = std::find(tags.begin(), tags.end(), std::string("function"));
        p_entry->Set_Function((iter != tags.end()));

        // Store
        m_menu_entries.push_back(p_entry);
    }
}

void cEditor::populate_menu()
{
    std::vector<cEditor_Menu_Entry*>::iterator iter;

    for(iter = m_menu_entries.begin(); iter != m_menu_entries.end(); iter++) {
        CEGUI::ListboxTextItem* p_item = new CEGUI::ListboxTextItem((*iter)->Get_Name());
        p_item->setTextColours(CEGUI::ColourRect((*iter)->Get_Color().Get_cegui_Color()));
        p_item->setUserData(*iter);
        mp_menu_listbox->addItem(p_item);
    }
}

void cEditor::load_image_items()
{
    std::vector<boost::filesystem::path> image_files = Get_Directory_Files(pResource_Manager->Get_Game_Pixmaps_Directory(), ".settings");
    std::vector<boost::filesystem::path>::iterator iter;

    for(iter=image_files.begin(); iter != image_files.end(); iter++) {
        Try_Add_Editor_Item(*iter);
    }
}

cEditor_Menu_Entry* cEditor::get_menu_entry(const std::string& name)
{
    std::vector<cEditor_Menu_Entry*>::iterator iter;
    for(iter=m_menu_entries.begin(); iter != m_menu_entries.end(); iter++) {
        if ((*iter)->Get_Name() == name) {
            return (*iter);
        }
    }

    throw(std::runtime_error(std::string("Element '") + name + "' not in editor menu list!"));
}

/**
 * Iterates the list of menu entries and returns a list of those whose
 * requirements match the graphic given. That is, a menu entry declares
 * a set of required tags, and if the passed graphic has all of these
 * tags set, the menu entry is included in the return list. If more
 * than the required tags are set, the entry is returned nevertheless.
 */
std::vector<cEditor_Menu_Entry*> cEditor::find_target_menu_entries_for(const cImage_Settings_Data& settings)
{
    std::vector<cEditor_Menu_Entry*> results;
    std::vector<std::string> available_tags = string_split(settings.m_editor_tags, ";");

    /* To show up in the editor, a graphic needs to have all tags set
     * on it that are required by a menu entry (except for the master
     * editor tag, which is checked elsewhere). */
    std::vector<cEditor_Menu_Entry*>::iterator iter;
    for(iter=m_menu_entries.begin(); iter != m_menu_entries.end(); iter++) {
        cEditor_Menu_Entry* p_menu_entry = *iter;
        std::vector<std::string>::iterator tag_iterator;

        // Check if all of this menu's tags are on the graphic.
        bool all_tags_available = true;
        for(tag_iterator=p_menu_entry->Get_Required_Tags().begin(); tag_iterator != p_menu_entry->Get_Required_Tags().end(); tag_iterator++) {
            if (std::find(available_tags.begin(), available_tags.end(), *tag_iterator) == available_tags.end()) {
                // Tag is missing.
                all_tags_available = false;
                break;
            }
        }

        if (all_tags_available) {
            results.push_back(p_menu_entry);
        }
    }

    return results;
}

bool cEditor::on_mouse_enter(const CEGUI::EventArgs& event)
{
    m_mouse_inside = true;
    m_visibility_timer = 0.0f;
    m_rested = false;

    mp_editor_tabpane->setAlpha(1.0f);
    mp_editor_tabpane->setXPosition(m_target_x_position);
    return true;
}

bool cEditor::on_mouse_leave(const CEGUI::EventArgs& event)
{
    m_mouse_inside = false;
    return true;
}

bool cEditor::on_menu_selection_changed(const CEGUI::EventArgs& event)
{
    CEGUI::ListboxItem* p_current_item = mp_menu_listbox->getFirstSelectedItem();

    // Ignore if no selection
    if (!p_current_item)
        return false;

    cEditor_Menu_Entry* p_menu_entry = static_cast<cEditor_Menu_Entry*>(p_current_item->getUserData());

    // Ignore clicks on headings
    if (p_menu_entry->Is_Header())
        return false;

    // Treat clicks on function elements specifically
    if (p_menu_entry->Is_Function()) {
        Activate_Function_Entry(p_menu_entry);
        return true;
    }

    // Ordinary menu item (i.e. submenu with game objects).
    p_menu_entry->Activate(mp_editor_tabpane);

    return true;
}

/**
 * Takes a function menu entry and executes the Function_*() function it
 * belongs to. These functions are intended to be overridden in subclasses.
 */
void cEditor::Activate_Function_Entry(cEditor_Menu_Entry* p_function_entry)
{
    std::vector<std::string>& tags = p_function_entry->Get_Required_Tags();
    std::vector<std::string>::const_iterator iter;

    // HIER! Führt zu einer Exception!

    if (std::find(tags.begin(), tags.end(), std::string("new")) != tags.end())
        Function_New();
    else if (std::find(tags.begin(), tags.end(), std::string("load")) != tags.end())
        Function_Load();
    else if (std::find(tags.begin(), tags.end(), std::string("save")) != tags.end())
        Function_Save(); // TODO: optional argument?
    else if (std::find(tags.begin(), tags.end(), std::string("save_as")) != tags.end())
        Function_Save_as();
    else if (std::find(tags.begin(), tags.end(), std::string("delete")) != tags.end())
        Function_Delete();
    else if (std::find(tags.begin(), tags.end(), std::string("reload")) != tags.end())
        Function_Reload();
    else if (std::find(tags.begin(), tags.end(), std::string("settings")) != tags.end())
        Function_Settings();
    else
        throw(std::runtime_error("Invalid function menu item!"));
}

cEditor_Menu_Entry::cEditor_Menu_Entry(std::string name)
{
    m_name = name;
    m_element_y = 0;

    // Prepare the CEGUI items window. This will be shown whenever
    // this menu entry is clicked.
    mp_tab_pane = static_cast<CEGUI::ScrollablePane*>(CEGUI::WindowManager::getSingleton().createWindow("TaharezLook/ScrollablePane", std::string("editor_items_") + name));
    mp_tab_pane->setPosition(CEGUI::UVector2(CEGUI::UDim(0, 0), CEGUI::UDim(0.01, 0)));
    mp_tab_pane->setSize(CEGUI::USize(CEGUI::UDim(0.99, 0), CEGUI::UDim(0.95, 0)));
    mp_tab_pane->setContentPaneAutoSized(false);
    mp_tab_pane->setContentPaneArea(CEGUI::Rectf(0, 0, 1000, 4000));
    mp_tab_pane->setShowHorzScrollbar(false);
}

cEditor_Menu_Entry::~cEditor_Menu_Entry()
{
    // TODO: Detach if still attached and destroy
    // Nonattached windows will not get destroyed, obviously
    // CEGUI::Window* p_editor_tabpane = CEGUI::System::getSingleton().getDefaultGUIContext().getRootWindow()->getChild("tabcontrol_editor/editor_tab_items")
}

void cEditor_Menu_Entry::Add_Image_Item(boost::filesystem::path settings_path, const cImage_Settings_Data& settings)
{
    static const int labelheight = 24;
    static const int imageheight = 48; /* Also image width (square) */
    static const int yskip = 24;

    // Find the PNG of this settings file. If an equally named .png exists,
    // assume that file, otherwise check the settings 'base' property. If
    // that also doesn't exist, that's an error.
    boost::filesystem::path pixmap_path(settings_path); // Copy
    pixmap_path.replace_extension(utf8_to_path(".png"));
    if (!boost::filesystem::exists(pixmap_path)) {
        if (settings.m_base.empty()) { // Error
            std::cerr << "PNG file for settings file '" << path_to_utf8(settings_path) << "' not found (no .png found and no 'base' setting)." << std::endl;
            return;
        }
        else {
            pixmap_path = pixmap_path.parent_path() / settings.m_base;
            if (!boost::filesystem::exists(pixmap_path)) {
                std::cerr << "PNG base file not found at '" << path_to_utf8(pixmap_path) << "'." << std::endl;
                return;
            }
        }
    }

    std::string escaped_pixmap_path(path_to_utf8(pixmap_path));
    std::string escaped_settings_path(path_to_utf8(settings_path));
    string_replace_all(escaped_pixmap_path, "/", "+"); // CEGUI doesn't like / in ImageManager image names
    string_replace_all(escaped_settings_path, "/", "+");

    CEGUI::Window* p_label = CEGUI::WindowManager::getSingleton().createWindow("TaharezLook/StaticText", std::string("label-of-") + escaped_settings_path);
    p_label->setText(settings.m_name);
    p_label->setSize(CEGUI::USize(CEGUI::UDim(1, 0), CEGUI::UDim(0, labelheight)));
    p_label->setPosition(CEGUI::UVector2(CEGUI::UDim(0, 0), CEGUI::UDim(0, m_element_y)));
    p_label->setProperty("FrameEnabled", "False");

    /* CEGUI only knows about image sets, not about single images.
     * Thus we effectively add one-image imagesets here to have CEGUI
     * display our image. Note CEGUI also caches these images and will
     * throw a CEGUI::AlreadyExsitsException in addFromImageFile() if
     * an image is added multiple times (like multiple loads of the
     * editor or just the same item being added to multiple menus).
     * Thus we have to check if the item graphic has been cached before,
     * and only if that isn't the case, load the file from disk in the
     * way described. */
    if (!CEGUI::ImageManager::getSingleton().isDefined(escaped_pixmap_path)) {
        try {
            CEGUI::ImageManager::getSingleton().addFromImageFile(escaped_pixmap_path, path_to_utf8(fs_relative(pResource_Manager->Get_Game_Pixmaps_Directory(), pixmap_path)), "ingame-images");
        }
        catch(CEGUI::RendererException& e) {
            std::cerr << "Warning: Failed to load as editor item image: " << pixmap_path << std::endl;
            return;
        }
    }

    CEGUI::Window* p_image = CEGUI::WindowManager::getSingleton().createWindow("TaharezLook/StaticImage", std::string("image-of-") + escaped_settings_path);
    p_image->setProperty("Image", escaped_pixmap_path);
    p_image->setSize(CEGUI::USize(CEGUI::UDim(0, imageheight), CEGUI::UDim(0, imageheight)));
    p_image->setPosition(CEGUI::UVector2(CEGUI::UDim(0.5, -imageheight/2) /* center on X */, CEGUI::UDim(0, m_element_y + labelheight)));
    p_image->setProperty("FrameEnabled", "False");
    p_image->setUserData(new boost::filesystem::path(settings_path)); // TODO: FIXME: Memory leak! This pointer is never freed!
    p_image->subscribeEvent(CEGUI::Window::EventMouseButtonDown, CEGUI::Event::Subscriber(&cEditor_Menu_Entry::on_image_mouse_down, this));

    // Apply rotation
    p_image->setRotation(CEGUI::Quaternion::eulerAnglesDegrees(settings.m_rotation_x, settings.m_rotation_y, settings.m_rotation_z));

    mp_tab_pane->addChild(p_label);
    mp_tab_pane->addChild(p_image);

    // Remember where we stopped for the next call.
    m_element_y += labelheight + imageheight + yskip;
}

/// Activate this entry's panel in the handed tabbook.
void cEditor_Menu_Entry::Activate(CEGUI::TabControl* p_tabcontrol)
{
    CEGUI::Window* p_container = p_tabcontrol->getTabContents("editor_tab_items");

    // Detach the current scrollable pane.
    CEGUI::ScrollablePane* p_current_pane = static_cast<CEGUI::ScrollablePane*>(p_container->getChildElementAtIdx(0));
    p_container->removeChild(p_current_pane);

    // Attach our content instead.
    p_container->addChild(mp_tab_pane);

    // Switch to the items page
    p_tabcontrol->setSelectedTab("editor_tab_items");
}

bool cEditor_Menu_Entry::on_image_mouse_down(const CEGUI::EventArgs& ev)
{
    // EventMouseButtonDown event handler receives a CEGUI::MouseEventArgs in reality,
    // see http://static.cegui.org.uk/docs/0.8.7/classCEGUI_1_1Window.html#a073c0f8e07cad39c21dce04cc2e49b3c.
    const CEGUI::MouseEventArgs& event = static_cast<const CEGUI::MouseEventArgs&>(ev);

    boost::filesystem::path* p_path = static_cast<boost::filesystem::path*>(event.window->getUserData());

    std::cout << "CLICKED THIS: " << path_to_utf8(*p_path) << std::endl;
    return true;
}

void cEditor::Select_Same_Object_Types(const cSprite* obj)
{
    if (!obj) {
        return;
    }

    bool is_basic_sprite = 0;

    if (obj->Is_Basic_Sprite()) {
        if (!obj->m_start_image) {
            return;
        }

        is_basic_sprite = 1;
    }

    // sprite manager
    for (cSprite_List::iterator itr = mp_sprite_manager->objects.begin(); itr != mp_sprite_manager->objects.end(); ++itr) {
        cSprite* game_obj = (*itr);

        if (game_obj->m_type != obj->m_type) {
            continue;
        }

        if (is_basic_sprite && game_obj->m_start_image != obj->m_start_image) {
            continue;
        }

        pMouseCursor->Add_Selected_Object(game_obj, 1);
    }
}

void cEditor::Process_Input(void)
{
    if (!m_enabled) {
        return;
    }

    // Drag Delete
    if (pKeyboard->Is_Ctrl_Down() && pMouseCursor->m_right) {
        cObjectCollision* col = pMouseCursor->Get_First_Editor_Collsion();

        if (col) {
            pMouseCursor->Delete(col->m_obj);
            delete col;
        }
    }

    // Camera Movement
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) || pJoystick->m_right) {
        if (pKeyboard->Is_Shift_Down()) {
            pActive_Camera->Move(CAMERA_SPEED * pFramerate->m_speed_factor * 3 * pPreferences->m_scroll_speed, 0.0f);
        }
        else {
            pActive_Camera->Move(CAMERA_SPEED * pFramerate->m_speed_factor * pPreferences->m_scroll_speed, 0.0f);
        }
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) || pJoystick->m_left) {
        if (pKeyboard->Is_Shift_Down()) {
            pActive_Camera->Move(-(CAMERA_SPEED * pFramerate->m_speed_factor * 3 * pPreferences->m_scroll_speed), 0.0f);
        }
        else {
            pActive_Camera->Move(-(CAMERA_SPEED * pFramerate->m_speed_factor * pPreferences->m_scroll_speed), 0.0f);
        }
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) || pJoystick->m_up) {
        if (pKeyboard->Is_Shift_Down()) {
            pActive_Camera->Move(0.0f, -(CAMERA_SPEED * pFramerate->m_speed_factor * 3 * pPreferences->m_scroll_speed));
        }
        else {
            pActive_Camera->Move(0.0f, -(CAMERA_SPEED * pFramerate->m_speed_factor * pPreferences->m_scroll_speed));
        }
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) || pJoystick->m_down) {
        if (pKeyboard->Is_Shift_Down()) {
            pActive_Camera->Move(0.0f, CAMERA_SPEED * pFramerate->m_speed_factor * 3 * pPreferences->m_scroll_speed);
        }
        else {
            pActive_Camera->Move(0.0f, CAMERA_SPEED * pFramerate->m_speed_factor * pPreferences->m_scroll_speed);
        }
    }
}

void cEditor::Function_Exit(void)
{
    sf::Event newevt;
    newevt.type = sf::Event::KeyPressed;
    newevt.key.code = sf::Keyboard::F8;
    pKeyboard->Key_Down(newevt);
}

cSprite_List cEditor::copy_direction(const cSprite_List& objects, const ObjectDirection dir) const
{
    // additional direction objects offset
    unsigned int offset = 0;

    // get the objects difference offset
    if (dir == DIR_LEFT || dir == DIR_RIGHT) {
        // first object
        const cSprite* first = objects[0];

        for (unsigned int i = 1; i < objects.size(); i++) {
            if (objects[i]->m_start_pos_x < first->m_start_pos_x) {
                first = objects[i];
            }
        }

        // last object
        const cSprite* last = objects[0];

        for (unsigned int i = 1; i < objects.size(); i++) {
            if (objects[i]->m_start_pos_x + objects[i]->m_start_rect.m_w > last->m_start_pos_x + last->m_start_rect.m_w) {
                last = objects[i];
            }
        }

        // Set X offset
        offset = static_cast<int>(last->m_start_pos_x - first->m_start_pos_x + last->m_start_rect.m_w);
    }
    else if (dir == DIR_UP || dir == DIR_DOWN) {
        // first object
        const cSprite* first = objects[0];

        for (unsigned int i = 1; i < objects.size(); i++) {
            if (objects[i]->m_start_pos_y < first->m_start_pos_y) {
                first = objects[i];
            }
        }

        // last object
        const cSprite* last = objects[0];

        for (unsigned int i = 1; i < objects.size(); i++) {
            if (objects[i]->m_start_pos_y + objects[i]->m_start_rect.m_h > last->m_start_pos_y + last->m_start_rect.m_h) {
                last = objects[i];
            }
        }

        // Set Y offset
        offset = static_cast<int>(last->m_start_pos_y - first->m_start_pos_y + last->m_start_rect.m_h);
    }

    // new copied objects
    cSprite_List new_objects;

    for (cSprite_List::const_iterator itr = objects.begin(); itr != objects.end(); ++itr) {
        const cSprite* obj = (*itr);

        new_objects.push_back(copy_direction(obj, dir, offset));
    }

    // return only new objects
    return new_objects;
}

cSprite* cEditor::copy_direction(const cSprite* obj, const ObjectDirection dir, int offset /* = 0 */) const
{
    float w = 0.0f;
    float h = 0.0f;

    if (dir == DIR_LEFT) {
        if (offset) {
            w = -static_cast<float>(offset);
        }
        else {
            w = -obj->m_start_rect.m_w;
        }
    }
    else if (dir == DIR_RIGHT) {
        if (offset) {
            w = static_cast<float>(offset);
        }
        else {
            w = obj->m_start_rect.m_w;
        }
    }
    else if (dir == DIR_UP) {
        if (offset) {
            h = -static_cast<float>(offset);
        }
        else {
            h = -obj->m_start_rect.m_h;
        }
    }
    else if (dir == DIR_DOWN) {
        if (offset) {
            h = static_cast<float>(offset);
        }
        else {
            h = obj->m_start_rect.m_h;
        }
    }

    // only move camera if obj is the mouse object
    if (pMouseCursor->m_hovering_object->m_obj == obj) {
        pActive_Camera->Move(w, h);
    }

    return pMouseCursor->Copy(obj, obj->m_start_pos_x + w, obj->m_start_pos_y + h);
}

void cEditor::replace_sprites(void)
{
    if (pMouseCursor->Get_Selected_Object_Size() == 0) {
        return;
    }

    if (!pMouseCursor->m_selected_objects[0]->m_obj->m_start_image) {
        return;
    }

    char i18nstr[256];
    sprintf(i18nstr, PL_("Change selected sprite's image", "Change selected %d sprites' images", pMouseCursor->Get_Selected_Object_Size()), pMouseCursor->Get_Selected_Object_Size());

    std::string image_filename = Box_Text_Input(path_to_utf8(pMouseCursor->m_selected_objects[0]->m_obj->m_start_image->m_path), i18nstr, 0);

    // aborted/invalid
    if (image_filename.empty()) {
        return;
    }

    cGL_Surface* image = pVideo->Get_Surface(image_filename);

    if (!image) {
        return;
    }

    for (SelectedObjectList::iterator itr = pMouseCursor->m_selected_objects.begin(); itr != pMouseCursor->m_selected_objects.end(); ++itr) {
        cSelectedObject* sel_obj = (*itr);

        // only sprites
        if (!sel_obj->m_obj->Is_Basic_Sprite()) {
            continue;
        }

        sel_obj->m_obj->Set_Image(image, 1);
    }
}

bool cEditor::on_help_window_exit_clicked(const CEGUI::EventArgs& args)
{
    CEGUI::Window* p_root = CEGUI::System::getSingleton().getDefaultGUIContext().getRootWindow();
    CEGUI::FrameWindow* p_helpframe = static_cast<CEGUI::FrameWindow*>(p_root->getChild("editor_help"));

    p_root->removeChild(p_helpframe);
    CEGUI::WindowManager::getSingleton().destroyWindow(p_helpframe);
    m_help_window_visible = false;

    return true;
}

#endif
