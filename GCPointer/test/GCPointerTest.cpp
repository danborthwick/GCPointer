#include "GarbageCollection.h"
#include "HangingObjectAssertingTest.h"
#include <random>

using namespace gc;
using namespace std;
using namespace testing;

class GCPointerTest : public HangingObjectAssertingTest
{
};

using StringPtr = gc_ptr<string>;

class Concrete : public Object
{
public:
	Concrete(int x) : x(x) {}
	int x;
};

class Parent : public Object
{
public:
	Parent(gc_ptr<Concrete> a, gc_ptr<Concrete> b) : a(a), b(b) {}
	
	gc_ptr<Concrete> a;
	gc_ptr<Concrete> b;
};

const bool cLoggingEnabled = false;

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
	collectGarbage();

	ASSERT_THAT(Object::instanceCount(), Eq(1));
}

TEST_F(GCPointerTest, hangingReciprocalOwnersAreGarbageCollected)
{
	{
		auto scopedPair = makeReciprocalPair();
	}
	ASSERT_THAT(Object::instanceCount(), Eq(2));
	
	collectGarbage();
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
		collectGarbage();
		ASSERT_THAT(Object::instanceCount(), Eq(3));
	}
	
	// Now three objects should be collected
	collectGarbage();
}

TEST_F(GCPointerTest, selfReferencingObjectIsGarbageCollected)
{
	{
		ListNode::Ptr node = make_gc<ListNode>("selfReferencing");
		node->next = node;
	}
	// Out of scope but requires garbage collection due to self-reference
	ASSERT_THAT(Object::instanceCount(), Eq(1));
	
	collectGarbage();
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
	collectGarbage();
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

TEST_F(GCPointerTest, objectsWithMultipleSelfReferencesAreGarbageCollected)
{
	{
		BinaryTreeNode::Ptr node = make_gc<BinaryTreeNode>("node");
		node->left = node->right = node;
	}
	collectGarbage();
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

	collectGarbage();
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

TEST_F(GCPointerTest, mixedClassesAreGarbageCollected)
{
	{
		ListNode::Ptr node = make_gc<MixedNode>("mixed");
		node->next = node;
	}
	ASSERT_THAT(Object::instanceCount(), Eq(1));
	
	collectGarbage();
}

TEST_F(GCPointerTest, pointersToBasicTypes)
{
	{
		gc_ptr<int> pInt = make_gc<int>(5);
		gc_ptr<string> pString = make_gc<string>("Hello World!");
		gc_ptr<vector<string>> pStringVector = make_gc<vector<string>>();
		
		ASSERT_THAT(live_object_count(), Eq(3));
	}
	
	collectGarbage();
}

TEST_F(GCPointerTest, stringPointerConcatentation)
{
	StringPtr a = make_gc<string>("Hello ");
	StringPtr b = make_gc<string>("world!");
	*a += *b;
	
	ASSERT_THAT(*a, Eq("Hello world!"));
}


class IntPointerHolder : public Object
{
public:
	gc_ptr<int> value;
	IntPointerHolder(int aValue)
	: value { make_owned_gc<int>(this, aValue) }
	{}
};

TEST_F(GCPointerTest, pointersToBasicTypesCanBeHeld)
{
	{
		gc_ptr<IntPointerHolder> p = make_gc<IntPointerHolder>(42);
		
		ASSERT_THAT(*p->value, Eq(42));
	}
	
	collectGarbage();
}

TEST_F(GCPointerTest, vectorInitialisation)
{
	using StringVector = vector<StringPtr>;
	
	StringVector v = { make_gc<string>("alpha"), make_gc<string>("beta") };
	
	ASSERT_THAT(*v[1], Eq("beta"));
	
	gc_ptr<StringVector> pv = make_gc<StringVector>(std::initializer_list<StringPtr> {
		make_gc<string>("alpha"), make_gc<string>("beta") });
	
	ASSERT_THAT(*(*pv)[1], Eq("beta"));
}

TEST_F(GCPointerTest, vectorAccumulate)
{
	gc_ptr<vector<StringPtr>> v = make_gc<vector<StringPtr>>(initializer_list<StringPtr> {
		make_gc<string>("Once"),
		make_gc<string>("upon"),
		make_gc<string>("a"),
		make_gc<string>("time")
	});
	
	auto append = [] (StringPtr& accumulator, StringPtr& next) {
		if (!accumulator->empty())
			*accumulator += " ";
		
		*accumulator += *next;
		return accumulator;
	};
	
	gc_ptr<string> joined = accumulate(v->begin(), v->end(), make_gc<string>(), append);
	
	ASSERT_THAT(*joined, Eq("Once upon a time"));
}

TEST_F(GCPointerTest, OwnersNeedNotBeGCObjects)
{
	class NotAnObject {
	public:
		StringPtr value;
		gc_ptr<NotAnObject> selfReference;
		
		NotAnObject(string const& value)
		: value { make_owned_gc<string>(this, value) }
		, selfReference { make_owned_null_gc<NotAnObject>(this) }
		{}
		
		virtual ~NotAnObject() {}
	};
	
	{
		gc_ptr<NotAnObject> p = make_gc<NotAnObject>("Pseudoantidisestablishmentarianism");
		p->selfReference = p;
	}
	collectGarbage();
}

// TODO: Self referencing