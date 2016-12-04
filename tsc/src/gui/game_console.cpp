#include "../core/global_basic.hpp"
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
    mp_console_root->getChild("input")->activate();
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

void cGame_Console::Clear()
{
}

void cGame_Console::Append_Line(std::string line)
{
}
