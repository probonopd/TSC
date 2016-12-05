#include "../core/global_basic.hpp"
#include <mruby/error.h>
#include "config.hpp"
#include "../core/i18n.hpp"
#include "../level/level.hpp"
#include "game_console.hpp"

// extern
TSC::cGame_Console* TSC::gp_game_console = NULL;

using namespace TSC;

cGame_Console::cGame_Console()
    : mp_console_root(NULL),
      mp_input_edit(NULL),
      mp_output_edit(NULL),
      mp_lino_text(NULL),
      m_lino(0)
{
    // Load layout file and add it to the root
    mp_console_root = CEGUI::WindowManager::getSingleton().loadLayoutFromFile("game_console.layout");
    CEGUI::System::getSingleton().getDefaultGUIContext().getRootWindow()->addChild(mp_console_root);

    mp_console_root->hide(); // Do not show for now

    mp_input_edit  = static_cast<CEGUI::Editbox*>(mp_console_root->getChild("input"));
    mp_output_edit = static_cast<CEGUI::MultiLineEditbox*>(mp_console_root->getChild("output"));
    mp_lino_text   = mp_console_root->getChild("lineno");

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

bool cGame_Console::IsVisible() const
{
    return mp_console_root->isVisible();
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
    mp_output_edit->setCaretIndex(curtext.length());
    mp_output_edit->ensureCaretIsVisible();
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
    std::string code(mp_input_edit->getText().c_str());
    mp_input_edit->setText("");

    if (!pActive_Level) { // This should never happen
        Append_Text(std::string("ERROR: No active level!"));
        return true;
    }

    // TODO: Given that the console should be the regular output in the future,
    // the following code should be merged into cMRuby_Interpreter::Run_Code().
    mrb_state* p_mrb_state = pActive_Level->m_mruby->Get_MRuby_State();
    mrbc_context* p_context = mrbc_context_new(p_mrb_state);
    p_context->lineno = ++m_lino;
    mrbc_filename(p_mrb_state, p_context, "(console)");
    mrb_value result = pActive_Level->m_mruby->Run_Code_In_Context(code, p_context);

    if (p_mrb_state->exc) {
        mrb_value exception   = mrb_obj_value(p_mrb_state->exc);
        mrb_value bt          = mrb_exc_backtrace(p_mrb_state, exception);
        mrb_value rdesc       = mrb_funcall(p_mrb_state, exception, "message", 0);
        const char* classname = mrb_obj_classname(p_mrb_state, exception);

        std::string message(classname);
        message += ": ";
        message += std::string(RSTRING_PTR(rdesc), RSTRING_LEN(rdesc));
        Append_Text(message);

        //mrb_value bt = mrb_funcall(p_mrb_state, exception, "backtrace", 0);
        for(int i=0; i < RARRAY_LEN(bt); i++) {
            std::string btline("    from ");
            mrb_value rstep = mrb_ary_ref(p_mrb_state, bt, i);

            btline += std::string(RSTRING_PTR(rstep), RSTRING_LEN(rstep));
            btline += "\n";
            Append_Text(btline);
        }

        // Clear exception pointer
        p_mrb_state->exc = NULL;
    }
    else {
        mrb_value rstr = mrb_inspect(p_mrb_state, result);
        if (mrb_string_p(rstr)) {
            std::string str(">> ");
            str += std::string(RSTRING_PTR(rstr), RSTRING_LEN(rstr));
            Append_Text(str);
        }
        else {
            Append_Text(std::string("(#inspect did not return a string)"));
        }
    }

    mrbc_context_free(p_mrb_state, p_context);
    mp_lino_text->setText(std::to_string(m_lino+1));

    return true;
}
