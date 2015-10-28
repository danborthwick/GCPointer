#pragma once

#include "GarbageCollection.h"
#include <string>

using namespace gc;
using namespace std;

class InstanceCounted
{
public:
	InstanceCounted()
	{
		sInstanceCount++;
	}
	
	virtual ~InstanceCounted()
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


class ListNode : public InstanceCounted
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
	
	static pair<ListNode::Ptr, ListNode::Ptr> makeReciprocalPair()
	{
		ListNode::Ptr first = make_gc<ListNode>("first");
		ListNode::Ptr second = make_gc<ListNode>("second");
		
		first->next = second;
		second->next = first;
		
		return { first, second };
	}
};

class BinaryTreeNode : public InstanceCounted
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

// Has member with different type to this
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
