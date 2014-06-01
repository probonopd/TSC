/***************************************************************************
 * crate.hpp - boxes you can move around
 *
 * Copyright © 2014 The SMC Contributors
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SMC_CRATE_HPP
#define SMC_CRATE_HPP
#include "animated_sprite.hpp"
#include "../core/global_basic.hpp"
#include "../core/xml_attributes.hpp"
#include "../scripting/objects/specials/mrb_crate.hpp"

namespace SMC {

	enum CrateState {
		CRATE_DEAD = 0,
		CRATE_STAND = 1,
		CRATE_SLIDE = 2
	};

	class cCrate: public cAnimated_Sprite
	{
	public:
		// contructor
		cCrate(cSprite_Manager* p_sprite_manager);
		cCrate(XmlAttributes& attributes, cSprite_Manager* p_sprite_manager);
		virtual ~cCrate();

		// Create the MRuby object for this
		virtual mrb_value Create_MRuby_Object(mrb_state* p_state)
		{
			return mrb_obj_value(Data_Wrap_Struct(p_state, Scripting::p_rcCrate, &Scripting::rtSMC_Scriptable, this));
		}

		virtual void Update();
		virtual cCrate* Copy() const;
		/*virtual void Draw(cSurface_Request* p_request = NULL);*/

		virtual xmlpp::Element* Save_To_XML_Node(xmlpp::Element* p_element);

		virtual void Handle_Collision_Player(cObjectCollision* p_collision);
		virtual void Handle_Collision_Enemy(cObjectCollision* p_collision);
		virtual void Handle_out_of_Level(ObjectDirection dir);

	protected:
		virtual std::string Get_XML_Type_Name();

	private:
		void Init();

		CrateState m_crate_state;
	};

}

#endif
