#include "../global_basic.hpp"
#include "../game_core.hpp"
#include "../../gui/generic.hpp"
#include "../framerate.hpp"
#include "../../audio/audio.hpp"
#include "../../video/font.hpp"
#include "../../video/animation.hpp"
#include "../../input/keyboard.hpp"
#include "../../input/mouse.hpp"
#include "../../input/joystick.hpp"
#include "../../user/preferences.hpp"
#include "../../level/level.hpp"
#include "../../level/level_player.hpp"
#include "../../overworld/world_manager.hpp"
#include "../../video/renderer.hpp"
#include "../sprite_manager.hpp"
#include "../../overworld/overworld.hpp"
#include "../i18n.hpp"
#include "../filesystem/filesystem.hpp"
#include "../filesystem/resource_manager.hpp"
#include "../../video/img_settings.hpp"
#include "../errors.hpp"
#include "editor.hpp"

#ifdef ENABLE_NEW_EDITOR

using namespace TSC;

cEditor::cEditor()
{
    mp_editor_tabpane = NULL;
    m_enabled = false;
    m_rested = false;
    m_visibility_timer = 0.0f;
    m_mouse_inside = false;
    m_element_y = 0.0f;
    m_menu_filename = boost::filesystem::path(path_to_utf8("Needs to be set by subclasses"));
    m_editor_item_tag = "Must be set by subclass";
}

cEditor::~cEditor()
{
    Unload();
}

/**
 * Load the CEGUI layout from disk and attach it to the root window.
 * Does not show it, use Enable() for that.
 *
 * Override in subclasses to fill the editor pane with your custom
 * items. Be sure to call this parent method before doing so, though.
 */
void cEditor::Init(void)
{
    mp_editor_tabpane = CEGUI::WindowManager::getSingleton().loadLayoutFromFile("editor.layout");
    m_target_x_position = mp_editor_tabpane->getXPosition();
    mp_editor_tabpane->hide(); // Do not show for now
    CEGUI::System::getSingleton().getDefaultGUIContext().getRootWindow()->addChild(mp_editor_tabpane);

    parse_menu_file();
    populate_menu();

    mp_editor_tabpane->subscribeEvent(CEGUI::Window::EventMouseEntersArea, CEGUI::Event::Subscriber(&cEditor::on_mouse_enter, this));
    mp_editor_tabpane->subscribeEvent(CEGUI::Window::EventMouseLeavesArea, CEGUI::Event::Subscriber(&cEditor::on_mouse_leave, this));
}

/**
 * Empties the editor panel, detaches it from the CEGUI root window
 * and destroys it. After calling this you need to call Init()
 * again to use the editor.
 */
void cEditor::Unload(void)
{
    if (mp_editor_tabpane) {
        CEGUI::System::getSingleton().getDefaultGUIContext().getRootWindow()->removeChild(mp_editor_tabpane);
        CEGUI::WindowManager::getSingleton().destroyWindow(mp_editor_tabpane); // destroys child windows
        mp_editor_tabpane = NULL;
    }
}

void cEditor::Toggle(void)
{
    if (m_enabled)
        Disable();
    else
        Enable();
}

void cEditor::Enable(void)
{
    if (m_enabled)
        return;

    // TRANS: displayed to the user when opening the editor
    Draw_Static_Text(_("Loading"), &orange, NULL, 0);

    pAudio->Play_Sound("editor/enter.ogg");
    pHud_Debug->Set_Text(_("Editor enabled"));
    pMouseCursor->Set_Active(true);

    pActive_Animation_Manager->Delete_All(); // Stop all animations

    mp_editor_tabpane->show();
    m_enabled = true;
}

void cEditor::Disable(void)
{
    if (!m_enabled)
        return;

    pAudio->Play_Sound("editor/leave.ogg");
    pMouseCursor->Reset(0);
    pMouseCursor->Set_Active(false);

    mp_editor_tabpane->hide();
    m_enabled = false;
}

void cEditor::Update(void)
{
    if (!m_enabled)
        return;

    // If we have the mouse, do nothing.
    if (!m_mouse_inside) {
        // Otherwise, slowly fade the panel out until it is invisible.
        // When it reaches transparency, set to fully visible again
        // and place it on the side.
        if (!m_rested) {
            float timeout = speedfactor_fps * 2;
            if (m_visibility_timer >= timeout) {
                mp_editor_tabpane->setXPosition(CEGUI::UDim(-0.19f, 0.0f));
                mp_editor_tabpane->setAlpha(1.0f);

                m_rested = true;
                m_visibility_timer = 0.0f;
            }
            else {
                m_visibility_timer += pFramerate->m_speed_factor;
                float alpha_max = 1.0f;
                mp_editor_tabpane->setAlpha(alpha_max - ((alpha_max * m_visibility_timer) / timeout));
            }
        }
    }

    //std::cout << "Update..." << std::endl;
}

void cEditor::Draw(void)
{
}

bool cEditor::Handle_Event(const sf::Event& evt)
{
    return false;
}

