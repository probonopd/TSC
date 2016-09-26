#include "../core/global_basic.hpp"
#include "../video/color.hpp"
#include "../objects/sprite.hpp"
#include "../objects/movingsprite.hpp"
#include "../core/i18n.hpp"
#include "hud.hpp"

// extern variables
TSC::cHud* TSC::gp_hud = NULL;

using namespace TSC;

cHud::cHud()
    : m_points(0), m_jewels(0), m_lives(3),
      mp_rescue_item(NULL), m_waypoint_name(""),
      m_elapsed_time(0), m_last_time(std::chrono::system_clock::now()),
      mp_hud_root(NULL), mp_points_label(NULL), mp_time_label(NULL),
      mp_jewels_label(NULL), mp_lives_label(NULL)
{
    load_hud_images_into_cegui();

    mp_hud_root = static_cast<CEGUI::FrameWindow*>(
        CEGUI::WindowManager::getSingleton()
        .loadLayoutFromFile("hud.layout"));

    mp_points_label = mp_hud_root->getChild("points");
    mp_time_label   = mp_hud_root->getChild("time");
    mp_jewels_label = mp_hud_root->getChild("jewels");
    mp_lives_label = mp_hud_root->getChild("lives");

    mp_hud_root->hide();
    CEGUI::System::getSingleton()
        .getDefaultGUIContext()
        .getRootWindow()
        ->addChild(mp_hud_root);

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

    while (m_jewels > 100) {
        m_jewels -= 100;
        // TODO: Fire event
    }

    char str[8];
    memset(str, '\0', 8);
    sprintf(str, "%02d", m_jewels);
    mp_jewels_label->setText(str);
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

    char str[8];
    memset(str, '\0', 8);
    sprintf(str, "%02d x", m_lives);
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
}

void cHud::Request_Item(void)
{
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
