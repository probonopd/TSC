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
#include "../filesystem/relative.hpp"
#include "../filesystem/resource_manager.hpp"
#include "../../video/img_settings.hpp"
#include "../errors.hpp"
#include "editor.hpp"

#ifdef ENABLE_NEW_EDITOR

using namespace TSC;

cEditor::cEditor()
{
    mp_editor_tabpane = NULL;
    mp_menu_listbox = NULL;
    m_enabled = false;
    m_rested = false;
    m_visibility_timer = 0.0f;
    m_mouse_inside = false;
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
    mp_editor_tabpane = static_cast<CEGUI::TabControl*>(CEGUI::WindowManager::getSingleton().loadLayoutFromFile("editor.layout"));
    m_target_x_position = mp_editor_tabpane->getXPosition();
    mp_editor_tabpane->hide(); // Do not show for now

    CEGUI::System::getSingleton().getDefaultGUIContext().getRootWindow()->addChild(mp_editor_tabpane);

    mp_menu_listbox = static_cast<CEGUI::Listbox*>(mp_editor_tabpane->getChild("editor_tab_menu/editor_menu"));

    parse_menu_file();
    populate_menu();
    load_image_items();

    mp_editor_tabpane->subscribeEvent(CEGUI::Window::EventMouseEntersArea, CEGUI::Event::Subscriber(&cEditor::on_mouse_enter, this));
    mp_editor_tabpane->subscribeEvent(CEGUI::Window::EventMouseLeavesArea, CEGUI::Event::Subscriber(&cEditor::on_mouse_leave, this));

    mp_menu_listbox->subscribeEvent(CEGUI::Listbox::EventSelectionChanged, CEGUI::Event::Subscriber(&cEditor::on_menu_selection_changed, this));
}

/**
 * Empties the editor panel, detaches it from the CEGUI root window
 * and destroys it. After calling this you need to call Init()
 * again to use the editor.
 */
void cEditor::Unload(void)
{
    std::vector<cEditor_Menu_Entry*>::iterator iter;
    for(iter=m_menu_entries.begin(); iter != m_menu_entries.end(); iter++)
        delete *iter;

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

/**
 * Adds a graphic to the editor menu. The settings file of this
 * graphic will be parsed and it will be placed in the menu
 * accordingly (subclasses have to set the `m_editor_item_tag`
 * member variable the the master tag required for graphics to
 * show up in this editor; these are "level" and "world" for the
 * level and world editor subclasses, respectively. That is,
 * a graphic tagged with "world" will never appear in the level
 * editor, and vice-versa.).
 *
 * \param settings_path
 * Absolute path that refers to the settings file
 * of the graphic to add.
 *
 * \returns false if the item was not added because the master tag
 * was missing, true otherwise.
 */
bool cEditor::Try_Add_Editor_Item(boost::filesystem::path settings_path)
{
    // Parse the image's settings file
    cImage_Settings_Parser parser;
    cImage_Settings_Data* p_settings = parser.Get(settings_path);

    // If the master tag is not in the tag list, do not add this graphic to the
    // editor.
    if (p_settings->m_editor_tags.find(m_editor_item_tag) == std::string::npos)
        return false;

    // Find the menu entries that contain the tags this graphic has set.
    std::vector<cEditor_Menu_Entry*> target_menu_entries = find_target_menu_entries_for(*p_settings);
    std::vector<cEditor_Menu_Entry*>::iterator iter;

    // Add the graphics to the respective menu entries' GUI panels.
    for(iter=target_menu_entries.begin(); iter != target_menu_entries.end(); iter++) {
        (*iter)->Add_Image_Item(settings_path, *p_settings);
    }

    delete p_settings;
    return true;
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

        // Set color if available (---header--- elements have no color property)
        std::string colorstr = "FFFFFFFF";
        xmlpp::NodeSet results = p_node->find("property[@name='color']");
        if (!results.empty())
            colorstr = dynamic_cast<xmlpp::Element*>(results[0])->get_attribute("value")->get_value();

        // Burst the tag list into its elements
        std::vector<std::string> tags = string_split(tagstr, ";");

        // Prepare informational menu object
        cEditor_Menu_Entry* p_entry = new cEditor_Menu_Entry(name);
        p_entry->Set_Color(Color(colorstr));
        p_entry->Set_Required_Tags(tags);

        // Mark as header element if the tag "header" is encountered.
        std::vector<std::string>::iterator iter;
        iter = std::find(tags.begin(), tags.end(), std::string("header"));
        p_entry->Set_Header((iter != tags.end()));

        // Store
        m_menu_entries.push_back(p_entry);
    }
}

