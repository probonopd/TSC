/***************************************************************************
 * font.cpp  -  internal font functions
 *
 * Copyright © 2006 - 2011 Florian Richter
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

#include "../core/global_basic.hpp"
#include "../video/font.hpp"
#include "../video/gl_surface.hpp"
#include "../core/property_helper.hpp"
#include "../core/filesystem/resource_manager.hpp"
#include "../core/game_core.hpp"
#include "renderer.hpp"

using namespace std;
using namespace TSC;

/* *** *** *** *** *** *** *** Font Manager class *** *** *** *** *** *** *** *** *** *** */

cFont_Manager::cFont_Manager(void)
{
    //
}

cFont_Manager::~cFont_Manager(void)
{
    //
}

void cFont_Manager::Init(void)
{
    if (!m_font_normal.loadFromFile(path_to_utf8(pResource_Manager->Get_Gui_Font_Directory() / utf8_to_path("DejaVuSans-Bold.ttf")))) {
        // FIXME: Throw a proper exception
        throw "Font loading failed";
    }
}

/**
 * This function wraps the given SFML text in a render request and
 * submits that request to the render queue.
 *
 * The `text` parameter must be prepared with Prepare_SFML_Text()
 * before passing it to this function.
 */
void cFont_Manager::Queue_Text(const sf::Text& text)
{
    cText_Request* p_req = new cText_Request(text);
    pRenderer->Add(p_req);
}

/**
 * From the given parameters, update an sf::Text instance
 * so that it can be passed to Queue_Text(). Use this function
 * instead of modifying the sf::Text instance directly; it
 * converts TSC specifics to SFML's understanding:
 *
 * 1. Conversion of TSC camera to SFML view (unless `ignore_camera` is given)
 * 2. Conversion of TSC font size name to SFML font size value
 * 3. Conversion of TSC color object to SFML color object
 *
 * It is not necessary to call this function every frame if you
 * don’t move the text around.
 */
void cFont_Manager::Prepare_SFML_Text(sf::Text& text, const std::string& str, float x, float y, int fontsize /* = FONTSIZE_NORMAL */, const Color color /* = black */, bool ignore_camera /* = false */)
{
    wstring utf32Str;
    sf::Utf8::toUtf32(str.begin(), str.end(), std::back_inserter(utf32Str));

    text.setFont(m_font_normal);
    text.setColor(color.Get_SFML_Color());
    text.setCharacterSize(fontsize);
    text.setString(utf32Str);

    if (ignore_camera) {
        // SFML thinks 0|0 is left top of the window
        // (we do not use SFML views yet).
        text.setPosition(x, y);
    }
    else {
        // Translate level coordinate to window coordinate.
        // TODO: Abolish our camera system and use SFML views instead.
        text.setPosition(x - pActive_Camera->m_x, y - pActive_Camera->m_y);
    }
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cFont_Manager* TSC::pFont = NULL;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
