#include "../core/global_basic.hpp"
#include "../core/global_game.hpp"
#include "../core/game_core.hpp"
#include "../video/color.hpp"
#include "../objects/sprite.hpp"
#include "../objects/movingsprite.hpp"
#include "../core/i18n.hpp"
#include "../audio/audio.hpp"
#include "../video/gl_surface.hpp"
#include "../level/level.hpp"
#include "../level/level_player.hpp"
#include "../objects/powerup.hpp"
#include "../objects/rescue_item.hpp"
#include "../scripting/events/gold_100_event.hpp"
#include "../core/property_helper.hpp"
#include "../core/filesystem/resource_manager.hpp"
#include "../core/sprite_manager.hpp"
#include "hud.hpp"

// 35 is the number of pixels set in berry's .settings file.
#define BERRY_WIDTH  35 * global_upscalex
#define BERRY_HEIGHT 35 * global_upscaley

// 48px is just slightly bigger.
#define ITEMBOX_WIDTH 48 * global_upscalex
#define ITEMBOX_HEIGHT 48 * global_upscaley

// extern variables
TSC::cHud* TSC::gp_hud = NULL;

using namespace TSC;
namespace fs = boost::filesystem;

cHud::cHud()
    : m_points(0), m_jewels(0), m_lives(3),
      mp_display_item(NULL), m_rescue_item_type(TYPE_UNDEFINED),
      m_waypoint_name(""),
      m_elapsed_time(0), m_last_time(std::chrono::system_clock::now()),
      mp_hud_root(NULL), mp_points_label(NULL), mp_time_label(NULL),
      mp_jewels_label(NULL), mp_lives_label(NULL), mp_item_image(NULL)
{
    load_hud_images_into_cegui();

    mp_hud_root = static_cast<CEGUI::FrameWindow*>(
        CEGUI::WindowManager::getSingleton()
        .loadLayoutFromFile("hud.layout"));

    mp_points_label = mp_hud_root->getChild("points");
    mp_time_label   = mp_hud_root->getChild("time");
    mp_jewels_label = mp_hud_root->getChild("jewels");
    mp_lives_label  = mp_hud_root->getChild("lives");
    mp_item_image   = mp_hud_root->getChild("itembox_image/item_image");

    mp_hud_root->hide();
    CEGUI::System::getSingleton()
        .getDefaultGUIContext()
        .getRootWindow()
        ->addChild(mp_hud_root);

    // Size & position the item box and its contents
    mp_hud_root->getChild("itembox_image")->setSize(CEGUI::USize(CEGUI::UDim(0, ITEMBOX_WIDTH),
                                                                 CEGUI::UDim(0, ITEMBOX_HEIGHT)));

    mp_item_image->setArea(CEGUI::UDim(0.5, -(0.5*BERRY_WIDTH)),
                           CEGUI::UDim(0.5, -(0.5*BERRY_HEIGHT)),
                           CEGUI::UDim(0, BERRY_WIDTH),
                           CEGUI::UDim(0, BERRY_HEIGHT));

    mp_item_image->hide();


    // Set initial values. Using methods rather than bare assignment
    // so translations can kick in for the HUD elements.
    Set_Points(0);
    Set_Jewels(0);
    Set_Elapsed_Time(0);
    Set_Lives(3);
}

cHud::~cHud()
{
    CEGUI::System::getSingleton()
        .getDefaultGUIContext()
        .getRootWindow()
        ->removeChild(mp_hud_root);

    CEGUI::WindowManager::getSingleton().destroyWindow(mp_hud_root);
}

void cHud::Show()
{
    mp_hud_root->show();
}

void cHud::Hide()
{
    mp_hud_root->hide();
}

