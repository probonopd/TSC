#ifndef TSC_WORLD_EDITOR_HPP
#define TSC_WORLD_EDITOR_HPP
#ifdef ENABLE_EDITOR
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

        virtual vector<cSprite*> Parse_Items_File();

        // This one should be private...
        cOverworld* mp_overworld;
    private:
        static std::vector<cSprite*> items_loader_callback(const std::string& name, XmlAttributes& attributes, int engine_version, cSprite_Manager* p_sprite_manager, void* p_data);
    };

    extern cEditor_World* pWorld_Editor;
}
#endif
#endif
