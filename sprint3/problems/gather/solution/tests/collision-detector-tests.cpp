#define _USE_MATH_DEFINES

#include <vector>
#include <sstream>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_contains.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "../src/collision_detector.h"

using namespace collision_detector;
using namespace std::literals;
using Catch::Matchers::WithinAbs;

class TestItemGathererProvider: public ItemGathererProvider {
public:
    TestItemGathererProvider(){
        items_.emplace_back(geom::Point2D{3, 0.5}, 0.2);
        items_.emplace_back(geom::Point2D{3, 1.5}, 0.2);
        gatherers_.emplace_back(geom::Point2D{1,0}, geom::Point2D{5,0}, 0.3);
        gatherers_.emplace_back(geom::Point2D{0,0}, geom::Point2D{4,0}, 0.3);
    }
    size_t ItemsCount()  const override{
        return items_.size();
    }
    Item GetItem(size_t idx) const override {
        return items_.at(idx);
    }
    size_t GatherersCount() const override {
        return gatherers_.size();
    }
    Gatherer GetGatherer(size_t idx) const override {
        return gatherers_.at(idx);
    }
private:
    std::vector<Item> items_;
    std::vector<Gatherer> gatherers_;
};

namespace Catch {
template<>
struct StringMaker<collision_detector::GatheringEvent> {
  static std::string convert(collision_detector::GatheringEvent const& value) {
      std::ostringstream tmp;
      tmp << "(" << value.gatherer_id << "," << value.item_id << "," << value.sq_distance << "," << value.time << ")";

      return tmp.str();
  }
};
}  // namespace Catch 

struct IsRangeMatcher : Catch::Matchers::MatcherGenericBase {
    IsRangeMatcher(std::vector<GatheringEvent> range)
        : range_{std::move(range)} {
            // std::sort(std::begin(range_), std::end(range_));
    }
    IsRangeMatcher(IsRangeMatcher&&) = default;

    template <typename OtherRange>
    bool match(OtherRange other) {
        using std::begin;
        using std::end;

        std::sort(other.begin(), other.end(), [](const GatheringEvent& ev1, const GatheringEvent& ev2){ return ev1.time < ev2.time;});
        return std::equal(begin(range_), end(range_), begin(other), end(other), 
            [](const GatheringEvent& ev1, const GatheringEvent& ev2){
                double EPS{1.e-10};
                return WithinAbs(ev1.time, EPS).match(ev2.time);
            });
    }
    std::string describe() const override {
        return "Is the same as: "s + Catch::rangeToString(range_);
    }
private:
    std::vector<GatheringEvent> range_;
};

IsRangeMatcher IsRangeEqual(std::vector<GatheringEvent>&& range){
    return IsRangeMatcher{std::forward<std::vector<GatheringEvent>>(range)};
}

// std::vector<GatheringEvent> FindGatherEvents(const ItemGathererProvider& provider){
//     return std::vector<GatheringEvent>{{0, 0, 0.25, 0.5}, {0, 1, 0.25, 0.75}};
// }

// Напишите здесь тесты для функции collision_detector::FindGatherEvents
SCENARIO("collision detector"){
    GIVEN("a test collisions"){
        double EPS{1.e-10};
        TestItemGathererProvider data;
        std::vector<GatheringEvent> test_events{{0, 0, 0.25, 0.5}, {0, 1, 0.25, 0.75}};
        std::vector<GatheringEvent> events = FindGatherEvents(data);
        WHEN("vector has right size"){
            REQUIRE(test_events.size() == events.size());
            THEN("test time order"){
                for(int i = 1; i < events.size(); ++i){
                    CHECK(events[i-1].time < events[i].time);
                    CHECK_THAT(events[i-1].time, !WithinAbs(events[i].time, EPS));
                }
                for(size_t i = 0; i < test_events.size(); ++i){
                    CHECK(events[i].gatherer_id == test_events[i].gatherer_id);
                    CHECK(events[i].item_id == test_events[i].item_id);
                    CHECK_THAT(events[i].sq_distance, WithinAbs(test_events[i].sq_distance, EPS));
                }
            }
            AND_THEN("check vector equality"){

            }
        }
    }
}
