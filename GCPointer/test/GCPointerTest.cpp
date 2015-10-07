#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "GCPointer.h"

using namespace std;
using namespace testing;

class GCPointerTest : public Test
{
	void SetUp() override
	{
		Object::resetInstanceCount();
	}
	
	void TearDown() override
	{
		ASSERT_THAT(Object::instanceCount(), Eq(0));
	}
};

class Concrete : public Object
{
public:
	Concrete(int x) : x(x) {}
	int x;
};

template<> gc_pool<Concrete> gc_pool<Concrete>::sInstance {};

class Parent : public Object
{
public:
	Parent(gc_ptr<Concrete> a, gc_ptr<Concrete> b) : a(a), b(b) {}
	
	gc_ptr<Concrete> a;
	gc_ptr<Concrete> b;
};

template<> gc_pool<Parent> gc_pool<Parent>::sInstance {};

const bool cLoggingEnabled = true;

void Log(string const& s)
{
	if (cLoggingEnabled)
		cerr << s << "\n";
}

TEST_F(GCPointerTest, pointerCanBeCreated)
{
	{
		gc_ptr<Concrete> a = make_gc<Concrete>(42);
		ASSERT_THAT(a->x, Eq(42));
	}
}

TEST_F(GCPointerTest, pointerWithParentCanBeCreated)
{
	{
		gc_ptr<Parent> p = make_gc<Parent>(make_gc<Concrete>(1), make_gc<Concrete>(2));
		ASSERT_THAT(p->a->x, Eq(1));
		ASSERT_THAT(Object::instanceCount(), Eq(3));
	}
}

TEST_F(GCPointerTest, pointersCanBeCopied)
{
	{
		gc_ptr<Concrete> b;
		
		{
			gc_ptr<Concrete> a = make_gc<Concrete>(42);
			b = a;
		}
		ASSERT_THAT(Object::instanceCount(), Eq(1));
	}
}

class ListNode : public Object
{
public:
	using Ptr = gc_ptr<ListNode>;
	Ptr next;
	string value;
	
	ListNode(string value)
	: next { make_owned_null_gc<ListNode>(this) }
	, value { value }
	{}
	
	virtual ~ListNode()
	{
		Log(string("Destroying ") + to_string());
	}
	
	string to_string() const override
	{
		return "ListNode{" + value + "}";
	}
	
	MOCK_METHOD0(destructor, void());
};

template<>
gc_pool<ListNode> gc_pool<ListNode>::sInstance {};

pair<ListNode::Ptr, ListNode::Ptr> makeReciprocalPair()
{
	ListNode::Ptr first = make_gc<ListNode>("first");
	ListNode::Ptr second = make_gc<ListNode>("second");
	
	first->next = second;
	second->next = first;
	
	return { first, second };
}

TEST_F(GCPointerTest, scopedPointersAreNotGarbageCollected)
{
	ListNode::Ptr scoped = make_gc<ListNode>("scoped");
	collectGarbage<ListNode>();
	
	ASSERT_THAT(Object::instanceCount(), Eq(1));
}

TEST_F(GCPointerTest, hangingReciprocalOwnersAreGarbageCollected)
{
	{
		auto scopedPair = makeReciprocalPair();
	}
	Log(gc_pool<ListNode>::sInstance.to_string());
	ASSERT_THAT(Object::instanceCount(), Eq(2));
	
	collectGarbage<ListNode>();
	Log(gc_pool<ListNode>::sInstance.to_string());
}

TEST_F(GCPointerTest, ownedReciprocalOwnersAreNotGarbageCollected)
{
	{
		ListNode::Ptr root = make_gc<ListNode>("root");
		
		{
			auto scopedPair = makeReciprocalPair();
			root->next = scopedPair.first;
		}
		ASSERT_THAT(Object::instanceCount(), Eq(3));
		
		// Should do nothing as root is still in scope
		collectGarbage<ListNode>();
		ASSERT_THAT(Object::instanceCount(), Eq(3));
	}
	
	// Now three objects should be collected
	collectGarbage<ListNode>();
}

