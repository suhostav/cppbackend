#pragma once
#include <string>
#include "Dog.h"

class repository {
public:
    virtual void CreateTable() = 0;
    virtual void AddResult(const model::Dog& dog) = 0;
    virtual std::string GetResults(int start, int32_t max_items) = 0;

    virtual ~repository() = default;
};