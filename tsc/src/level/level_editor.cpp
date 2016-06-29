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
#include "../overworld/world_manager.hpp"
#include "../video/renderer.hpp"
#include "../core/sprite_manager.hpp"
#include "../overworld/overworld.hpp"
#include "../core/i18n.hpp"
#include "../core/filesystem/filesystem.hpp"
#include "../core/filesystem/resource_manager.hpp"
#include "../core/errors.hpp"
#include "level_editor.hpp"

#ifdef ENABLE_NEW_EDITOR

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

void cEditor_Level::Set_Level(cLevel* p_level)
{
    mp_level = p_level;
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

#endif
