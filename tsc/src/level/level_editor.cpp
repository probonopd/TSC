#include "../core/global_basic.hpp"
#include "../core/game_core.hpp"
#include "../gui/generic.hpp"
#include "../core/framerate.hpp"
#include "../audio/audio.hpp"
#include "../video/font.hpp"
#include "../video/animation.hpp"
#include "../input/keyboard.hpp"
#include "../input/mouse.hpp"
#include "../input/joystick.hpp"
#include "../user/preferences.hpp"
#include "../level/level.hpp"
#include "../level/level_player.hpp"
#include "../level/level_loader.hpp"
#include "../overworld/world_manager.hpp"
#include "../video/renderer.hpp"
#include "../core/sprite_manager.hpp"
#include "../overworld/overworld.hpp"
#include "../core/i18n.hpp"
#include "../core/filesystem/filesystem.hpp"
#include "../core/filesystem/resource_manager.hpp"
#include "../core/editor/editor_items_loader.hpp"
#include "../core/xml_attributes.hpp"
#include "../core/errors.hpp"
#include "level_settings.hpp"
#include "level_editor.hpp"

#ifdef ENABLE_EDITOR

using namespace TSC;

// extern
cEditor_Level* TSC::pLevel_Editor = NULL;

cEditor_Level::cEditor_Level()
    : cEditor()
{
    mp_level = NULL;
    m_editor_item_tag = "level";
    m_menu_filename = pResource_Manager->Get_Game_Editor("level_menu.xml");
}

cEditor_Level::~cEditor_Level()
{
}

void cEditor_Level::Enable(cSprite_Manager* p_sprite_manager)
{
    if (m_enabled)
        return;

    cEditor::Enable(p_sprite_manager);
    editor_level_enabled = true;
}

void cEditor_Level::Disable(void)
{
    if (!m_enabled)
        return;

    cEditor::Disable();
    editor_level_enabled = false;
}

bool cEditor_Level::Key_Down(const sf::Event& evt)
{
    if (!m_enabled)
        return false;

    // Handle general commands
    if (cEditor::Key_Down(evt)) {
        return true;
    }
    // focus last levelexit
    else if (evt.key.code == sf::Keyboard::End) {
        float new_camera_posx = 0.0f;
        float new_camera_posy = 0.0f;

        for (cSprite_List::iterator itr = mp_edited_sprite_manager->objects.begin(); itr != mp_edited_sprite_manager->objects.end(); ++itr) {
            cSprite* obj = (*itr);

            if (obj->m_sprite_array != ARRAY_ACTIVE) {
                continue;
            }

            if (obj->m_type == TYPE_LEVEL_EXIT && new_camera_posx < obj->m_pos_x) {
                new_camera_posx = obj->m_pos_x;
                new_camera_posy = obj->m_pos_y;
            }
        }

        if (!Is_Float_Equal(new_camera_posx, 0.0f) || !Is_Float_Equal(new_camera_posy, 0.0f)) {
            pActive_Camera->Set_Pos(new_camera_posx - (game_res_w * 0.5f), new_camera_posy - (game_res_h * 0.5f));
        }
    }
    // Handle level-editor-specific commands
    else if (evt.key.code == sf::Keyboard::M) {
        if (!pMouseCursor->m_selected_objects.empty()) {
            cSprite* mouse_obj = pMouseCursor->m_selected_objects[0]->m_obj;

            // change state of the base object
            if (cycle_object_massive_type(mouse_obj)) {
                // change selected objects state to the base object state
                for (SelectedObjectList::iterator itr = pMouseCursor->m_selected_objects.begin(); itr != pMouseCursor->m_selected_objects.end(); ++itr) {
                    cSprite* obj = (*itr)->m_obj;

                    // skip base object
                    if (obj == mouse_obj) {
                        continue;
                    }

                    // set state
                    obj->Set_Massive_Type(mouse_obj->m_massive_type);
                }
            }
        }
    }
    else {
        // not processed
        return false;
    }

    // key got processed
    return true;
}

