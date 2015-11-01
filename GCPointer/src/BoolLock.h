#pragma once

namespace gc
{
	class bool_lock
	{
	public:
		bool& variable;
		
		bool_lock(bool& variable)
		: variable(variable)
		{
			variable = true;
		}
		
		~bool_lock()
		{
			variable = false;
		}
	};
}