#include "collision_detector.h"
#include <cassert>

namespace collision_detector {

CollectionResult TryCollectPoint(geom::Point2D a, geom::Point2D b, geom::Point2D c) {
    // Проверим, что перемещение ненулевое.
    // Тут приходится использовать строгое равенство, а не приближённое,
    // пскольку при сборе заказов придётся учитывать перемещение даже на небольшое
    // расстояние.
    assert(b.x != a.x || b.y != a.y);
    const double u_x = c.x - a.x;
    const double u_y = c.y - a.y;
    const double v_x = b.x - a.x;
    const double v_y = b.y - a.y;
    const double u_dot_v = u_x * v_x + u_y * v_y;
    const double u_len2 = u_x * u_x + u_y * u_y;
    const double v_len2 = v_x * v_x + v_y * v_y;
    const double proj_ratio = u_dot_v / v_len2;
    const double sq_distance = u_len2 - (u_dot_v * u_dot_v) / v_len2;

    return CollectionResult(sq_distance, proj_ratio);
}

std::vector<GatheringEvent> FindGatherEvents(const ItemGathererProvider& provider) {
    std::vector<GatheringEvent> detected_events;
    for(size_t i = 0; i < provider.GatherersCount(); ++i){
        Gatherer gatherer = provider.GetGatherer(i);
        if(gatherer.start_pos == gatherer.end_pos){
            continue;
        }
        for(size_t j = 0; j < provider.ItemsCount(); ++j){
            Item item = provider.GetItem(j);
            //если перемещение не нулевое
            if(gatherer.start_pos.x != gatherer.end_pos.x || gatherer.start_pos.y != gatherer.end_pos.y){
                auto result = TryCollectPoint(gatherer.start_pos, gatherer.end_pos, item.position);
                if(result.IsCollected(gatherer.width + item.width)){
                    detected_events.emplace_back(j, i, result.sq_distance, result.proj_ratio);
                }
            }
        }
    }

    std::sort(detected_events.begin(), detected_events.end(),
        [](const GatheringEvent& l, const GatheringEvent& r) {
            return l.time < r.time;
        });

    return detected_events;
}


}  // namespace collision_detector