#include "../core/global_basic.hpp"
#include "../video/color.hpp"
#include "../objects/sprite.hpp"
#include "../objects/movingsprite.hpp"
#include "hud.hpp"

// extern variables
TSC::cHud* TSC::gp_hud = NULL;

using namespace TSC;

cHud::cHud()
    : m_points(0), m_jewels(0), m_lives(3),
      mp_rescue_item(NULL), m_waypoint_name(""),
      mp_hud_root(NULL)
{
    mp_hud_root = static_cast<CEGUI::FrameWindow*>(CEGUI::WindowManager::getSingleton().createWindow("TaharezLook/FrameWindow", "hud"));

    mp_hud_root->hide();
    CEGUI::System::getSingleton()
        .getDefaultGUIContext()
        .getRootWindow()
        ->addChild(mp_hud_root);
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
    // Do nothing if not shown anyway
    if (!mp_hud_root->isVisible())
        return;
}

void cHud::Set_Points(long points)
{
    m_points = points;
}

void cHud::Add_Points(long points, float /* x = 0.0f */, float y /* = 0.0f */, std::string strtext /* = "" */, const Color& color /* = 255 */, bool allow_multiplier /* = false */)
{
    m_points += points;
}

void cHud::Reset_Points()
{
    m_points = 0;
}

long cHud::Get_Points()
{
    return m_points;
}

void cHud::Set_Jewels(int jewels)
{
    m_jewels = jewels;
}

void cHud::Add_Jewels(int jewels)
{
    m_jewels += jewels;

    while (m_jewels > 100) {
        m_jewels -= 100;
        // TODO: Fire event
    }
}

void cHud::Reset_Jewels()
{
    m_jewels = 0;
}

int cHud::Get_Jewels()
{
    return m_jewels;
}

void cHud::Set_Lives(int lives)
{
    m_lives = lives;
}

void cHud::Add_Lives(int lives)
{
    m_lives += lives;

    if (m_lives > 99)
        m_lives = 99;
}

void cHud::Reset_Lives()
{
    m_lives = 3;
}

int cHud::Get_Lives()
{
    return m_lives;
}

void cHud::Set_Time(uint32_t milliseconds)
{
}

void cHud::Reset_Time()
{
}

uint32_t cHud::Get_Time()
{
    return 1000;
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