void cHud::Update()
{
    static char timestr[32];
    static int seconds;

    // Do nothing if not shown anyway
    if (!mp_hud_root->isVisible())
        return;

    // Increase playtime
    std::chrono::system_clock::time_point time_now = std::chrono::system_clock::now();
    std::chrono::milliseconds time_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(time_now - m_last_time);

    m_elapsed_time += time_elapsed.count();
    m_last_time     = time_now;
    seconds         = m_elapsed_time / 1000;

    memset(timestr, '\0', 32);
    sprintf(timestr, _("Time %02d:%02d"), seconds / 60, seconds % 60);
    mp_time_label->setText(timestr);
}

void cHud::Set_Points(long points)
{
    m_points = points;

    char str[32];
    memset(str, '\0', 32);
    sprintf(str, _("Points %08ld"), m_points);
    mp_points_label->setText(str);
}

void cHud::Add_Points(long points, float /* x = 0.0f */, float y /* = 0.0f */, std::string strtext /* = "" */, const Color& color /* = 255 */, bool allow_multiplier /* = false */)
{
    Set_Points(m_points + points);
}

void cHud::Reset_Points()
{
    Set_Points(0);
}

long cHud::Get_Points()
{
    return m_points;
}

void cHud::Set_Jewels(int jewels)
{
    m_jewels = jewels;

    while (m_jewels >= 100) {
        m_jewels -= 100;
        pAudio->Play_Sound("item/live_up_2.ogg", RID_1UP_MUSHROOM);
        Add_Lives(1);

        // Display "1UP"
        Add_Points(0, pLevel_Player->m_pos_x + pLevel_Player->m_image->m_w/3, pLevel_Player->m_pos_y + 5, "1UP", lightred);

        // Fire the Gold100 event
        Scripting::cGold_100_Event evt;
        evt.Fire(pActive_Level->m_mruby, pLevel_Player);
    }

    char str[8];
    memset(str, '\0', 8);
    sprintf(str, "%02d", m_jewels);
    mp_jewels_label->setText(str);

    // Change text colour with more and more jewels collected
    // OLD Color color = Color(static_cast<uint8_t>(255), 255, 255 - (gold * 2));
}

void cHud::Add_Jewels(int jewels)
{
    Set_Jewels(m_jewels + jewels);
}

void cHud::Reset_Jewels()
{
    Set_Jewels(0);
}

int cHud::Get_Jewels()
{
    return m_jewels;
}

void cHud::Set_Lives(int lives)
{
    m_lives = lives;

    if (m_lives > 99)
        m_lives = 99;

    char str[32];
    memset(str, '\0', 32);
    sprintf(str, "[colour='FF00FF00']%02d x", m_lives);
    mp_lives_label->setText(str);
}

void cHud::Add_Lives(int lives)
{
    Set_Lives(m_lives + lives);
}

void cHud::Reset_Lives()
{
    Set_Lives(3);
}

int cHud::Get_Lives()
{
    return m_lives;
}

void cHud::Set_Elapsed_Time(uint32_t milliseconds)
{
    m_elapsed_time = milliseconds;
}

void cHud::Reset_Elapsed_Time()
{
    m_elapsed_time = 0;
    m_last_time = std::chrono::system_clock::now();
}

uint32_t cHud::Get_Elapsed_Time()
{
    return m_elapsed_time;
}

void cHud::Set_Item(SpriteType item_type, bool sound /* = true */)
{
    if (Game_Mode != MODE_LEVEL)
        throw(std::runtime_error("cHud::Set_Item() may only be called in level mode!"));

    // play sound
    if (sound) {
        pAudio->Play_Sound("itembox_set.ogg");
    }

    m_rescue_item_type = item_type;

    switch(m_rescue_item_type) {
    case TYPE_MUSHROOM_DEFAULT:
        mp_item_image->setProperty("Image", m_normal_berry_img);
        break;
    case TYPE_MUSHROOM_BLUE:
        mp_item_image->setProperty("Image", m_ice_berry_img);
        break;
    case TYPE_FIREPLANT:
        mp_item_image->setProperty("Image", m_fire_berry_img);
        break;
    default:
        std::cerr << "Warning: Unsupported item type stored in HUD item box, changing to normal berry" << std::endl;
        m_rescue_item_type = TYPE_MUSHROOM_DEFAULT;
        mp_item_image->setProperty("Image", m_normal_berry_img);
        break;
    }

    mp_item_image->show();
}