void cEditor::populate_menu()
{
    std::vector<cEditor_Menu_Entry*>::iterator iter;

    for(iter = m_menu_entries.begin(); iter != m_menu_entries.end(); iter++) {
        CEGUI::ListboxTextItem* p_item = new CEGUI::ListboxTextItem((*iter)->Get_Name());
        p_item->setTextColours(CEGUI::ColourRect((*iter)->Get_Color().Get_cegui_Color()));
        p_item->setUserData(*iter);
        mp_menu_listbox->addItem(p_item);
    }
}

void cEditor::load_image_items()
{
    std::vector<boost::filesystem::path> image_files = Get_Directory_Files(pResource_Manager->Get_Game_Pixmaps_Directory(), ".settings");
    std::vector<boost::filesystem::path>::iterator iter;

    for(iter=image_files.begin(); iter != image_files.end(); iter++) {
        Try_Add_Editor_Item(*iter);
    }
}

cEditor_Menu_Entry* cEditor::get_menu_entry(const std::string& name)
{
    std::vector<cEditor_Menu_Entry*>::iterator iter;
    for(iter=m_menu_entries.begin(); iter != m_menu_entries.end(); iter++) {
        if ((*iter)->Get_Name() == name) {
            return (*iter);
        }
    }

    throw(std::runtime_error(std::string("Element '") + name + "' not in editor menu list!"));
}

/**
 * Iterates the list of menu entries and returns a list of those whose
 * requirements match the graphic given. That is, a menu entry declares
 * a set of required tags, and if the passed graphic has all of these
 * tags set, the menu entry is included in the return list. If more
 * than the required tags are set, the entry is returned nevertheless.
 */
