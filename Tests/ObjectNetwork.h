#pragma once

#include "TestObjects.h"
#include <random>

using namespace std;

class ObjectNetwork
{
public:
	const float nullPointerProportion = 0.2f;
	
	vector<BinaryTreeNode::Ptr> nodes;
	default_random_engine randomGenerator;
	uniform_int_distribution<int> distribution;
	
	ObjectNetwork(int nodeCount)
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