#pragma once

#include "Containers.h"
#include <string>
namespace gc
{
	using uint = unsigned int;
	
	void Log(std::string const& s);

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

	class gc_ptr_base
	{
	public:
		using OwnerType = Object;
	protected:
		static const OwnerType* cNoOwner;
	};

	// Forward declarations for friendship
	template <typename T>
	class gc_pool;

	template<typename T>
	class gc_ptr : public gc_ptr_base
	{
	public:
		friend class gc_pool<T>;
		
	private:
		// Constructors
		gc_ptr(const OwnerType* owner, T* to)
		: owner(owner)
		, impl(to ? new impl_class({ to, 1 , false }) : nullptr)
		{}
		
		gc_ptr(T* to) : gc_ptr(nullptr, to) {}
		
	public:
		gc_ptr() : gc_ptr(nullptr, nullptr) {}
		
		gc_ptr(gc_ptr const& other)
		: owner(other.owner)
		, impl(nullptr)
		{
			retain(other.impl);
		}
		
		gc_ptr(gc_ptr&& other)
		: owner(other.owner)
		, impl(std::move(other.impl))
		{
			other.impl = nullptr;
		}
		
		~gc_ptr()
		{
			gc_pool<T>::sInstance.remove(*this);
			release();
		}
		
		T* get() { return impl ? impl->to : nullptr; }
		const T* get() const { return impl ? impl->to : nullptr; }
		T* operator->() { return get(); }
		T& operator*() { return *impl->to; }
		
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
		
	//	// Downcast
	//	template<typename Base>
	//	gc_ptr<T>& operator=(gc_ptr<Base> const& other)
	//	{
	//		if (other.impl != impl)
	//		{
	//			release();
	//			retain(other.impl);
	//		}
	//		return *this;
	//	}
		
		int refCount() const
		{
			return impl ? impl->refCount : 0;
		}
		
		std::string to_string() const
		{
			return std::string("gc_ptr{ ") + (impl ? impl->to->to_string() : " null ") + " }";
		}
		
	private:
		const OwnerType* owner;
		
		class impl_class {
		public:
			~impl_class()
			{
				delete to;
			}
			
			T* to;
			uint refCount;
			bool marked;
		} *impl;
		
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
				delete impl;
			}
			
			impl = nullptr;
		}
	};

	template<typename T>
	class gc_pool
	{
	public:
		using Ptr = gc_ptr<T>;
		using OwnerType = gc_ptr_base::OwnerType;
		friend class gc_ptr<T>;
		static gc_pool<T> sInstance;
		
		template<typename... ARGS>
		Ptr makeUnowned(ARGS... args)
		{
			return makeOwned(Ptr::cNoOwner, std::forward<ARGS>(args)...);
		}
		
		Ptr makeUnownedFromInstance(T* instance)
		{
			Ptr p { Ptr::cNoOwner, instance };
			add(p);
			return p;
		}
		
		template<typename... ARGS>
		Ptr makeOwned(const OwnerType* owner, ARGS... args)
		{
			Ptr p(owner, new T(std::forward<ARGS>(args)...));
			add(p);
			return p;
		}
		
		Ptr makeOwnedNull(OwnerType* owner)
		{
			Ptr p(owner, nullptr);
			add(p);
			return p;
		}
		
		void reset()
		{
			pointers.clear();
		}
		
		void collectGarbage()
		{
			unmarkAll();
			mark(unownedPointers());
			deleteUnmarked();
		}
		
		std::string to_string() const
		{
			std::string result = std::string("gc_pool<") + typeid(T).name() + "> { size: " + std::to_string(pointers.size());
			for (auto it = pointers.begin(); it != pointers.end(); ++it)
			{
				result += "\n\t{ " + it->first->to_string() + ", " + it->second->to_string() + " }";
			}
			result += pointers.size() ? "\n}" : "}";

			return result;
		}
		
	private:
		using OwnerPointerMap = std::multimap<const OwnerType*, Ptr*>;
		using MapIt = typename OwnerPointerMap::iterator;
		using Range = std::pair<MapIt, MapIt>;
		
		OwnerPointerMap pointers;
		
		void add(Ptr& ptr)
		{
			pointers.insert({ ptr.owner, &ptr });
		}
		
		void remove(Ptr& ptr)
		{
			map_remove_if_value(pointers, [&](Ptr* candidate) {
				return candidate == &ptr;
			});
		}

		void mark(Range range)
		{
			for (MapIt it = range.first; it != range.second; ++it)
			{
				Ptr& ptr = *it->second;
				if (ptr.impl)
				{
					if (ptr.impl->marked)
					{
						// Already encountered this part of the graph, terminate recursion
						break;
					}
					else
						ptr.impl->marked = true;
				}
				
				Range children = pointers.equal_range(ptr.get());
				mark(children);
			}
		}
		
		void deleteUnmarked()
		{
			// TODO: Remove unmarked in single pass
			// Iterate until a complete pass finds no marked nodes
			for (bool keepGoing = true; keepGoing; )
			{
				keepGoing = false;
				
				for (auto it = pointers.begin(); it != pointers.end(); )
				{
					Ptr& ptr = *it->second;
					if (ptr.impl && !ptr.impl->marked)
					{
						OwnerType* pointee = ptr.get();
						nullifyPointersTo(*ptr);
						
						// Need to prevent this invalidating iterator
						delete pointee;
						
						// Iterator now invalid, need to break
						keepGoing = true;
						break;
					}
					else
						++it;
				}
			}
		}
		
		void nullifyPointersTo(OwnerType& pointee)
		{
			for (auto entry : pointers)
			{
				Ptr& p = *entry.second;
				if (p.impl && (p.impl->to == &pointee))
				{
					p.impl = nullptr;
				}
			}
		}
		
		void unmarkAll()
		{
			for (auto entry : pointers)
			{
				Ptr& p = *entry.second;
				if (p.impl)
				{
					p.impl->marked = false;
				}
			}
		}
		
		Range unownedPointers()
		{
			return pointers.equal_range(Ptr::cNoOwner);
		}
	};

	template <typename T, typename... ARGS>
	gc_ptr<T> make_gc(ARGS&&... args)
	{
		return gc_pool<T>::sInstance.makeUnowned(std::forward<ARGS>(args)...);
	}

	template <class Base, class InstanceClass>
	gc_ptr<Base> make_gc(Derived* instance)
	{
		Base* pBase = dynamic_cast<Base*>(instance);
		return gc_pool<Base>::sInstance.makeUnownedFromInstance(pBase);
	}
	
	template <typename T, typename... ARGS>
	gc_ptr<T> make_owned_null_gc(gc_ptr_base::OwnerType* owner)
	{
		return gc_pool<T>::sInstance.makeOwnedNull(owner);
	}

	template <typename T, typename... ARGS>
	gc_ptr<T> make_owned_gc(gc_ptr_base::OwnerType* owner, ARGS&&... args)
	{
		return gc_pool<T>::sInstance.makeOwned(owner, std::forward<ARGS>(args)...);
	}

	template <typename T>
	void collectGarbage()
	{
		gc_pool<T>::sInstance.collectGarbage();
	}
	
	template <class Base, class Derived>
	gc_ptr<Derived> dynamic_pointer_cast(gc_ptr<Base>& base)
	{
		Derived* pBase = dynamic_cast<Derived*>(base.get());
		return pBase ? make_gc<Derived>(pBase/*TODO , base.impl*/) : make_gc<Derived>(nullptr);
	}
}
