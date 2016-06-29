#ifndef TSC_LEVEL_EDITOR_HPP
#define TSC_LEVEL_EDITOR_HPP
#ifdef ENABLE_NEW_EDITOR
#include "../core/editor/editor.hpp"

namespace TSC {
    class cEditor_Level: public cEditor
    {
    public:
        cEditor_Level();
        virtual ~cEditor_Level();

        virtual void Enable(cSprite_Manager* p_sprite_manager);
        virtual void Disable(void);
        void Set_Level(cLevel* p_level);

        virtual bool Function_New(void);
        virtual void Function_Save(bool with_dialog = 0);
        virtual void Function_Save_as(void);
        virtual void Function_Delete(void);

    private:
        cLevel* mp_level;
    };

    extern cEditor_Level* pLevel_Editor;
}

#endif
#endif
