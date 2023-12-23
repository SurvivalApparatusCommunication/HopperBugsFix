/**
 * @file plugin.cpp
 * @brief The main file of the plugin
 */

#include <llapi/LoggerAPI.h>
#include <llapi/mc/JukeboxBlockActor.hpp>
#include <llapi/mc/ItemStack.hpp>
#include <llapi/mc/ItemActor.hpp>
#include <llapi/mc/Actor.hpp>
#include <llapi/mc/Actor.hpp>
#include <llapi/mc/BlockSource.hpp>
#include <llapi/mc/Level.hpp>
#include "llapi/mc/Vec2.hpp"
#include "llapi/mc/Vec3.hpp"
#include "llapi/mc/BlockPos.hpp"
#include "llapi/mc/Block.hpp"
#include "llapi/mc/Hopper.hpp"
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

TInstanceHook(bool,
              "?_tryAddItemsFromPos@Hopper@@AEAA_NAEAVBlockSource@@AEAVContainer@@AEBVVec3@@@Z",
              Hopper,
              class BlockSource& blockSource,
              class Container& container,
              class Vec3 const& pos) {
    AABB aabb;

    if (isEntity()) {
        aabb.min = pos.sub(0.5f, 0.0f, 0.5f);
        aabb.max = pos.add(0.5f, 1.0f, 0.5f);
    } else {
        aabb.min = pos;
        aabb.min.y -= 0.375f;
        aabb.max = pos + 1;
    }

    ChunkPos minChunk(((int)std::floor(aabb.min.x - 0.125f)) >> 4, ((int)std::floor(aabb.min.z - 0.125f)) >> 4);
    ChunkPos maxChunk(((int)std::floor(aabb.max.x + 0.125f)) >> 4, ((int)std::floor(aabb.max.z + 0.125f)) >> 4);

    for (int x = minChunk.x; x <= maxChunk.x; x++)
        for (int z = minChunk.z; z <= maxChunk.z; z++) {
            LevelChunk* chunk = blockSource.getChunk({x, z});
            if (chunk == nullptr) {
                continue;
            }
            for (auto& weakEntityRef : chunk->getChunkEntities()) {
                auto* actor = weakEntityRef.tryUnwrap<Actor>();
                if (actor == nullptr || !ActorClassTree::isInstanceOf(*actor, ActorType::ItemEntity) ||
                    !aabb.intersects(actor->getAABB()) || actor->isRemoved()) {
                    continue;
                }
                auto* item = ((ItemActor*)(actor))->getItemStack();
                if (item == nullptr || item->isNull()) {
                    continue;
                }
                if (_addItem(blockSource, container, *item, -1, item->getCountNoCheck())) {
                    if (item->getCountNoCheck() != 0) {
                        _addItem(blockSource, container, *item, -1, item->getCountNoCheck()); // need to add twice
                    }
                    if (item->getCountNoCheck() == 0) {
                        actor->remove();
                    }
                    return true;
                }
            }
        }

    return false;
}

TInstanceHook(bool, "?isContainerBlock@Block@@QEBA_NXZ", Block) {
    return this == StaticVanillaBlocks::mComposter || original(this);
}
