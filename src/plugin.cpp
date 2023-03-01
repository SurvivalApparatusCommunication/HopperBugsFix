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

// TInstanceHook(bool,
//               "?canPullOutItem@JukeboxBlockActor@@UEBA_NHHAEBVItemStack@@@Z",
//               JukeboxBlockActor,
//               int uk0,
//               int uk1,
//               class ItemStack const& item) {
//     bool res = original(this, uk0, uk1, item);
//     // logger.info("{} {} {} {} {} {} {}", uk0, uk1, item.getTypeName(), res, (int)dAccess<bool>(this, 388),
//     //             (int)dAccess<bool>(this, 628), dAccess<int>(this, 632));
//     return true;
// }

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
    // if (i == 0) {
    //     pt().drawCuboid(aabb, this->getDimensionId(), mce::ColorPalette::GREEN);
    // }
    auto items = fetchEntities2(type, aabb, actor);
    std::vector<gsl::not_null<class Actor*>> nitems;
    nitems.clear();
    for (auto& item : items) {
        if (item != nullptr) {
            nitems.push_back(gsl::make_not_null(item));
        }
    }
    return gsl::span(nitems);
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