#include "GCPointer.h"
#include "HangingObjectAssertingTest.h"
#include <random>

using namespace gc;
using namespace std;
using namespace testing;

class GCPointerTest : public HangingObjectAssertingTest
{
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

void gc::Log(string const& s)
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
	ASSERT_THAT(Object::instanceCount(), Eq(2));
	
	collectGarbage<ListNode>();
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

TEST_F(GCPointerTest, selfReferencingObjectIsGarbageCollected)
{
	{
		ListNode::Ptr node = make_gc<ListNode>("selfReferencing");
		node->next = node;
	}
	// Out of scope but requires garbage collection due to self-reference
	ASSERT_THAT(Object::instanceCount(), Eq(1));
	
	collectGarbage<ListNode>();
}

TEST_F(GCPointerTest, fourObjectChainIsGarbageCollected)
{
	{
		ListNode::Ptr nodes[4] =  {
			make_gc<ListNode>("a"),
			make_gc<ListNode>("b"),
			make_gc<ListNode>("c"),
			make_gc<ListNode>("d")
		};

		for (int i=0; i < 4; i++)
			nodes[i]->next = nodes[i % 4];
	}
	collectGarbage<ListNode>();
}

class BinaryTreeNode : public Object
{
public:
	using Ptr = gc_ptr<BinaryTreeNode>;
	
	BinaryTreeNode(string value)
	: value { value }
	, left { make_owned_null_gc<BinaryTreeNode>(this) }
	, right { make_owned_null_gc<BinaryTreeNode>(this) }
	{}
	
	string value;
	Ptr left;
	Ptr right;
	
	string to_string() const override
	{
		return "BinaryTreeNode{" + value + "}";
	}
};

template<>
gc_pool<BinaryTreeNode> gc_pool<BinaryTreeNode>::sInstance {};

TEST_F(GCPointerTest, objectsWithMultipleSelfReferencesAreGarbageCollected)
{
	{
		BinaryTreeNode::Ptr node = make_gc<BinaryTreeNode>("node");
		node->left = node->right = node;
	}
	collectGarbage<BinaryTreeNode>();
}

class Network
{
public:
	const float nullPointerProportion = 0.2f;

	vector<BinaryTreeNode::Ptr> nodes;
	default_random_engine randomGenerator;
	uniform_int_distribution<int> distribution;
	
	Network(int nodeCount)
	: nodes(nodeCount)
	, distribution(0, nodeCount * (1.f + nullPointerProportion))
	{
		for (int i=0; i < nodeCount; i++)
			nodes[i] = make_gc<BinaryTreeNode>(to_string(i));
		
		for (auto &ptr : nodes)
		{
			ptr->left = pointerToRandomNode();
			ptr->right = pointerToRandomNode();
		}
	}

	BinaryTreeNode::Ptr pointerToRandomNode()
	{
		int index = distribution(randomGenerator);
		return (index < nodes.size()) ? nodes[index] : gc_ptr<BinaryTreeNode>();
	};

};

TEST_F(GCPointerTest, randomNetworkIsGarbageCollected)
{
	const int nodeCount = 100;

	{
		Network network(nodeCount);
		ASSERT_THAT(Object::instanceCount(), Eq(nodeCount));
	}

	ASSERT_THAT(Object::instanceCount(), AllOf(Gt(0), Lt(nodeCount)));

	collectGarbage<BinaryTreeNode>();
}

class MixedNode : public ListNode
{
public:
	using Ptr = gc_ptr<MixedNode>;
	string value;
	BinaryTreeNode::Ptr binaryNode;
	
	MixedNode(string value)
	: ListNode(value)
	, binaryNode { make_owned_null_gc<BinaryTreeNode>(this) }
	{}

};

//TEST_F(GCPointerTest, pointersCanBeDowncast)
//{
//	ListNode::Ptr base = make_gc<ListNode>(new MixedNode("mixed"));
//	MixedNode::Ptr derived;
//	derived = base;
//	//TODO: Provide downcast constructor or utility function?
//}
//
//TEST_F(GCPointerTest, mixedClassesAreGarbageCollected)
//{
//	{
//		ListNode::Ptr node = make_gc<ListNode>(new MixedNode("mixed"));
//		node->next = node;
//		
//		// TODO: Downcasting
//	}
//	
//}
