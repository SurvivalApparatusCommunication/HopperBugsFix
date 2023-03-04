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
#include "llapi/mc/LevelChunk.hpp"
#include "llapi/ScheduleAPI.h"
#include "llapi/ParticleAPI.h"
#include "gsl/gsl"

#include "version.h"

extern Logger logger;

void PluginInit() {
}

TInstanceHook(bool,
              "?canPullOutItem@JukeboxBlockActor@@UEBA_NHHAEBVItemStack@@@Z",
              JukeboxBlockActor,
              int uk0,
              int uk1,
              class ItemStack const& item) {
    return dAccess<int>(this, 632 - 240) > 2;
}

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

    ChunkPos minChunk(((int)std::floor(aabb.min.x - 0.125f)) >> 4, ((int)std::floor(aabb.min.z - 0.125f)) >> 4);
    ChunkPos maxChunk(((int)std::floor(aabb.max.x + 0.125f)) >> 4, ((int)std::floor(aabb.max.z + 0.125f)) >> 4);

    std::vector<gsl::not_null<class Actor*>>().swap(tempItems);
    std::vector<class Actor*> items;
    items.clear();

    for (int x = minChunk.x; x <= maxChunk.x; x++)
        for (int z = minChunk.z; z <= maxChunk.z; z++) {
            LevelChunk* chunk = getChunk({x, z});
            if (chunk != nullptr) {
                chunk->getEntities(ActorType::ItemEntity, aabb, items, false);
            }
        }
    tempItems.reserve(items.size());
    for (auto& item : items) {
        tempItems.emplace_back(gsl::make_not_null(item));
    }
    return gsl::span(tempItems);
}