void cEditor::Add_Editor_Item(boost::filesystem::path pixmap_path)
{
    static const int labelheight = 24;
    static const int imageheight = 48; /* Also image width (square) */
    static const int yskip = 24;

    // Several different formats of the same path
    std::string string_path(path_to_utf8(pixmap_path));
    std::string escaped_path(string_path);
    string_replace_all(escaped_path, "/", "+"); // CEGUI doesn't like / in ImageManager image names

    boost::filesystem::path settings_file = pResource_Manager->Get_Game_Pixmap(string_path);
    settings_file.replace_extension(utf8_to_path(".settings"));

    // Parse the image's settings file
    cImage_Settings_Parser parser;
    cImage_Settings_Data* p_settings = parser.Get(settings_file);

    CEGUI::Window* p_label = CEGUI::WindowManager::getSingleton().createWindow("TaharezLook/StaticText", std::string("label-of-") + escaped_path);
    p_label->setText(p_settings->m_name);
    p_label->setSize(CEGUI::USize(CEGUI::UDim(1, 0), CEGUI::UDim(0, labelheight)));
    p_label->setPosition(CEGUI::UVector2(CEGUI::UDim(0, 0), CEGUI::UDim(0, m_element_y)));
    p_label->setProperty("FrameEnabled", "False");

    CEGUI::ImageManager::getSingleton().addFromImageFile(escaped_path, string_path, "ingame-images");
    CEGUI::Window* p_image = CEGUI::WindowManager::getSingleton().createWindow("TaharezLook/StaticImage", std::string("image-of-") + escaped_path);
    p_image->setProperty("Image", escaped_path);
    p_image->setSize(CEGUI::USize(CEGUI::UDim(0, imageheight), CEGUI::UDim(0, imageheight)));
    p_image->setPosition(CEGUI::UVector2(CEGUI::UDim(0.5, -imageheight/2) /* center on X */, CEGUI::UDim(0, m_element_y + labelheight)));
    p_image->setProperty("FrameEnabled", "False");

    CEGUI::ScrollablePane* p_pane = static_cast<CEGUI::ScrollablePane*>(mp_editor_tabpane->getChild("editor_tab_items/editor_items"));
    p_pane->setContentPaneAutoSized(false);
    p_pane->setContentPaneArea(CEGUI::Rectf(0, 0, 1000, 4000));
    p_pane->setShowHorzScrollbar(false);
    p_pane->addChild(p_label);
    p_pane->addChild(p_image);

    m_element_y += labelheight + imageheight + yskip;
    delete p_settings;
}

void cEditor::parse_menu_file()
{
    std::string menu_file = path_to_utf8(m_menu_filename);

    // The menu XML file is so dead simple that a SAX parser would
    // simply be overkill. Leightweight XPath queries are enough.
    xmlpp::DomParser parser;
    parser.parse_file(menu_file);

    xmlpp::Element* p_root = parser.get_document()->get_root_node();
    xmlpp::NodeSet items = p_root->find("item");

    xmlpp::NodeSet::const_iterator iter;
    for (iter=items.begin(); iter != items.end(); iter++) {
        xmlpp::Element* p_node = dynamic_cast<xmlpp::Element*>(*iter);

        std::string name   = dynamic_cast<xmlpp::Element*>(p_node->find("property[@name='name']")[0])->get_attribute("value")->get_value();
        std::string tagstr = dynamic_cast<xmlpp::Element*>(p_node->find("property[@name='tags']")[0])->get_attribute("value")->get_value();

        std::cout << "Found XML <item> '" << name << std::endl;

        // Set color if available (---header--- elements have no color property)
        std::string colorstr = "FFFFFFFF";
        xmlpp::NodeSet results = p_node->find("property[@name='color']");
        if (!results.empty())
            colorstr = dynamic_cast<xmlpp::Element*>(results[0])->get_attribute("value")->get_value();

        // Burst the tag list into its elements
        std::vector<std::string> tags = string_split(tagstr, ";");

        // Prepare informational menu object
        Editor_Menu_Entry entry;
        entry.m_name = name;
        entry.m_color = Color(colorstr);
        entry.m_required_tags = tags;
        entry.mp_tab_window = CEGUI::WindowManager::getSingleton().createWindow("DefaultWindow", std::string("editor_items_") + name);

        // Mark as header element if the tag "header" is encountered.
        std::vector<std::string>::iterator iter;
        iter = std::find(tags.begin(), tags.end(), std::string("header"));
        entry.m_is_header = (iter != tags.end());

        // Store
        m_menu_entries.push_back(entry);
    }
}

void cEditor::populate_menu()
{
    CEGUI::Listbox* p_menu_listbox = static_cast<CEGUI::Listbox*>(mp_editor_tabpane->getChild("editor_tab_menu/editor_menu"));
    std::vector<Editor_Menu_Entry>::iterator iter;

    for(iter = m_menu_entries.begin(); iter != m_menu_entries.end(); iter++) {
        CEGUI::ListboxTextItem* p_item = new CEGUI::ListboxTextItem(iter->m_name);
        p_item->setTextColours(CEGUI::ColourRect(iter->m_color.Get_cegui_Color()));
        p_menu_listbox->addItem(p_item);
    }
}

Editor_Menu_Entry& cEditor::get_menu_entry(const std::string& name)
{
    std::vector<Editor_Menu_Entry>::iterator iter;
    for(iter=m_menu_entries.begin(); iter != m_menu_entries.end(); iter++) {
        if (iter->m_name == name) {
            return (*iter);
        }
    }

    throw(std::runtime_error(std::string("Element '") + name + "' not in editor menu list!"));
}

bool cEditor::on_mouse_enter(const CEGUI::EventArgs& event)
{
    m_mouse_inside = true;
    m_visibility_timer = 0.0f;
    m_rested = false;

    mp_editor_tabpane->setAlpha(1.0f);
    mp_editor_tabpane->setXPosition(m_target_x_position);
    return true;
}

bool cEditor::on_mouse_leave(const CEGUI::EventArgs& event)
{
    m_mouse_inside = false;
    return true;
}

#endif
