/***************************************************************************
 * secret_area.cpp - Where you find bonuses
 *
 * Copyright © 2017 The TSC Contributors
 ***************************************************************************/
/*
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../core/global_game.hpp"
#include "secret_area.hpp"
#include "../level/level_player.hpp"
#include "../core/game_core.hpp"
#include "../core/i18n.hpp"
#include "../core/xml_attributes.hpp"
#include "../core/sprite_manager.hpp"
#include "../level/level.hpp"
#include "../level/level_settings.hpp"
#include "../core/editor/editor.hpp"
#include "../level/level_editor.hpp"
#include "../video/renderer.hpp"

using namespace TSC;

cSecret_Area::cSecret_Area(cSprite_Manager* sprite_manager)
    : cMovingSprite(sprite_manager, "secretarea")
{
    cSecret_Area::Init();
}

cSecret_Area::cSecret_Area(XmlAttributes& attributes, cSprite_Manager* sprite_manager)
    : cMovingSprite(sprite_manager, "secretarea")
{
    cSecret_Area::Init();

    // position
    Set_Pos(string_to_float(attributes["posx"]), string_to_float(attributes["posy"]), true);

    // Sizing
    m_rect.m_w = string_to_float(attributes["width"]);
    m_rect.m_h = string_to_float(attributes["height"]);
    m_col_rect.m_w = m_rect.m_w;
    m_col_rect.m_h = m_rect.m_h;
    m_start_rect.m_w = m_rect.m_w;
    m_start_rect.m_h = m_rect.m_h;

    // activated
    m_activated = string_to_bool(attributes["activated"]);
}

cSecret_Area::~cSecret_Area(void)
{
}

void cSecret_Area::Init()
{
    m_sprite_array = ARRAY_ACTIVE;
    m_type = TYPE_SECRET_AREA;
    m_name = _("Secret Area");
    m_massive_type = MASS_PASSIVE;
    m_editor_pos_z = 0.112f;
    m_camera_range = 1000;

    // size
    m_rect.m_w = 100;
    m_rect.m_h = 100;
    m_col_rect.m_w = m_rect.m_w;
    m_col_rect.m_h = m_rect.m_h;
    m_start_rect.m_w = m_rect.m_w;
    m_start_rect.m_h = m_rect.m_h;

    m_activated = false;
}

cSecret_Area* cSecret_Area::Copy(void) const
{
    cSecret_Area* secarea = new cSecret_Area(m_sprite_manager);
    secarea->Set_Pos(m_start_pos_x, m_start_pos_y, 1);
    secarea->m_rect.m_w = m_rect.m_w;
    secarea->m_rect.m_h = m_rect.m_h;
    secarea->m_col_rect.m_w = m_col_rect.m_w;
    secarea->m_col_rect.m_h = m_col_rect.m_h;
    secarea->m_start_rect.m_w = m_start_rect.m_w;
    secarea->m_start_rect.m_h = m_start_rect.m_h;
    secarea->m_activated = m_activated;
    return secarea;
}

void cSecret_Area::Draw(cSurface_Request* request /* = NULL */)
{
    if (!m_valid_draw)
        return;

    // draw rect
    cRect_Request* req = new cRect_Request();
    pVideo->Draw_Rect(m_col_rect.m_x - pActive_Camera->m_x, m_col_rect.m_y - pActive_Camera->m_y, m_col_rect.m_w, m_col_rect.m_h, m_editor_pos_z, &orange, req);
    req->m_filled = 0;
    req->m_stipple_pattern = 0xAAAA;
    req->m_line_width = 6.0f;
    pRenderer->Add(req);
}

bool cSecret_Area::Is_Draw_Valid(void)
{
    // if editor not enabled
    if (!editor_enabled) {
        return 0;
    }

    // if not visible on the screen
    if (!m_active || !Is_Visible_On_Screen()) {
        return 0;
    }

    return 1;
}


void cSecret_Area::Activate(void)
{
    if (m_activated)
        return;

    std::cout << "SECRET AREA FOUND!" << std::endl;

    m_activated = true;
}

#ifdef ENABLE_EDITOR
void cSecret_Area::Editor_Activate(void)
{
}

void cSecret_Area::Editor_Deactivate(void)
{
    cMovingSprite::Editor_Deactivate();
}

void cSecret_Area::Editor_State_Update(void)
{
}
#endif

xmlpp::Element* cSecret_Area::Save_To_XML_Node(xmlpp::Element* p_element)
{
    xmlpp::Element* p_node = cMovingSprite::Save_To_XML_Node(p_element);
    Add_Property(p_node, "activated", m_activated);
    Add_Property(p_node, "width", m_rect.m_w);
    Add_Property(p_node, "height", m_rect.m_h);

    return p_node;
}

std::string cSecret_Area::Get_XML_Type_Name()
{
    return "secretarea";
}

void cSecret_Area::Set_Massive_Type(MassiveType type)
{
    // Ignore to prevent "m" toggling in level editor
}
