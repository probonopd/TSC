/***************************************************************************
 * hud.cpp  -  heads-up display
 *
 * Copyright © 2003 - 2011 Florian Richter
 * Copyright © 2013 - 2014 The TSC Contributors
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../gui/hud.hpp"
#include "../core/game_core.hpp"
#include "../level/level_player.hpp"
#include "../audio/audio.hpp"
#include "../video/font.hpp"
#include "../core/framerate.hpp"
#include "../level/level.hpp"
#include "../core/sprite_manager.hpp"
#include "../objects/bonusbox.hpp"
#include "../video/renderer.hpp"
#include "../core/i18n.hpp"
#include "../core/filesystem/filesystem.hpp"
#include "../scripting/events/gold_100_event.hpp"
#include "../core/global_basic.hpp"

#define HUD_LEVEL_Y 24.0f
#define HUD_WORLD_Y 4.0f

using namespace std;

namespace TSC {

/* *** *** *** *** *** *** *** cHudSprite *** *** *** *** *** *** *** *** *** *** */

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

/* *** *** *** *** *** *** *** cMiniPointsText *** *** *** *** *** *** *** *** *** *** */

cMiniPointsText::cMiniPointsText()
{
    m_vely = 0;
    m_disabled = false;
}

cMiniPointsText::~cMiniPointsText()
{
}

void cMiniPointsText::Draw()
{
    m_vely -= m_vely *  0.01f * pFramerate->m_speed_factor;
    m_text.move(0, m_vely * pFramerate->m_speed_factor);

    // disable
    if (m_vely > -1.0f) {
        Disable();
        return;
    }
    // fade out
    else if (m_vely > -1.2f) {
        sf::Color col = m_text.getColor();
        col.a = static_cast<uint8_t>(255 * -(m_vely + 1.0f) * 5);
        m_text.setColor(col);
    }

    // Convert level coordinate to window coordinate
    float x = m_text.getPosition().x - pActive_Camera->m_x;
    float y = m_text.getPosition().y - pActive_Camera->m_y;

    // out in left
    if (x < 0.0f) {
        x = 3.0f;
    }
    // out in right
    else if (x > game_res_w) {
        x = game_res_w - m_text.getGlobalBounds().width - 3.0f;
    }

    // out on top
    if (y < 0.0f) {
        y = 3.0f;
    }
    // out on bottom
    else if (y > game_res_h) {
        y = game_res_h - m_text.getGlobalBounds().height - 3.0f;
    }

    // create request
    pFont->Queue_Text(m_text);

    // OLD // shadow
    // OLD request->m_shadow_color = black;
    // OLD request->m_shadow_color.alpha = obj->m_color.alpha;
    // OLD request->m_shadow_pos = 1;

    // OLD // color
    // OLD request->m_color = Color(static_cast<uint8_t>(255 - (obj->m_points / 150)), static_cast<uint8_t>(255 - (obj->m_points / 150)), static_cast<uint8_t>(255 - (obj->m_points / 30)), obj->m_color.alpha);
}

/* *** *** *** *** *** *** *** cHud_Manager *** *** *** *** *** *** *** *** *** *** */

cHud_Manager::cHud_Manager(cSprite_Manager* sprite_manager)
{
    m_sprite_manager = sprite_manager;
    m_loaded = 0;
    mp_menu_background = NULL;
}

cHud_Manager::~cHud_Manager(void)
{
    Unload();
}

void cHud_Manager::Load(void)
{
    if (!m_loaded) {
        Unload();
    }
    else if (m_loaded) {
        Update_Text();
        return;
    }

    // Menu Background ( Alex head and the Goldpiece image )
    mp_menu_background = new cMenuBackground(m_sprite_manager);
    // Point Display
    pHud_Points = new cPlayerPoints();
    // Time Display
    pHud_Time = new cTimeDisplay();
    // Live Display
    pHud_Lives = new cLiveDisplay();
    // Gold Display
    pHud_Goldpieces = new cGoldDisplay();
    // Info Message Display
    pHud_Infomessage = new cInfoMessage();
    // Itembox
    pHud_Itembox = new cItemBox(m_sprite_manager);
    // FPS Display
    pHud_Fps = new cFpsDisplay();
    // Debug Display
    pHud_Debug = new cDebugDisplay(m_sprite_manager);

    m_loaded = 1;
}

