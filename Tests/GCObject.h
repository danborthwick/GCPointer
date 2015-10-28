#pragma once

#include <string>

namespace gc
{
	class Object
	{
	public:
		Object()
		{
			sInstanceCount++;
		}
		
		virtual ~Object()
		{
			sInstanceCount--;
			destructor();
		}
		
		virtual std::string to_string() const
		{
			return "Object{}";
		}
		
		// Allow expectations on destructor
		virtual void destructor() {}
		
		static int instanceCount() { return sInstanceCount; }
		static void resetInstanceCount() { sInstanceCount = 0; }
		
	private:
		static int sInstanceCount;
	};
};