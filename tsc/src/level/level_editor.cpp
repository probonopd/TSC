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

#endif
