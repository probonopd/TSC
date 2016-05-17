#ifndef TSC_WORLD_EDITOR_HPP
#define TSC_WORLD_EDITOR_HPP
#ifdef ENABLE_NEW_EDITOR
#include "../core/editor/editor.hpp"

namespace TSC {
    class cEditor_World: public cEditor
    {
    public:
        cEditor_World();
        virtual ~cEditor_World();

        void Set_World(cOverworld* p_world);
    private:
        cOverworld* mp_overworld;
    };

    extern cEditor_World* pWorld_Editor;
}
#endif
#endif
