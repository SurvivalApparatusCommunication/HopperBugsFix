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
#include "llapi/mc/Block.hpp"
#include "llapi/mc/AABB.hpp"
#include "llapi/mc/StaticVanillaBlocks.hpp"
#include "llapi/mc/BoundingBox.hpp"
#include "llapi/mc/ChunkPos.hpp"
#include "llapi/mc/ChunkBlockPos.hpp"
#include "llapi/mc/LevelChunk.hpp"
#include "llapi/mc/LevelChunk.hpp"
#include "llapi/ScheduleAPI.h"
#include "llapi/mc/WeakStorageEntity.hpp"
#include "llapi/mc/WeakEntityRef.hpp"
#include "llapi/mc/ActorClassTree.hpp"
#include "llapi/mc/Scheduler.hpp"
#include "gsl/gsl"

#include "version.h"

extern Logger logger;

void PluginInit() {}

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
    if (type != ActorType::ItemEntity) {
        return original(this, type, aabb, actor);
    }

    ChunkPos minChunk(((int)std::floor(aabb.min.x - 0.125f)) >> 4, ((int)std::floor(aabb.min.z - 0.125f)) >> 4);
    ChunkPos maxChunk(((int)std::floor(aabb.max.x + 0.125f)) >> 4, ((int)std::floor(aabb.max.z + 0.125f)) >> 4);

    std::vector<gsl::not_null<class Actor*>>().swap(tempItems);
    for (int x = minChunk.x; x <= maxChunk.x; x++)
        for (int z = minChunk.z; z <= maxChunk.z; z++) {
            LevelChunk* chunk = getChunk({x, z});
            if (chunk != nullptr) {
                for (auto& weakEntityRef : chunk->getChunkEntities()) {
                    Actor* actor = weakEntityRef.tryUnwrap();
                    if (actor != nullptr && ActorClassTree::isInstanceOf(*actor, ActorType::ItemEntity) &&
                        aabb.intersects(actor->getAABB())) {
                        tempItems.emplace_back(actor);
                    }
                }
            }
        }
    return gsl::span(tempItems);
}

TInstanceHook(bool, "?isContainerBlock@Block@@QEBA_NXZ", Block) {
    return this == StaticVanillaBlocks::mComposter || original(this);
}

// TInstanceHook(void,
//               "?processCoroutines@Scheduler@@QEAAXV?$duration@_JU?$ratio@$00$0DLJKMKAA@@std@@@chrono@std@@0@Z",
//               Scheduler,
//                std::chrono::duration<int64_t, struct std::ratio<1, 1000000000>> r1,
//                std::chrono::duration<int64_t, struct std::ratio<1, 1000000000>> r2) {
//     original(this, r1, std::chrono::duration<int64_t, struct std::ratio<1, 1000000000>>{0});
// }