#pragma once

#include "TestObjects.h"
#include <string>

class Base : public InstanceCounted
{
public:
	virtual const std::string className() const { return "Base"; }
	virtual ~Base() {}
};

class Derived : public Base
{
public:
	const std::string className() const override
	{
		static const std::string sClassName = "Derived";
		return sClassName;
	}
	
	virtual ~Derived() {}
	int x = 42;
};

class NotDerivedFromBase {};
