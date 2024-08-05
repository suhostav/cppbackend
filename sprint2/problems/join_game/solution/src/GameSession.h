#include "model.h"
#include "Dog.h"
#include <unordered_map>
#include <vector>

class GameSession {
public:
    GameSession(model::Map* map): map_(map) {
    }
    Dog* AddDog(Dog dog){
        dogs_.push_back(std::move(dog));
        return &dogs_.back();
    }
    model::Map* GetMap(){
        return map_;
    }

private:
    std::vector<Dog> dogs_;
    model::Map* map_;
};
