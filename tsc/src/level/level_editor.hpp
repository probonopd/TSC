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

        void Set_Level(cLevel* p_level);

    private:
        cLevel* mp_level;
    };

    extern cEditor_Level* pLevel_Editor;
}

#endif
#endif
