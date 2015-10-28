#pragma once

#include "TestObjects.h"
#include <string>

class Base : public InstanceCounted
{
public:
	virtual std::string className() { return "Base"; }
	virtual ~Base() {}
};

class Derived : public Base
{
public:
	std::string className() override { return "Derived"; }
	virtual ~Derived() {}
};

class NotDerivedFromBase {};
