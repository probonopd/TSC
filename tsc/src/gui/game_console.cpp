#include "../core/global_basic.hpp"
#include "config.hpp"
#include "../core/i18n.hpp"
#include "game_console.hpp"

// extern
TSC::cGame_Console* TSC::gp_game_console = NULL;

using namespace TSC;

cGame_Console::cGame_Console()
    : mp_console_root(NULL)
{
    // Load layout file and add it to the root
    mp_console_root = CEGUI::WindowManager::getSingleton().loadLayoutFromFile("game_console.layout");
    CEGUI::System::getSingleton().getDefaultGUIContext().getRootWindow()->addChild(mp_console_root);

    mp_console_root->hide(); // Do not show for now

    mp_input_edit  = static_cast<CEGUI::Editbox*>(mp_console_root->getChild("input"));
    mp_output_edit = static_cast<CEGUI::MultiLineEditbox*>(mp_console_root->getChild("output"));

    mp_input_edit->subscribeEvent(CEGUI::Editbox::EventTextAccepted,
                                  CEGUI::Event::Subscriber(&cGame_Console::on_input_accepted, this));

    Reset();
}

cGame_Console::~cGame_Console()
{
    if (mp_console_root) {
        CEGUI::System::getSingleton().getDefaultGUIContext().getRootWindow()->removeChild(mp_console_root);
        CEGUI::WindowManager::getSingleton().destroyWindow(mp_console_root);
        mp_console_root = NULL;
    }
}

void cGame_Console::Show()
{
    mp_console_root->show();
    mp_input_edit->activate();
}

void cGame_Console::Hide()
{
    mp_console_root->hide();
}

void cGame_Console::Toggle()
{
    if (mp_console_root->isVisible())
        Hide();
    else
        Show();
}

void cGame_Console::Update()
{

}

/**
 * Clear output and show preamble.
 */
void cGame_Console::Reset()
{
    mp_output_edit->setText("");
    print_preamble();
}

/**
 * Convenience function that expects you pass valid UTF-8-encoded
 * text and conerts it to a CEGUI::String before forwarding it
 * to the other overload that takes a CEGUI::String.
 */
void cGame_Console::Append_Text(std::string text)
{
    CEGUI::String ceguitext(reinterpret_cast<const CEGUI::utf8*>(text.c_str()));
    Append_Text(ceguitext);
}

/**
 * Append the given text to the output widget.
 */
void cGame_Console::Append_Text(CEGUI::String text)
{
    CEGUI::String curtext = mp_output_edit->getText();
    curtext += text;
    mp_output_edit->setText(curtext);
}

void cGame_Console::print_preamble()
{
    char text[10000];
    // TRANS: The text is copied from the end of the GPLv3. If there is an
    // TRANS: accepted translation of the GPLv3 available in your language,
    // TRANS: then you should use its wording rather than trying to translate
    // TRANS: it yourself.
    sprintf(text, _("TSC Scripting Console\nCopyright © %d The TSC Contributors\n\n"
    "This program comes with ABSOLUTELY NO WARRANTY; for details\n"
    "see the file tsc/docs/license.txt. This is free software, and\n"
    "you are welcome to redistribute it under certain conditions;\n"
    "see the aforementioned file for details."), TSC_COMPILE_YEAR);

    Append_Text(std::string(text));
    Append_Text(std::string("\n"));

#ifdef TSC_VERSION_POSTFIX
    // TRANS: %s is the version postfix, e.g. "dev" or "beta3".
    sprintf(text, _("You are running TSC version %d.%d.%d-%s."),
            TSC_VERSION_MAJOR,
            TSC_VERSION_MINOR,
            TSC_VERSION_PATCH,
            TSC_VERSION_POSTFIX);
#else
    sprintf(text, _("You are running TSC version %d.%d.%d."),
            TSC_VERSION_MAJOR,
            TSC_VERSION_MINOR,
            TSC_VERSION_PATCH);
#endif

    Append_Text(std::string(text));
}

bool cGame_Console::on_input_accepted(const CEGUI::EventArgs& evt)
{
    std::cout << "!!" << std::endl;
    return true;
}
