#ifndef SMC_SCRIPTING_CRATE_HPP
#define SMC_SCRIPTING_CRATE_HPP
namespace SMC {
	namespace Scripting {
		extern struct RClass* p_rcCrate;
		void Init_Crate(mrb_state* p_state);
	}
}
#endif