void cHud_Manager::Unload(void)
{
    delete pHud_Lives;
    delete pHud_Goldpieces;
    delete pHud_Points;
    delete pHud_Time;
    delete pHud_Infomessage;
    delete pHud_Fps;
    delete pHud_Debug;
    delete pHud_Itembox;

    pHud_Lives = NULL;
    pHud_Goldpieces = NULL;
    pHud_Points = NULL;
    pHud_Time = NULL;
    pHud_Infomessage = NULL;
    pHud_Fps = NULL;
    pHud_Debug = NULL;
    pHud_Itembox = NULL;

    m_loaded = 0;
}

/**
 * This function positions the different elements of the HUD on
 * the screen. It is (contrary to what the name suggests) *not*
 * called once a frame.
 */
void cHud_Manager::Update_Text(void)
{
    // note : update the life display before updating the time display
    unsigned int window_width = pVideo->mp_window->getSize().x;

    if (mp_menu_background) {
        if (Game_Mode != MODE_OVERWORLD) {
            // goldpiece
            mp_menu_background->m_rect_goldpiece.m_y = mp_menu_background->m_rect_alex_head.m_y + 6.0f;
        }
        else {
            // goldpiece
            mp_menu_background->m_rect_goldpiece.m_y = 7.0f;
        }
    }

    if (pHud_Lives) {
        if (Game_Mode != MODE_OVERWORLD) {
            pHud_Lives->Set_Pos(window_width - window_width * 0.1f, HUD_LEVEL_Y);
        }
        else {
            pHud_Lives->Set_Pos(window_width - window_width / 7.5f, HUD_WORLD_Y);
        }

        pHud_Lives->Add_Lives(0);
    }

    if (pHud_Goldpieces) {
        if (Game_Mode != MODE_OVERWORLD) {
            pHud_Goldpieces->Set_Pos(360.0f, HUD_LEVEL_Y);
        }
        else {
            pHud_Goldpieces->Set_Pos(360.0f, HUD_WORLD_Y);
        }

        pHud_Goldpieces->Add_Gold(0);
    }

    if (pHud_Points) {
        if (Game_Mode != MODE_OVERWORLD) {
            pHud_Points->Set_Pos(50.0f, HUD_LEVEL_Y);
        }
        else {
            pHud_Points->Set_Pos(50.0f, HUD_WORLD_Y);
        }

        pHud_Points->Add_Points(0);
    }

    if (pHud_Time) {
        pHud_Time->Set_Pos(window_width * 0.70f, HUD_LEVEL_Y);
        pHud_Time->Update();
    }

    if (pHud_Infomessage) {
        pHud_Infomessage->Set_Pos(20.0f, 95.0f);
        pHud_Infomessage->Update();
    }

    if (pHud_Fps) {
        pHud_Fps->Set_Pos(window_width * 0.1f, 10.0f);
        pHud_Fps->Update();
    }

    if (pHud_Debug) {
        pHud_Debug->Set_Pos(window_width * 0.45f, 80.0f);
        pHud_Debug->Update();
    }

    if (pHud_Itembox) {
        pHud_Itembox->Set_Pos(game_res_w * 0.49f, 10.0f);
        pHud_Itembox->Update();
    }
}

void cHud_Manager::Update(void)
{
    // update HUD objects

    mp_menu_background->Update();
    pHud_Points->Update();
    pHud_Time->Update();
    pHud_Lives->Update();
    pHud_Goldpieces->Update();
    pHud_Infomessage->Update();
    pHud_Itembox->Update();
    pHud_Fps->Update();
    pHud_Debug->Update();
}

