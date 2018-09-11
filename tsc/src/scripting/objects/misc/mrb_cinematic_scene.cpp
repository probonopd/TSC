/***************************************************************************
 * mrb_cinematic_scene.cpp
 *
 * Copyright © 2012-2018 The TSC Contributors
 ***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "mrb_cinematic_scene.hpp"

using namespace TSC;
using namespace TSC::Scripting;

static mrb_value Initialize(mrb_state* p_state, mrb_value self)
{
    // DATA_PTR(self) = Something c++ish
    DATA_TYPE(self) = &rtTSC_Scriptable;

    return self;
}

void TSC::Scripting::Init_Cinematic_Scene(mrb_state* p_state)
{
    struct RClass* p_rcCinematicScene = mrb_define_class(p_state, "CinematicScene", p_state->object_class);
    MRB_SET_INSTANCE_TT(p_rcCinematicScene, MRB_TT_DATA);

    mrb_define_method(p_state, p_rcCinematicScene, "initialize", Initialize, MRB_ARGS_NONE());
}
