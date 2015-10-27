#pragma once

#include "Containers.h"
#include "GCObject.h"
#include <string>

namespace gc
{
	using uint = unsigned int;
	
	void Log(std::string const& s);

	class gc_ptr_base
	{
	public:
		using OwnerType = Object;

	// TODO: Private
	public:
		static const OwnerType* cNoOwner;
		const OwnerType* owner;

		class impl_class {
		public:
			void* to;
			uint refCount;
			bool marked;
			// TODO: Needs destructor?
		} *impl;
		
		gc_ptr_base(const OwnerType* owner, impl_class* impl)
		: owner(owner)
		, impl(impl)
		{}
	};

	// Forward declarations for friendship
	template <typename T>
	class gc_pool;

	template<typename T>
	class gc_ptr : public gc_ptr_base
	{
	public:
		friend class gc_pool<T>;
		
		//TODO: Base = T
		template <class Base, class Derived>
		friend gc_ptr<Derived> dynamic_pointer_cast(gc_ptr<Base>& base);

	private:
		// Constructors
		gc_ptr(const OwnerType* owner, T* to)
		: gc_ptr_base(owner, to ? new impl_class({ to, 1 , false }) : nullptr)
		{}
		
	public:
		gc_ptr() : gc_ptr(nullptr, nullptr) {}

		gc_ptr(T* to) : gc_ptr(nullptr, to) {}

		// Copy constructor
		gc_ptr(gc_ptr const& other)
		: gc_ptr_base(other.owner, nullptr)
		{
			retain(other.impl);
		}
		
		// Move constructor
		gc_ptr(gc_ptr&& other)
		: gc_ptr_base(other.owner, std::move(other.impl))
		{
			other.impl = nullptr;
		}
		
		template<typename OtherT>
		gc_ptr(gc_ptr<OtherT> const& other)
		: gc_ptr_base(other.owner, nullptr)
		{
			retain(other.impl);
		}
		
		~gc_ptr()
		{
			gc_pool<T>::sInstance.remove(*this);
			release();
		}
		
		T* get() { return impl ? (T*)impl->to : nullptr; }
		const T* get() const { return impl ? (T*)impl->to : nullptr; }
		T* operator->() { return get(); }
		T& operator*() { return *get(); }
		
		gc_ptr<T>& operator=(gc_ptr<T> const& other)
		{
			if (other.impl != impl)
			{
				release();
				retain(other.impl);
			}
			return *this;
		}
		
		gc_ptr<T>& operator=(gc_ptr<T> && other)
		{
			if (other.impl != impl)
			{
				release();
				impl = other.impl;
				
				other.impl = nullptr;
			}
			return *this;
		}
		
		bool operator<(gc_ptr<T> const& other) const
		{
			if (!impl)
				return true;
			else if (!other.impl)
				return false;
			else
				return impl->to < other.impl->to;
		}
		
		int refCount() const
		{
			return impl ? impl->refCount : 0;
		}
		
		std::string to_string() const
		{
			return std::string("gc_ptr{ ") + (impl ? to_string(*impl->to) : " null ") + " }";
		}
		
	private:
		void retain(impl_class* newImpl)
		{
			if (newImpl)
				newImpl->refCount++;
			
			impl = newImpl;
		}
		
		void release()
		{
			if (impl && --impl->refCount == 0)
			{
				delete (T*) impl->to;
				impl->to = nullptr;
				delete impl;
			}
			
			impl = nullptr;
		}
	};
}