void cEditor_Level::Set_Level(cLevel* p_level)
{
    mp_level = p_level;
    m_settings_screen.Set_Level(p_level);
}

bool cEditor_Level::Function_New(void)
{
    std::string level_name = Box_Text_Input(_("Create a new Level"), C_("level", "Name"));

    // aborted
    if (level_name.empty()) {
        return 0;
    }

    // if it already exists
    if (!pLevel_Manager->Get_Path(level_name, true).empty()) {
        pHud_Debug->Set_Text(_("Level ") + level_name + _(" already exists"));
        return 0;
    }

    Game_Action = GA_ENTER_LEVEL;
    Game_Action_Data_Start.add("music_fadeout", "1000");
    Game_Action_Data_Start.add("screen_fadeout", int_to_string(EFFECT_OUT_BLACK));
    Game_Action_Data_Start.add("screen_fadeout_speed", "3");
    Game_Action_Data_Middle.add("new_level", level_name.c_str());
    Game_Action_Data_End.add("screen_fadein", int_to_string(EFFECT_IN_RANDOM));
    Game_Action_Data_End.add("screen_fadein_speed", "3");

    pHud_Debug->Set_Text(_("Created ") + level_name);
    return 1;
}

void cEditor_Level::Function_Load(void)
{
    std::string level_name = C_("level", "Name");

    // valid level
    while (level_name.length()) {
        level_name = Box_Text_Input(level_name, _("Load a Level"), level_name.compare(C_("level", "Name")) == 0 ? 1 : 0);

        // aborted
        if (level_name.empty()) {
            break;
        }

        // if available
        boost::filesystem::path level_path = pLevel_Manager->Get_Path(level_name);
        if (!level_path.empty()) {
            Game_Action = GA_ENTER_LEVEL;
            Game_Mode_Type = MODE_TYPE_LEVEL_CUSTOM;
            Game_Action_Data_Start.add("screen_fadeout", int_to_string(EFFECT_OUT_BLACK_TILED_RECTS));
            Game_Action_Data_Start.add("screen_fadeout_speed", "3");
            Game_Action_Data_Middle.add("load_level", level_name.c_str());
            Game_Action_Data_Middle.add("reset_save", "1");
            Game_Action_Data_End.add("screen_fadein", int_to_string(EFFECT_IN_BLACK));
            Game_Action_Data_End.add("screen_fadein_speed", "3");

            pHud_Debug->Set_Text(_("Loaded ") + path_to_utf8(Trim_Filename(level_path, 0, 0)));

            break;
        }
        // not found
        else {
            pAudio->Play_Sound("error.ogg");
        }
    }
}

void cEditor_Level::Function_Save(bool with_dialog /* = 0 */)
{
    // not loaded
    if (!pActive_Level->Is_Loaded()) {
        return;
    }

    // if denied
    if (with_dialog && !Box_Question(_("Save ") + pActive_Level->Get_Level_Name() + " ?")) {
        return;
    }

    pActive_Level->Save();
}

void cEditor_Level::Function_Save_as(void)
{
    std::string levelname = Box_Text_Input(_("Save Level as"), _("New name"), 1);

    // aborted
    if (levelname.empty()) {
        return;
    }

    pActive_Level->Set_Filename(levelname, 0);
    pActive_Level->Save();
}

