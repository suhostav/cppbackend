#include "model.h"
#include <unordered_map>

class GameSession {
public:
    GameSession(model::Map& map): map_(map) {

    }

private:
    model::Map& map_;
};
