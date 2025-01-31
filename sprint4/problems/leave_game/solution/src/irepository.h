#pragma once
#include <string>
#include "Dog.h"

class repository {
public:
    virtual void CreateTable() = 0;
    virtual void AddResult(const model::Dog& dog) = 0;
    virtual std::string GetResults() = 0;

    virtual ~repository() = default;
};