void cEditor_Level::Function_Delete(void)
{
    std::string levelname = pActive_Level->Get_Level_Name();
    if (pLevel_Manager->Get_Path(levelname, true).empty()) {
        pHud_Debug->Set_Text(_("Level was not yet saved"));
        return;
    }

    // if denied
    if (!Box_Question(_("Delete and Unload ") + levelname + " ?")) {
        return;
    }

    pActive_Level->Delete();
    Disable();

    Game_Action = GA_ENTER_MENU;
    Game_Action_Data_Start.add("music_fadeout", "1000");
    Game_Action_Data_Start.add("screen_fadeout", int_to_string(EFFECT_OUT_BLACK));
    Game_Action_Data_Start.add("screen_fadeout_speed", "3");
    Game_Action_Data_Middle.add("load_menu", int_to_string(MENU_MAIN));
    if (Game_Mode_Type != MODE_TYPE_LEVEL_CUSTOM) {
        Game_Action_Data_Middle.add("menu_exit_back_to", int_to_string(MODE_OVERWORLD));
    }
    Game_Action_Data_End.add("screen_fadein", int_to_string(EFFECT_IN_BLACK));
    Game_Action_Data_End.add("screen_fadein_speed", "3");
}

void cEditor_Level::Function_Reload(void)
{
    // if denied
    if (!Box_Question(_("Reload Level ?"))) {
        return;
    }

    // Simulate level ending followed by loading the level from scratch
    // (cf. cLevel_Manager::Finish_Level)
    Game_Action = GA_ENTER_LEVEL;
    pHud_Time->Reset();
    pLevel_Player->Clear_Return();

    // Remove old level
    Game_Action_Data_Start.add("music_fadeout", "1500");
    Game_Action_Data_Start.add("screen_fadeout", int_to_string(EFFECT_OUT_RANDOM));
    Game_Action_Data_Middle.add("unload_levels", "1");

    // Load new level
    Game_Action_Data_Middle.add("load_level", path_to_utf8(pActive_Level->m_level_filename.filename()));
    Game_Action_Data_End.add("screen_fadein", int_to_string(EFFECT_IN_RANDOM));
}

void cEditor_Level::Function_Settings(void)
{
    Game_Action = GA_ENTER_LEVEL_SETTINGS;
    Game_Action_Data_Start.add("screen_fadeout", int_to_string(EFFECT_OUT_BLACK));
    Game_Action_Data_Start.add("screen_fadeout_speed", "3");
    Game_Action_Data_End.add("screen_fadein", int_to_string(EFFECT_IN_BLACK));
    Game_Action_Data_End.add("screen_fadein_speed", "3");
}

std::vector<cSprite*> cEditor_Level::items_loader_callback(const std::string& name, XmlAttributes& attributes, int engine_version, cSprite_Manager* p_sprite_manager, void* p_data)
{
    return cLevelLoader::Create_Level_Objects_From_XML_Tag(name, attributes, engine_version, p_sprite_manager);
}

std::vector<cSprite*> cEditor_Level::Parse_Items_File()
{
    cEditorItemsLoader parser;
    parser.parse_file(pResource_Manager->Get_Game_Editor("level_items.xml"), &m_sprite_manager, NULL, items_loader_callback);
    return parser.get_tagged_sprites();
}

bool cEditor_Level::cycle_object_massive_type(cSprite* obj) const
{
    // empty object or lava
    if (!obj || obj->m_sprite_array == ARRAY_LAVA) {
        return 0;
    }

    if (obj->m_massive_type == MASS_FRONT_PASSIVE) {
        obj->Set_Massive_Type(MASS_MASSIVE);
    }
    else if (obj->m_massive_type == MASS_MASSIVE) {
        obj->Set_Massive_Type(MASS_HALFMASSIVE);
    }
    else if (obj->m_massive_type == MASS_HALFMASSIVE) {
        obj->Set_Massive_Type(MASS_CLIMBABLE);
    }
    else if (obj->m_massive_type == MASS_CLIMBABLE) {
        obj->Set_Massive_Type(MASS_PASSIVE);
    }
    else if (obj->m_massive_type == MASS_PASSIVE) {
        obj->Set_Massive_Type(MASS_FRONT_PASSIVE);
    }
    // invalid object type
    else {
        return 0;
    }

    return 1;
}

#endif
