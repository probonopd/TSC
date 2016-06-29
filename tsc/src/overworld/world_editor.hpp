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

        virtual void Enable(cSprite_Manager* p_sprite_manager);
        virtual void Disable(void);
        void Set_World(cOverworld* p_world);

        // Menu functions
        virtual bool Function_New(void);
        virtual void Function_Load(void);
        virtual void Function_Save(bool with_dialog = 0);
        //virtual void Function_Save_as( void );
        virtual void Function_Reload(void);
        //void Function_Settings( void );
    private:
        cOverworld* mp_overworld;
    };

    extern cEditor_World* pWorld_Editor;
}
#endif
#endif
