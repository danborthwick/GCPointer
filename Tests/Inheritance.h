#pragma once

#include <string>

class Base
{
public:
	virtual std::string className() { return "Base"; }
};

class Derived : public Base
{
public:
	std::string className() override { return "Derived"; }
};

class NotDerivedFromBase {};