void cHud_Manager::Draw(void)
{
    // draw HUD objects

    mp_menu_background->Draw();
    pHud_Points->Draw();
    pHud_Time->Draw();
    pHud_Lives->Draw();
    pHud_Goldpieces->Draw();
    pHud_Infomessage->Draw();
    pHud_Itembox->Draw();
    pHud_Fps->Draw();
    pHud_Debug->Draw();
}

void cHud_Manager::Set_Sprite_Manager(cSprite_Manager* sprite_manager)
{
    m_sprite_manager = sprite_manager;

    // update HUD objects
    Update();
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cHud_Manager* pHud_Manager = NULL;

/* *** *** *** *** *** *** cMenuBackground *** *** *** *** *** *** *** *** *** *** *** */

cMenuBackground::cMenuBackground(cSprite_Manager* sprite_manager)
    : cHudSprite(sprite_manager)
{
    m_type = TYPE_HUD_BACKGROUND;
    m_sprite_array = ARRAY_HUD;
    m_name = "HUD Menu Background";

    m_alex_head = pVideo->Get_Package_Surface("game/alex_l.png");
    m_goldpiece = pVideo->Get_Package_Surface("game/gold_m.png");

    if (!m_alex_head || !m_goldpiece) {
        cerr << "Error : MenuBackground images loading failed" << endl;
        return;
    }

    // alex head
    m_rect_alex_head.m_x = game_res_w * 0.933f;
    m_rect_alex_head.m_y = 15.0f;
    // goldpiece
    m_rect_goldpiece.m_x = 250.0f;
    m_rect_goldpiece.m_y = 0.0f;
}

cMenuBackground::~cMenuBackground(void)
{
    //
}

void cMenuBackground::Draw(cSurface_Request* request /* = NULL */)
{
    if (editor_enabled || Game_Mode == MODE_MENU) {
        return;
    }

    // alex head
    if (Game_Mode != MODE_OVERWORLD) {
        m_alex_head->Blit(m_rect_alex_head.m_x, m_rect_alex_head.m_y, m_pos_z);
    }

    // goldpiece
    m_goldpiece->Blit(m_rect_goldpiece.m_x, m_rect_goldpiece.m_y, m_pos_z);
}

/* *** *** *** *** *** *** cStatusText *** *** *** *** *** *** *** *** *** *** *** */

cStatusText::cStatusText()
{
    m_x = 0;
    m_y = 0;
    // OLD Set_Shadow(black, 1.5f);
}

cStatusText::~cStatusText(void)
{
    //
}

/**
 * Prepares the wrapped sf::Text instance (`m_text` member) so that it is suitable
 * for drawing by SFML. Use this method instead of adjusting the sf::Text instance
 * directly where possible, because it does some conversions.
 *
 * This method is a convenience wrapper around cFont_Manager::Prepare_SFML_Text()
 * with the things we know to always be the same in all subclasses set automatically.
 *
 * When you subclass cStatusText(), you must call this method each time you want
 * to change the text or its attributes. Do not call it every frame, that is
 * unnecessary.
 */
void cStatusText::Prepare_Text_For_SFML(const std::string& text, int fontsize, Color color)
{
    pFont->Prepare_SFML_Text(m_text, text, m_x, m_y, fontsize, color, true);
}

void cStatusText::Update()
{
    // Nothing by default
}

void cStatusText::Draw()
{
    if (Game_Mode == MODE_MENU) {
        return;
    }

    // Subclasses are supposed to call Prepare_Text_For_SFML()
    // before Draw() gets called.
    pFont->Queue_Text(m_text);
}

/* *** *** *** *** *** *** cPlayerPoints *** *** *** *** *** *** *** *** *** *** *** */

cPlayerPoints::cPlayerPoints()
    : cStatusText()
{
    Set_Points(pLevel_Player->m_points);

    m_points_objects.reserve(50);
}

cPlayerPoints::~cPlayerPoints(void)
{
    Clear();
}

void cPlayerPoints::Draw()
{
    if (editor_enabled || Game_Mode == MODE_MENU) {
        return;
    }

    cStatusText::Draw();

    // draw small points (those next to enemies)
    for (std::vector<cMiniPointsText*>::iterator itr = m_points_objects.begin(); itr != m_points_objects.end();) {
        // get object pointer
        cMiniPointsText* obj = (*itr);

        // if finished
        if (obj->Is_Disabled()) {
            itr = m_points_objects.erase(itr);
            delete obj;
        }
        // active
        else {
            obj->Draw();
            ++itr;
        }
    }
}

void cPlayerPoints::Set_Points(long points)
{
    pLevel_Player->m_points = points;

    char text[70];
    sprintf(text, _("Points %08d"), static_cast<int>(pLevel_Player->m_points));

    Prepare_Text_For_SFML(text, cFont_Manager::FONTSIZE_NORMAL, white);
}

void cPlayerPoints::Add_Points(unsigned int points, float x /* = 0.0f */, float y /* = 0.0f */, std::string strtext /* = "" */, const Color& color /* = static_cast<uint8_t>(255) */, bool allow_multiplier /* = 0 */)
{
    if (allow_multiplier) {
        points = static_cast<unsigned int>(pLevel_Player->m_kill_multiplier * static_cast<float>(points));
    }

    Set_Points(pLevel_Player->m_points + points);

    if (Is_Float_Equal(x, 0.0f) || Is_Float_Equal(y, 0.0f) || m_points_objects.size() > 50) {
        return;
    }

    // if empty set the points as text
    if (strtext.empty()) {
        strtext = int_to_string(points);
    }

    cMiniPointsText* new_obj = new cMiniPointsText();
    pFont->Prepare_SFML_Text(new_obj->Get_Text(), strtext, x, y, cFont_Manager::FONTSIZE_SMALL, color);
    new_obj->Set_Vel_Y(-1.4f);

    // check if it collides with an already active points text
    for (std::vector<cMiniPointsText*>::const_iterator itr = m_points_objects.begin(); itr != m_points_objects.end(); ++itr) {
        cMiniPointsText* obj = (*itr);

        // If they collide, move our text to the right to ensure readability.
        if (new_obj->Get_Text().getGlobalBounds().intersects(obj->Get_Text().getGlobalBounds())) {
            new_obj->Get_Text().move(obj->Get_Text().getGlobalBounds().width + 5, 0);
        }

    }

    m_points_objects.push_back(new_obj);
}

void cPlayerPoints::Clear(void)
{
    for (std::vector<cMiniPointsText*>::iterator itr = m_points_objects.begin(); itr != m_points_objects.end(); ++itr) {
        delete *itr;
    }

    m_points_objects.clear();
}

/* *** *** *** *** *** cGoldDisplay *** *** *** *** *** *** *** *** *** *** *** *** */

cGoldDisplay::cGoldDisplay()
    : cStatusText()
{
    Set_Gold(pLevel_Player->m_goldpieces);
}

cGoldDisplay::~cGoldDisplay(void)
{
    //
}

void cGoldDisplay::Draw()
{
    if (editor_enabled || Game_Mode == MODE_MENU) {
        return;
    }

    cStatusText::Draw();
}

void cGoldDisplay::Set_Gold(int gold)
{
    while (gold >= 100) {
        gold -= 100;
        pAudio->Play_Sound("item/live_up_2.ogg", RID_1UP_MUSHROOM);
        pHud_Lives->Add_Lives(1);

        pHud_Points->Add_Points(0, pLevel_Player->m_pos_x + pLevel_Player->m_image->m_w/3, pLevel_Player->m_pos_y + 5, "1UP", lightred);

        // Fire the Gold100 event
        Scripting::cGold_100_Event evt;
        evt.Fire(pActive_Level->m_mruby, pLevel_Player);
    }

    pLevel_Player->m_goldpieces = gold;
    std::string text = int_to_string(pLevel_Player->m_goldpieces);

    Color color = Color(static_cast<uint8_t>(255), 255, 255 - (gold * 2));

    Prepare_Text_For_SFML(text, cFont_Manager::FONTSIZE_NORMAL, color);
}

void cGoldDisplay::Add_Gold(int gold)
{
    Set_Gold(pLevel_Player->m_goldpieces + gold);
}

/* *** *** *** *** *** cLiveDisplay *** *** *** *** *** *** *** *** *** *** *** *** */

cLiveDisplay::cLiveDisplay()
    : cStatusText()
{
    Set_Lives(pLevel_Player->m_lives);
}

cLiveDisplay::~cLiveDisplay(void)
{
    //
}

void cLiveDisplay::Draw()
{
    if (editor_enabled || Game_Mode == MODE_MENU) {
        return;
    }

    cStatusText::Draw();
}

void cLiveDisplay::Set_Lives(int lives)
{
    pLevel_Player->m_lives = lives;

    if (lives < 0) {
        return;
    }

    std::string text;

    // if not in Overworld
    if (Game_Mode != MODE_OVERWORLD) {
        text = int_to_string(pLevel_Player->m_lives) + "x";
    }
    else {
        text = _("Lives : ") + int_to_string(pLevel_Player->m_lives);
    }

    Prepare_Text_For_SFML(text, cFont_Manager::FONTSIZE_NORMAL, green);
}

void cLiveDisplay::Add_Lives(int lives)
{
    Set_Lives(pLevel_Player->m_lives + lives);
}

/* *** *** *** *** *** *** *** cTimeDisplay *** *** *** *** *** *** *** *** *** *** */

cTimeDisplay::cTimeDisplay()
    : cStatusText()
{
    Reset();
}

cTimeDisplay::~cTimeDisplay(void)
{
    //
}

void cTimeDisplay::Update(void)
{
    if (editor_enabled || Game_Mode == MODE_OVERWORLD || Game_Mode == MODE_MENU) {
        return;
    }

    m_milliseconds += pFramerate->m_elapsed_ticks;

    // not valid
    if (m_milliseconds == 0) {
        return;
    }

    const uint32_t seconds = m_milliseconds / 1000;

    // update is not needed
    if (seconds == m_last_update_seconds) {
        return;
    }

    m_last_update_seconds = seconds;

    const uint32_t minutes = seconds / 60;

    // Set new time
    sprintf(m_clocktext, _("Time %02d:%02d"), minutes, seconds - (minutes * 60));
    Prepare_Text_For_SFML(m_clocktext, cFont_Manager::FONTSIZE_NORMAL, white);
}

void cTimeDisplay::Draw()
{
    if (editor_enabled || Game_Mode == MODE_OVERWORLD || Game_Mode == MODE_MENU) {
        return;
    }

    cStatusText::Draw();
}

void cTimeDisplay::Reset(void)
{
    // TRANS: HUD Clock is reset to 00:00, the digits are shown a second later for 00:01.
    sprintf(m_clocktext, _("Time"));
    m_last_update_seconds = 1000;
    m_milliseconds = 0;
}

void cTimeDisplay::Set_Time(uint32_t milliseconds)
{
    m_milliseconds = milliseconds;
    Update();
}

/* *** *** *** *** *** *** cFpsDisplay *** *** *** *** *** *** *** *** *** *** *** */

cFpsDisplay::cFpsDisplay()
    : cStatusText()
{
    sprintf(m_fps_text, "Initialising...");
}

cFpsDisplay::~cFpsDisplay()
{
}

void cFpsDisplay::Update(void)
{
    cStatusText::Update();

    sprintf(m_fps_text, "FPS: best %d worst %d current %d average %u speedfactor %.4f",
            static_cast<int>(pFramerate->m_fps_best),
            static_cast<int>(pFramerate->m_fps_worst),
            static_cast<int>(pFramerate->m_fps),
            pFramerate->m_fps_average,
            pFramerate->m_speed_factor);

    Prepare_Text_For_SFML(m_fps_text, cFont_Manager::FONTSIZE_VERYSMALL, white);
}

void cFpsDisplay::Draw()
{
    if (!game_debug || (Game_Mode == MODE_LEVEL && pLevel_Player->m_alex_type == ALEX_DEAD)) {
        return;
    }

    cStatusText::Draw();
}

/* *** *** *** *** *** cInfoMessage *** *** *** *** *** *** *** *** *** *** *** */

cInfoMessage::cInfoMessage()
    : cStatusText()
{

    m_infotext = "Test";
    m_alpha = -1;
    m_display_time = 0.0f;

    m_background = new cMovingSprite(pActive_Level->m_sprite_manager); // TODO: Lifetime difference? Info message in overworlds?
    m_background->Set_Ignore_Camera(true);
    m_background->m_camera_range = 0;
    m_background->Set_Massive_Type(MASS_MASSIVE);
    m_background->m_pos_z = 0.1299f;
    m_background->Set_Image(pVideo->Get_Package_Surface("game/infomessage.png"));
}

cInfoMessage::~cInfoMessage()
{
    delete m_background;
}

void cInfoMessage::Set_Text(const std::string& text)
{
    m_infotext = text;
    m_display_time = 100.0f;
    m_alpha = 255.0f;

    Prepare_Text_For_SFML(m_infotext, cFont_Manager::FONTSIZE_NORMAL, yellow);
}

std::string cInfoMessage::Get_Text()
{
    return m_infotext;
}

void cInfoMessage::Update()
{
    cStatusText::Update();

    // Display the message a few seconds without fading out,
    // then start to fade it out. If m_alpha is <= 0, it
    // has been fade out completed.
    if (m_alpha > 0) {
        m_background->Set_Pos(0.0f, m_y - 20.0f);

        if (m_display_time > 0)
            m_display_time -= pFramerate->m_speed_factor;
        else
            m_alpha -= pFramerate->m_speed_factor * 5;
    }
}

void cInfoMessage::Draw()
{
    if (editor_enabled || Game_Mode == MODE_OVERWORLD || Game_Mode == MODE_MENU) {
        return;
    }

    if (m_alpha > 0) {
        m_background->Set_Color(255,255,255, static_cast<uint8_t>(m_alpha));
        m_background->Draw();
        cStatusText::Draw();
    }
}

/* *** *** *** *** *** *** *** cItemBox *** *** *** *** *** *** *** *** *** *** */

cItemBox::cItemBox(cSprite_Manager* sprite_manager)
    : cHudSprite(sprite_manager)
{
    m_type = TYPE_HUD_ITEMBOX;
    m_name = "HUD Itembox";

    Set_Image(pVideo->Get_Package_Surface("game/itembox.png"));
    // disable shadow
    Set_Shadow_Pos(0);

    m_box_color = white;

    m_item_counter = 0;
    m_item_counter_mod = 0;
    m_item_id = TYPE_UNDEFINED;

    m_item = new cMovingSprite(sprite_manager);
    m_item->Set_Ignore_Camera(1);
    m_item->m_camera_range = 0;
    m_item->Set_Massive_Type(MASS_MASSIVE);
    m_item->m_pos_z = 0.1299f;
}

cItemBox::~cItemBox(void)
{
    delete m_item;
}

void cItemBox::Set_Sprite_Manager(cSprite_Manager* sprite_manager)
{
    cHudSprite::Set_Sprite_Manager(sprite_manager);
    m_item->Set_Sprite_Manager(sprite_manager);
}

void cItemBox::Update(void)
{
    if (!m_item_counter) {
        return;
    }

    m_item->Move(0.0f, 4.0f);

    if (m_item_counter_mod) {
        m_item_counter += pFramerate->m_speed_factor * 10.0f;

        if (m_item_counter >= 90.0f) {
            m_item_counter_mod = 0;
            m_item_counter = 90.0f;
        }
    }
    else {
        m_item_counter -= pFramerate->m_speed_factor * 10.0f;

        if (m_item_counter <= 0.0f) {
            m_item_counter_mod = 1;
            m_item_counter = 1.0f;
        }
    }

    if (m_item->m_pos_y > pLevel_Manager->m_camera->m_limit_rect.m_y) {
        Reset();
    }

    cObjectCollisionType* col_list = m_item->Collision_Check(&m_item->m_col_rect, COLLIDE_ONLY_BLOCKING);

    // if colliding with the player
    if (col_list->Is_Included(TYPE_PLAYER)) {
        // player can send an item back
        SpriteType item_id_temp = m_item_id;
        Reset();
        pLevel_Player->Get_Item(item_id_temp, 1);
    }

    delete col_list;
}

void cItemBox::Draw(cSurface_Request* request /* = NULL */)
{
    if (editor_enabled || Game_Mode == MODE_OVERWORLD || Game_Mode == MODE_MENU) {
        return;
    }

    if (m_item_id && m_item->m_image) {
        // with alpha
        if (m_item_counter) {
            m_item->Set_Color(255, 255, 255, 100 + static_cast<uint8_t>(m_item_counter));
        }

        m_item->Draw();
    }

    Set_Color(m_box_color.red, m_box_color.green, m_box_color.blue);
    cHudSprite::Draw();
}

void cItemBox::Set_Item(SpriteType item_type, bool sound /* = 1 */)
{
    // play sound
    if (sound) {
        pAudio->Play_Sound("itembox_set.ogg");
    }
    // reset data
    Reset();

    // reset startposition
    m_item->Set_Pos(0.0f, 0.0f, 1);
    // reset color
    m_item->Set_Color(white);

    if (item_type == TYPE_MUSHROOM_DEFAULT) {
        m_box_color = Color(static_cast<uint8_t>(250), 50, 50);
        m_item->Set_Image(pVideo->Get_Package_Surface("game/items/mushroom_red.png"));
    }
    else if (item_type == TYPE_FIREPLANT) {
        m_box_color = Color(static_cast<uint8_t>(250), 200, 150);
        m_item->Set_Image(pVideo->Get_Package_Surface("game/items/fireberry_1.png"));
    }
    else if (item_type == TYPE_MUSHROOM_BLUE) {
        m_box_color = Color(static_cast<uint8_t>(100), 100, 250);
        m_item->Set_Image(pVideo->Get_Package_Surface("game/items/mushroom_blue.png"));
    }


    if (m_item->m_image) {
        m_item->Set_Pos(m_pos_x - ((m_item->m_image->m_w - m_rect.m_w) / 2), m_pos_y - ((m_item->m_image->m_h - m_rect.m_h) / 2));
    }

    m_item_id = item_type;
}

void cItemBox::Request_Item(void)
{
    if (!m_item_id || m_item_counter) {
        return;
    }

    pAudio->Play_Sound("itembox_get.ogg");

    m_item_counter = 255.0f;
    // draw item with camera
    m_item->Set_Ignore_Camera(0);
    m_item->Set_Pos(m_item->m_pos_x + pActive_Camera->m_x, m_item->m_pos_y + pActive_Camera->m_y);
}

void cItemBox::Push_back(void)
{
    m_item_counter = 0.0f;
    m_item_counter_mod = 0;

    // draw item without camera
    m_item->Set_Ignore_Camera(1);
    m_item->Set_Pos(m_item->m_start_pos_x, m_item->m_start_pos_y);
    m_item->Set_Color(white);
}

void cItemBox::Reset(void)
{
    m_item->Set_Ignore_Camera(1);
    m_item_id = TYPE_UNDEFINED;
    m_item_counter = 0.0f;
    m_item_counter_mod = 0;
    m_box_color = white;
}

/* *** *** *** *** *** *** cDebugDisplay *** *** *** *** *** *** *** *** *** *** *** */

cDebugDisplay::cDebugDisplay(cSprite_Manager* sprite_manager)
    : cHudSprite(sprite_manager)
{
    m_type = TYPE_HUD_DEBUG;
    m_name = "HUD Debug";

    // debug text
    m_text_debug_text = CEGUI::WindowManager::getSingleton().loadLayoutFromFile("debugtext.layout");
    CEGUI::System::getSingleton().getDefaultGUIContext().getRootWindow()->addChild(m_text_debug_text);
    // hide
    m_text_debug_text->setVisible(0);

    m_counter = 0.0f;
}

cDebugDisplay::~cDebugDisplay(void)
{
    CEGUI::System::getSingleton().getDefaultGUIContext().getRootWindow()->removeChild(m_text_debug_text);
    CEGUI::WindowManager::getSingleton().destroyWindow(m_text_debug_text);
}

void cDebugDisplay::Update(void)
{
    // no text to display
    if (m_text.empty()) {
        return;
    }

    // if display time passed hide the text display
    if (m_counter <= 0) {
        m_text.clear();
        m_text_old.clear();

        m_text_debug_text->setVisible(0);
        return;
    }

    // update counter
    m_counter -= pFramerate->m_speed_factor;

    // set new text
    if (m_text.compare(m_text_old) != 0) {
        m_text_old = m_text;
        CEGUI::String gui_text = reinterpret_cast<const CEGUI::utf8*>(m_text.c_str());

        // display the new text
        m_text_debug_text->setText(gui_text);
        m_text_debug_text->setVisible(1);

        // update position
        CEGUI::Font* font = &CEGUI::FontManager::getSingleton().get("DejaVuSans");
        float text_width = font->getTextExtent(gui_text) * global_downscalex;
        float text_height = font->getLineSpacing() * global_downscaley;

        // fixme : works only correctly for one too long line
        if (text_width > 800.0f) {
            // add wrapped newlines
            text_height *= 1 + static_cast<int>(text_width / 800.0f);
            text_width = 800.0f;
        }

        // add newlines
        text_height *= 1 + std::count(m_text.begin(), m_text.end(), '\n');

        m_text_debug_text->setSize(CEGUI::USize(CEGUI::UDim(0, (text_width + 15) * global_upscalex), CEGUI::UDim(0, (text_height + 15) * global_upscaley)));
        m_text_debug_text->setXPosition(CEGUI::UDim(0, ((game_res_w * 0.5f) - text_width * 0.5f) * global_upscalex));
    }
}

void cDebugDisplay::Draw(cSurface_Request* request /* = NULL */)
{
    // Override cHudSprite::Draw() to do nothing.
    // CEGUI renders elsewhere in the main loop.
}

void cDebugDisplay::Set_Text(const std::string& ntext, float display_time /* = speedfactor_fps * 2.0f */)
{
    m_text = ntext;

    if (m_text.empty()) {
        m_text = m_text_old;
        m_text_old = "";

        m_counter = 0;
        return;
    }

    m_counter = display_time;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cPlayerPoints* pHud_Points = NULL;
cDebugDisplay* pHud_Debug = NULL;
cGoldDisplay* pHud_Goldpieces = NULL;
cLiveDisplay* pHud_Lives = NULL;
cTimeDisplay* pHud_Time = NULL;
cInfoMessage* pHud_Infomessage = NULL;
cItemBox* pHud_Itembox = NULL;
cFpsDisplay* pHud_Fps = NULL;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace TSC
