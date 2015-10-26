#pragma once

#include "GCObject.h"
#include <string>

class Base : public gc::Object
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
