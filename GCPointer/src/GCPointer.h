#pragma once

#include "Containers.h"
#include <string>

namespace gc
{
	using uint = unsigned int;

	// Forward declarations
	void Log(std::string const& s);
	template<typename T> class gc_ptr;
	template <typename T> class gc_pool;

	// Base for all gc_ptr template instantiations.
	// All public API is declared in gc_ptr
	class gc_ptr_base
	{
	public:
		using OwnerType = void;
		static const OwnerType* cNoOwner;
	protected:
		using Deleter = std::function<void(void*)>;

	private:
		const OwnerType* owner;

		class Backing {
		public:
			void* to;
			Deleter& deleter;
			uint refCount;
			bool marked;
			
			void deletePointee()
			{
				deleter(to);
				to = nullptr;
			}
		} *backing;
		
	protected:
		gc_ptr_base(const OwnerType* owner, Backing* backing)
		: owner(owner)
		, backing(backing)
		{}
		
		void* get_void() const { return backing ? backing->to : nullptr; }
		
		template <typename Derived, typename Base>
		gc_ptr<Derived> make_dynamic_cast()
		{
			using DerivedBackingType = typename gc_ptr<Derived>::Backing;

			gc_ptr<Derived> derived;
			
			if (dynamic_cast<Derived*>((Base*)get_void()))
			{
				derived.owner = owner;
				derived.backing = (DerivedBackingType*)backing;
			}
			return derived;
		}
		
		friend class gc_pool_base;
		template<typename T> friend class gc_ptr;
		template <typename Derived, typename Base> friend gc_ptr<Derived> dynamic_pointer_cast(gc_ptr<Base>&);
	};

	template<typename T>
	class gc_ptr : public gc_ptr_base
	{
	private:
		template<typename PoolT> friend class gc_pool;

		// Constructors
		gc_ptr(const OwnerType* owner, T* to)
		: gc_ptr_base(owner, to ? new Backing({
			to, make_deleter(), 1 , false
		}) : nullptr)
		{}
		
	public:
		gc_ptr() : gc_ptr(nullptr, nullptr) {}

		gc_ptr(T* to) : gc_ptr(nullptr, to) {}

		// Copy constructor
		gc_ptr(gc_ptr const& other)
		: gc_ptr_base(other.owner, nullptr)
		{
			retain(other.backing);
		}
		
		// Move constructor
		gc_ptr(gc_ptr&& other)
		: gc_ptr_base(other.owner, std::move(other.backing))
		{
			other.backing = nullptr;
		}
		
		// Copy construct from other (hopefully polymorphic) type
		// TODO: Make this safe
		template<typename OtherT>
		gc_ptr(gc_ptr<OtherT> const& other)
		: gc_ptr_base(other.owner, nullptr)
		{
			retain(other.backing);
		}
		
		~gc_ptr()
		{
			gc_pool<T>::instance().remove(*this);
			release();
		}
		
		T* get() { return (T*) get_void(); }
		const T* get() const { return (const T*) get_void(); }
		T* operator->() { return get(); }
		T& operator*() { return *get(); }
		const T& operator*() const { return *get(); }
		
		gc_ptr<T>& operator=(gc_ptr<T> const& other)
		{
			if (other.backing != backing)
			{
				release();
				retain(other.backing);
			}
			return *this;
		}
		
		gc_ptr<T>& operator=(gc_ptr<T> && other)
		{
			if (other.backing != backing)
			{
				release();
				backing = other.backing;
				
				other.backing = nullptr;
			}
			return *this;
		}
		
		bool operator<(gc_ptr<T> const& other) const
		{
			if (!backing)
				return true;
			else if (!other.backing)
				return false;
			else
				return backing->to < other.backing->to;
		}
		
		int refCount() const
		{
			return backing ? backing->refCount : 0;
		}
		
		std::string to_string() const
		{
			return std::string("gc_ptr{ ") + (backing ? to_string(*backing->to) : " null ") + " }";
		}
		
	private:
		void retain(Backing* newBacking)
		{
			if (newBacking)
				newBacking->refCount++;
			
			backing = newBacking;
		}
		
		void release()
		{
			if (backing && --backing->refCount == 0)
			{
				backing->deletePointee();
				delete backing;
			}
			
			backing = nullptr;
		}
		
		Deleter& make_deleter()
		{
			static Deleter sDeleter = [] (void* p) {
				delete (T*) p;
			};
			return sDeleter;
		}
	};
}
