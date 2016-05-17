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
#include "../overworld/overworld.hpp"
#include "../overworld/world_player.hpp"
#include "../overworld/world_manager.hpp"
#include "../video/renderer.hpp"
#include "../core/sprite_manager.hpp"
#include "../overworld/overworld.hpp"
#include "../core/i18n.hpp"
#include "../core/filesystem/filesystem.hpp"
#include "../core/filesystem/resource_manager.hpp"
#include "../core/errors.hpp"
#include "world_editor.hpp"

#ifdef ENABLE_NEW_EDITOR

using namespace TSC;

// extern
cEditor_World* TSC::pWorld_Editor = NULL;

cEditor_World::cEditor_World()
    : cEditor()
{
    mp_overworld = NULL;
    m_editor_item_tag = "world";
    m_menu_filename = pResource_Manager->Get_Game_Editor("world_menu.xml");
}

cEditor_World::~cEditor_World()
{
}

void cEditor_World::Set_World(cOverworld* p_world)
{
    mp_overworld = p_world;
}

#endif
