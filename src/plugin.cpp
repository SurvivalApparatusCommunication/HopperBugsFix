/**
 * @file plugin.cpp
 * @brief The main file of the plugin
 */

#include <llapi/LoggerAPI.h>
#include <llapi/mc/JukeboxBlockActor.hpp>
#include <llapi/mc/ItemStack.hpp>
#include <llapi/mc/Actor.hpp>
#include <llapi/mc/Actor.hpp>
#include <llapi/mc/BlockSource.hpp>
#include <llapi/mc/Level.hpp>
#include "llapi/mc/Vec2.hpp"
#include "llapi/mc/Vec3.hpp"
#include "llapi/mc/BlockPos.hpp"
#include "llapi/mc/AABB.hpp"
#include "llapi/mc/BoundingBox.hpp"
#include "llapi/mc/ChunkPos.hpp"
#include "llapi/mc/ChunkBlockPos.hpp"
#include "llapi/ScheduleAPI.h"
#include "llapi/ParticleAPI.h"
#include "gsl/gsl"

#include "version.h"

extern Logger logger;

// int i = 0;

void PluginInit() {
    // Schedule::repeat(
    //     [&]() {
    //         if (i < 39) {
    //             i += 1;
    //         } else {
    //             i = 0;
    //         }
    //     },
    //     1);
}

// ParticleCUI& pt() {
//     static ParticleCUI pt = ParticleCUI();
//     return pt;
// }

std::unordered_set<JukeboxBlockActor*> sbSet;

TInstanceHook(bool,
              "?canPullOutItem@JukeboxBlockActor@@UEBA_NHHAEBVItemStack@@@Z",
              JukeboxBlockActor,
              int uk0,
              int uk1,
              class ItemStack const& item) {
    bool res = original(this, uk0, uk1, item);
    // logger.info("{} {} {} {} {} {} {}", uk0, uk1, item.getTypeName(), res, (int)dAccess<bool>(this, 388),
    //             (int)dAccess<bool>(this, 628), dAccess<int>(this, 632));
    if (!res) {
        auto iter = sbSet.find(this);

        if (iter == sbSet.end()) {
            sbSet.insert(this);
            return res;
        }
        // logger.info("{} erase",__LINE__);
        sbSet.erase(iter);
        return true;
    }
    return true;
}
TInstanceHook(void,
              "?onChanged@JukeboxBlockActor@@UEAAXAEAVBlockSource@@@Z",
              JukeboxBlockActor,
              class BlockSource& bs) {
    // logger.info("{} erase", __LINE__);
    sbSet.erase(this);
    original(this, bs);
}
// TInstanceHook(void,
//               "?stopPlayingRecord@JukeboxBlockActor@@QEBAXAEAVBlockSource@@@Z",
//               JukeboxBlockActor,
//               class BlockSource& bs) {
//     logger.info("{} erase", __LINE__);
//     sbSet.erase(this);
//     original(this, bs);
// }
// TInstanceHook(void,
//               "?setRecord@JukeboxBlockActor@@QEAAXAEBVItemStack@@@Z",
//               JukeboxBlockActor,
//               class ItemStack const& i) {
//     logger.info("{} erase", __LINE__);
//     sbSet.erase(this);
//     original(this, i);
// }

std::vector<gsl::not_null<class Actor*>> tempItems;

TInstanceHook(gsl::span<gsl::not_null<class Actor*>>,
              "?fetchEntities@BlockSource@@UEAA?AV?$span@V?$not_null@PEAVActor@@@gsl@@$0?"
              "0@gsl@@W4ActorType@@AEBVAABB@@PEBVActor@@@Z",
              BlockSource,
              enum class ActorType type,
              class AABB const& aabb,
              class Actor const* actor) {
    if (type != ActorType::ItemEntity || (aabb.max.x - aabb.min.x > 1) || (aabb.max.z - aabb.min.z > 1)) {
        return original(this, type, aabb, actor);
    }
    Vec3 center = aabb.getCenter();
    if (abs(std::round(center.x / 16.0f) * 16.0f - center.x) > 1 &&
        abs(std::round(center.z / 16.0f) * 16.0f - center.z) > 1) {
        return original(this, type, aabb, actor);
    }
    AABB expandedAABB = aabb;
    expandedAABB.min.x -= 0.125f;
    expandedAABB.min.z -= 0.125f;
    expandedAABB.max.x += 0.125f;
    expandedAABB.max.z += 0.125f;
    auto items = original(this, type, expandedAABB, actor);

    if (items.empty()) {
        return items;
    }
    std::vector<gsl::not_null<class Actor*>>().swap(tempItems);
    tempItems.reserve(items.size());
    for (auto& item : items) {
        if (aabb.intersects(item->getAABB())) {
            tempItems.push_back(item);
        }
    }
    return gsl::span(tempItems);
}

TInstanceHook(bool, "?intersectsInner@AABB@@QEBA_NAEBV1@@Z", AABB, class AABB const& aabb) {
    return intersects(aabb);
}

// TInstanceHook(gsl::span<gsl::not_null<class Actor*>>,
//               "?fetchEntities@BlockSource@@UEAA?AV?$span@V?$not_null@PEAVActor@@@gsl@@$0?"
//               "0@gsl@@PEBVActor@@AEBVAABB@@_N2@Z",
//               BlockSource,
//               class Actor const* actor,
//               class AABB const& aabb,
//               bool uk0,
//               bool uk1) {
//     // if (actor != nullptr)
//     //     logger.info("{}", actor->getTypeName());
//     if (i == 0) {
//         pt().drawCuboid(aabb, this->getDimensionId());
//     }
//     return original(this, actor, aabb, uk0, uk1);
// }
