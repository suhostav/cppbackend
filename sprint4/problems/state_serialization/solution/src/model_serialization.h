#include <vector>
#include <boost/serialization/vector.hpp>

#include "model.h"

namespace geom {

template <typename Archive>
void serialize(Archive& ar, Point2D& point, [[maybe_unused]] const unsigned version) {
    ar& point.x;
    ar& point.y;
}

template <typename Archive>
void serialize(Archive& ar, Vec2D& vec, [[maybe_unused]] const unsigned version) {
    ar& vec.x;
    ar& vec.y;
}

}  // namespace geom

namespace model {

template <typename Archive>
void serialize(Archive& ar, DogSpeed& ds, [[maybe_unused]] const unsigned version){
    ar & ds.hs;
    ar & ds.vs;
}

template <typename Archive>
void serialize(Archive& ar, Loot& loot, [[maybe_unused]] const unsigned version) {
    ar & loot.id_;
    ar & loot.pos_;
    ar & loot.taken;
    ar & loot.type_;
}

}  // namespace model

namespace serialization {

// LootRepr (LootRepresentation) - сериализованное представление класса Loot
class LootRepr {
public:
    LootRepr() = default;

    explicit LootRepr(const model::Loot& loot)
        : id_(loot.id_)
        , type_(loot.type_)
        , pos_(loot.pos_) {
    }

    [[nodiscard]] model::Loot Restore() const {
        model::Loot loot{id_, type_, pos_};
        return loot;
    }

    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar& id_;
        ar& type_;
        ar& pos_;
    }

private:
    size_t id_;
    int type_;
    geom::Point2D pos_;
};

// DogRepr (DogRepresentation) - сериализованное представление класса Dog
class DogRepr {
public:
    DogRepr() = default;

    explicit DogRepr(const model::Dog& dog)
        : id_(dog.GetId())
        , name_(dog.GetName())
        , pos_(dog.GetPoint2D())
        , bag_capacity_(dog.GetBagCapacity())
        , speed_(dog.GetSpeed())
        , direction_(dog.GetDir())
        , limit_(dog.GetLimit())
        , score_(dog.GetScore())
        , bag_content_(std::move(CreateBagRepr(dog.GetBagContent()))) {
    }

    [[nodiscard]] model::Dog Restore() const {
        model::Dog dog{id_, name_, pos_, speed_, bag_capacity_};
        dog.SetSpeed(speed_);
        dog.SetDir(direction_, limit_);
        dog.AddScore(score_);
        for (const auto& item : bag_content_) {
            if (!dog.AddLoot(item.Restore())) {
                throw std::runtime_error("Failed to put bag content");
            }
        }
        return dog;
    }

    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar& id_;
        ar& name_;
        ar& pos_;
        ar& bag_capacity_;
        ar& speed_;
        ar& direction_;
        ar& limit_;
        ar& score_;
        ar& bag_content_;
    }

private:
    std::vector<LootRepr> CreateBagRepr(const model::Dog::BagContent& bag_content){
        bag_content_.clear();
        for(const model::Loot& loot : bag_content){
            bag_content_.emplace_back(loot);
        }
        return bag_content_;
    }
    uint64_t id_{0u};
    std::string name_;
    geom::Point2D pos_;
    size_t bag_capacity_ = 0;
    model::DogSpeed speed_;
    model::DogDir direction_ = model::DogDir::NORTH;
    model::DCoord limit_;
    int score_ = 0;
    std::vector<LootRepr> bag_content_;
};

class SessionRepr {
public:
    SessionRepr() = default;
    explicit SessionRepr(const model::GameSession& session)
        : random_point_(session.IsRandomPoint())
        , dog_width_(session.GetDogWidth())
        , loot_width_(session.GetLootWidth())
        , office_width_(session.GetOfficeWidth())
        , map_id_(*(session.GetMap()->GetId()))
        , dogs_(GetDogsRepr(session)){

    }
    std::vector<DogRepr> GetDogsRepr(const model::GameSession& session){
        std::vector<serialization::DogRepr> dogs_repr;
        for(const model::Dog& dog : session.GetDogs()){
            dogs_repr.emplace_back(dog);
        }
        return dogs_repr;
    }

    std::vector<LootRepr> GetLootsRepr(const model::GameSession& session){
        std::vector<LootRepr> loots_repr;
        for(const model::Loot& loot : session.GetLoots()){
            loots_repr.emplace_back(loot);
        }
        return loots_repr;
    }

    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar& random_point_;
        ar& dog_width_;
        ar& loot_width_;
        ar& office_width_;
        ar& map_id_;
        ar& dogs_;
    }

    const std::string GetMapId() const {
        return map_id_;
    }

//     model::GameSession Restore (model::Map* map, loot_gen::LootGenerator::TimeInterval loot_period, double loot_probability){
//         model::GameSession session{map, random_point_, loot_period, loot_probability};
//         for(DogRepr dr : dogs_){
//             session.AddDog(dr.Restore());
//         }
//         return session;
//     }   

// private:
    bool random_point_;
    double dog_width_;
    double loot_width_;
    double office_width_;
    std::string map_id_;
    std::vector<DogRepr> dogs_;
};

class TokenRepr{
public:
    TokenRepr() = default;
    TokenRepr(std::uint64_t dog_id, std::string token)
        :dog_id_(dog_id)
        ,token_(token){

        }
    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar & dog_id_;
        ar & token_;
    }
    std::uint64_t GetDogId() const {
        return dog_id_;
    }
    const std::string& GetToken() const {
        return token_;
    }
private:
    std::uint64_t dog_id_;
    std::string token_;
};

}  // namespace serialization
