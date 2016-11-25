#include "../core/global_basic.hpp"
#include "../core/game_core.hpp"
#include "../gui/generic.hpp"
#include "../core/framerate.hpp"
#include "../audio/audio.hpp"
#include "../video/animation.hpp"
#include "../input/keyboard.hpp"
#include "../input/mouse.hpp"
#include "../input/joystick.hpp"
#include "../user/preferences.hpp"
#include "../overworld/overworld.hpp"
#include "../overworld/world_player.hpp"
#include "../overworld/world_manager.hpp"
#include "../overworld/overworld_loader.hpp"
#include "../video/renderer.hpp"
#include "../core/sprite_manager.hpp"
#include "../overworld/overworld.hpp"
#include "../core/i18n.hpp"
#include "../core/filesystem/filesystem.hpp"
#include "../core/filesystem/resource_manager.hpp"
#include "../core/editor/editor_items_loader.hpp"
#include "../core/errors.hpp"
#include "world_editor.hpp"

#ifdef ENABLE_EDITOR

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

void cEditor_World::Enable(cSprite_Manager* p_sprite_manager)
{
    if (m_enabled)
        return;

    cEditor::Enable(p_sprite_manager);
    editor_world_enabled = true;
}

void cEditor_World::Disable(void)
{
    if (!m_enabled)
        return;

    cEditor::Disable();
    editor_world_enabled = false;
}

void cEditor_World::Set_World(cOverworld* p_world)
{
    mp_overworld = p_world;
}

bool cEditor_World::Function_New(void)
{
    std::string world_name = Box_Text_Input(_("Create a new World"), C_("world", "Name"));

    // aborted/invalid
    if (world_name.empty()) {
        return 0;
    }

    if (pOverworld_Manager->New(world_name)) {
        Game_Action = GA_ENTER_WORLD;
        Game_Action_Data_Start.add("screen_fadeout", int_to_string(EFFECT_OUT_BLACK_TILED_RECTS));
        Game_Action_Data_Start.add("screen_fadeout_speed", "3");
        Game_Action_Data_Middle.add("enter_world", world_name.c_str());
        Game_Action_Data_End.add("screen_fadein", int_to_string(EFFECT_IN_BLACK));
        Game_Action_Data_End.add("screen_fadein_speed", "3");

        gp_hud->Set_Text(_("Created ") + world_name);
        return 1;
    }
    else {
        gp_hud->Set_Text(_("World ") + world_name + _(" already exists"));
    }

    return 0;
}

void cEditor_World::Function_Load(void)
{
    std::string world_name = C_("world", "Name");

    // valid world
    while (world_name.length()) {
        world_name = Box_Text_Input(world_name, _("Load an Overworld"), world_name.compare(C_("world", "Name")) == 0 ? 1 : 0);

        // break if empty
        if (world_name.empty()) {
            break;
        }

        // success
        if (pOverworld_Manager->Get(world_name)) {
            Game_Action = GA_ENTER_WORLD;
            Game_Action_Data_Start.add("screen_fadeout", int_to_string(EFFECT_OUT_BLACK_TILED_RECTS));
            Game_Action_Data_Start.add("screen_fadeout_speed", "3");
            Game_Action_Data_Middle.add("enter_world", world_name.c_str());
            Game_Action_Data_End.add("screen_fadein", int_to_string(EFFECT_IN_BLACK));
            Game_Action_Data_End.add("screen_fadein_speed", "3");

            gp_hud->Set_Text(_("Loaded ") + world_name);
            break;
        }
        // failed
        else {
            pAudio->Play_Sound("error.ogg");
        }
    }
}

void cEditor_World::Function_Save(bool with_dialog /* = 0 */)
{
    // if denied
    if (with_dialog && !Box_Question(_("Save ") + mp_overworld->m_description->m_name + " ?")) {
        return;
    }

    mp_overworld->Save();
}

void cEditor_World::Function_Reload(void)
{
    // if denied
    if (!Box_Question(_("Reload World ?"))) {
        return;
    }

    cOverworld* p_old_world = mp_overworld;
    mp_overworld = cOverworld::Load_From_Directory(p_old_world->m_description->Get_Path());
    delete p_old_world;
}

std::vector<cSprite*> cEditor_World::items_loader_callback(const std::string& name, XmlAttributes& attributes, int engine_version, cSprite_Manager* p_sprite_manager, void* p_data)
{
    cSprite* p_sprite = cOverworldLoader::Create_World_Object_From_XML(name, attributes, engine_version, p_sprite_manager, pActive_Overworld);

    std::vector<cSprite*> result;
    result.push_back(p_sprite);
    return result;
}

std::vector<cSprite*> cEditor_World::Parse_Items_File()
{
    cEditorItemsLoader parser;
    parser.parse_file(pResource_Manager->Get_Game_Editor("world_items.xml"), &m_sprite_manager, NULL, items_loader_callback);
    return parser.get_tagged_sprites();
    return std::vector<cSprite*>(); // FIXME ↑ Segfaults
}

#endif