void cHud::Request_Item(void)
{
    if (Game_Mode != MODE_LEVEL)
        throw(std::runtime_error("cHud::Reqeust_Item() may only be called in level mode!"));

    // Currently no rescue item
    if (m_rescue_item_type == TYPE_UNDEFINED)
        return;

    GL_rect cam_rect = pLevel_Manager->m_camera->Get_Rect();

    // Spawn rescue item
    cRescueItem* p_item = new cRescueItem(pActive_Level->m_sprite_manager);
    p_item->Set_Item_Type(m_rescue_item_type);
    p_item->Set_Pos(cam_rect.m_x + (cam_rect.m_w / 2.0f) - (BERRY_WIDTH / 2.0f),
                    cam_rect.m_y);
    pActive_Level->m_sprite_manager->Add(p_item);

    pAudio->Play_Sound("itembox_get.ogg");

    // Clear rescue item
    m_rescue_item_type = TYPE_UNDEFINED;
    mp_item_image->hide();
}

void cHud::Reset_Item(void)
{
}

SpriteType cHud::Get_Item(void)
{
    return TYPE_FIREPLANT;
}

void cHud::Set_Waypoint_Name(std::string name)
{
    m_waypoint_name = name;
}

std::string& cHud::Get_Waypoint_Name()
{
    return m_waypoint_name;
}

void cHud::load_hud_images_into_cegui()
{
    CEGUI::ImageManager& imgmanager = CEGUI::ImageManager::getSingleton();

    imgmanager.addFromImageFile("hud_jewel",   "game/hud_jewel.png",   "ingame-images");
    imgmanager.addFromImageFile("hud_alex",    "game/hud_alex.png",    "ingame-images");
    imgmanager.addFromImageFile("hud_itembox", "game/hud_itembox.png", "ingame-images");

    m_normal_berry_img = path_to_utf8(pResource_Manager->Get_User_Pixmap("game/items/mushroom_red.png"));
    m_fire_berry_img   = path_to_utf8(pResource_Manager->Get_User_Pixmap("game/items/fireberry_1.png"));
    m_ice_berry_img    = path_to_utf8(pResource_Manager->Get_User_Pixmap("game/items/mushroom_blue.png"));

    // Convert the pathes to the names they are stored under in CEGUI
    // (see cEditor::load_cegui_image(), which is executed on game startup).
    string_replace_all(m_normal_berry_img, "/", "+");
    string_replace_all(m_fire_berry_img,   "/", "+");
    string_replace_all(m_ice_berry_img,    "/", "+");
}

cHudSprite::cHudSprite(cSprite_Manager* sprite_manager)
    : cSprite(sprite_manager)
{
    m_camera_range = 0;
    m_pos_z = 0.13f;

    Set_Ignore_Camera(1);
}

cHudSprite::~cHudSprite(void)
{

}

cHudSprite* cHudSprite::Copy(void) const
{
    cHudSprite* hud_sprite = new cHudSprite(m_sprite_manager);
    hud_sprite->Set_Image(m_start_image);
    hud_sprite->Set_Pos(m_start_pos_x, m_start_pos_y, 1);
    hud_sprite->m_type = m_type;
    hud_sprite->m_sprite_array = m_sprite_array;
    hud_sprite->m_massive_type = m_massive_type;
    hud_sprite->Set_Ignore_Camera(m_no_camera);
    hud_sprite->Set_Shadow_Pos(m_shadow_pos);
    hud_sprite->Set_Shadow_Color(m_shadow_color);
    return hud_sprite;
}