std::vector<cEditor_Menu_Entry*> cEditor::find_target_menu_entries_for(const cImage_Settings_Data& settings)
{
    std::vector<cEditor_Menu_Entry*> results;
    std::vector<std::string> available_tags = string_split(settings.m_editor_tags, ";");

    /* To show up in the editor, a graphic needs to have all tags set
     * on it that are required by a menu entry (except for the master
     * editor tag, which is checked elsewhere). */
    std::vector<cEditor_Menu_Entry*>::iterator iter;
    for(iter=m_menu_entries.begin(); iter != m_menu_entries.end(); iter++) {
        cEditor_Menu_Entry* p_menu_entry = *iter;
        std::vector<std::string>::iterator tag_iterator;

        // Check if all of this menu's tags are on the graphic.
        bool all_tags_available = true;
        for(tag_iterator=p_menu_entry->Get_Required_Tags().begin(); tag_iterator != p_menu_entry->Get_Required_Tags().end(); tag_iterator++) {
            if (std::find(available_tags.begin(), available_tags.end(), *tag_iterator) == available_tags.end()) {
                // Tag is missing.
                all_tags_available = false;
                break;
            }
        }

        if (all_tags_available) {
            results.push_back(p_menu_entry);
        }
    }

    return results;
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

bool cEditor::on_menu_selection_changed(const CEGUI::EventArgs& event)
{
    CEGUI::ListboxItem* p_current_item = mp_menu_listbox->getFirstSelectedItem();

    // Ignore if no selection
    if (!p_current_item)
        return false;

    cEditor_Menu_Entry* p_menu_entry = static_cast<cEditor_Menu_Entry*>(p_current_item->getUserData());

    // Ignore clicks on headings
    if (p_menu_entry->Is_Header())
        return false;

    p_menu_entry->Activate(mp_editor_tabpane);

    return true;
}

cEditor_Menu_Entry::cEditor_Menu_Entry(std::string name)
{
    m_name = name;
    m_element_y = 0;

    // Prepare the CEGUI items window. This will be shown whenever
    // this menu entry is clicked.
    mp_tab_pane = static_cast<CEGUI::ScrollablePane*>(CEGUI::WindowManager::getSingleton().createWindow("TaharezLook/ScrollablePane", std::string("editor_items_") + name));
    mp_tab_pane->setPosition(CEGUI::UVector2(CEGUI::UDim(0, 0), CEGUI::UDim(0.01, 0)));
    mp_tab_pane->setSize(CEGUI::USize(CEGUI::UDim(0.99, 0), CEGUI::UDim(0.95, 0)));
    mp_tab_pane->setContentPaneAutoSized(false);
    mp_tab_pane->setContentPaneArea(CEGUI::Rectf(0, 0, 1000, 4000));
    mp_tab_pane->setShowHorzScrollbar(false);
}

cEditor_Menu_Entry::~cEditor_Menu_Entry()
{
    // TODO: Detach if still attached and destroy
    // Nonattached windows will not get destroyed, obviously
    // CEGUI::Window* p_editor_tabpane = CEGUI::System::getSingleton().getDefaultGUIContext().getRootWindow()->getChild("tabcontrol_editor/editor_tab_items")
}

void cEditor_Menu_Entry::Add_Image_Item(boost::filesystem::path settings_path, const cImage_Settings_Data& settings)
{
    static const int labelheight = 24;
    static const int imageheight = 48; /* Also image width (square) */
    static const int yskip = 24;

    // Find the PNG of this settings file. If an equally named .png exists,
    // assume that file, otherwise check the settings 'base' property. If
    // that also doesn't exist, that's an error.
    boost::filesystem::path pixmap_path(settings_path); // Copy
    pixmap_path.replace_extension(utf8_to_path(".png"));
    if (!boost::filesystem::exists(pixmap_path)) {
        if (settings.m_base.empty()) { // Error
            std::cerr << "PNG file for settings file '" << path_to_utf8(settings_path) << "' not found (no .png found and no 'base' setting)." << std::endl;
            return;
        }
        else {
            pixmap_path = pixmap_path.parent_path() / settings.m_base;
            if (!boost::filesystem::exists(pixmap_path)) {
                std::cerr << "PNG base file not found at '" << path_to_utf8(pixmap_path) << "'." << std::endl;
                return;
            }
        }
    }

    std::string escaped_pixmap_path(path_to_utf8(pixmap_path));
    std::string escaped_settings_path(path_to_utf8(settings_path));
    string_replace_all(escaped_pixmap_path, "/", "+"); // CEGUI doesn't like / in ImageManager image names
    string_replace_all(escaped_settings_path, "/", "+");

    CEGUI::Window* p_label = CEGUI::WindowManager::getSingleton().createWindow("TaharezLook/StaticText", std::string("label-of-") + escaped_settings_path);
    p_label->setText(settings.m_name);
    p_label->setSize(CEGUI::USize(CEGUI::UDim(1, 0), CEGUI::UDim(0, labelheight)));
    p_label->setPosition(CEGUI::UVector2(CEGUI::UDim(0, 0), CEGUI::UDim(0, m_element_y)));
    p_label->setProperty("FrameEnabled", "False");

    // TODO: Apply rotation

    /* CEGUI only knows about image sets, not about single images.
     * Thus we effectively add one-image imagesets here to have CEGUI
     * display our image. Note CEGUI also caches these images and will
     * throw a CEGUI::AlreadyExsitsException in addFromImageFile() if
     * an image is added multiple times (like multiple loads of the
     * editor or just the same item being added to multiple menus).
     * Thus we have to check if the item graphic has been cached before,
     * and only if that isn't the case, load the file from disk in the
     * way described. */
    if (!CEGUI::ImageManager::getSingleton().isDefined(escaped_pixmap_path)) {
        try {
            CEGUI::ImageManager::getSingleton().addFromImageFile(escaped_pixmap_path, path_to_utf8(fs_relative(pResource_Manager->Get_Game_Pixmaps_Directory(), pixmap_path)), "ingame-images");
        }
        catch(CEGUI::RendererException& e) {
            std::cerr << "Warning: Failed to load as editor item image: " << pixmap_path << std::endl;
            return;
        }
    }

    CEGUI::Window* p_image = CEGUI::WindowManager::getSingleton().createWindow("TaharezLook/StaticImage", std::string("image-of-") + escaped_settings_path);
    p_image->setProperty("Image", escaped_pixmap_path);
    p_image->setSize(CEGUI::USize(CEGUI::UDim(0, imageheight), CEGUI::UDim(0, imageheight)));
    p_image->setPosition(CEGUI::UVector2(CEGUI::UDim(0.5, -imageheight/2) /* center on X */, CEGUI::UDim(0, m_element_y + labelheight)));
    p_image->setProperty("FrameEnabled", "False");

    mp_tab_pane->addChild(p_label);
    mp_tab_pane->addChild(p_image);

    // Remember where we stopped for the next call.
    m_element_y += labelheight + imageheight + yskip;
}

/// Activate this entry's panel in the handed tabbook.
void cEditor_Menu_Entry::Activate(CEGUI::TabControl* p_tabcontrol)
{
    CEGUI::Window* p_container = p_tabcontrol->getTabContents("editor_tab_items");

    // Detach the current scrollable pane.
    CEGUI::ScrollablePane* p_current_pane = static_cast<CEGUI::ScrollablePane*>(p_container->getChildElementAtIdx(0));
    p_container->removeChild(p_current_pane);

    // Attach our content instead.
    p_container->addChild(mp_tab_pane);

    // Switch to the items page
    p_tabcontrol->setSelectedTab("editor_tab_items");
}

#endif
