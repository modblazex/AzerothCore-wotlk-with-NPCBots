/*
 * This file is part of the AzerothCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "MapTree.h"
#include "Errors.h"
#include "Log.h"
#include "Metric.h"
#include "ModelInstance.h"
#include "VMapDefinitions.h"
#include "VMapMgr2.h"
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>

using G3D::Vector3;

namespace VMAP
{
    class MapRayCallback
    {
    public:
        MapRayCallback(ModelInstance* val, ModelIgnoreFlags ignoreFlags): prims(val), flags(ignoreFlags), hit(false) { }
        bool operator()(const G3D::Ray& ray, uint32 entry, float& distance, bool StopAtFirstHit)
        {
            bool result = prims[entry].intersectRay(ray, distance, StopAtFirstHit, flags);
            if (result)
            {
                hit = true;
            }
            return result;
        }
        bool didHit() { return hit; }
    protected:
        ModelInstance* prims;
        ModelIgnoreFlags flags;
        bool hit;
    };

    class AreaInfoCallback
    {
    public:
        AreaInfoCallback(ModelInstance* val): prims(val) {}
        void operator()(const Vector3& point, uint32 entry)
        {
#if defined(VMAP_DEBUG)
            LOG_DEBUG("maps", "AreaInfoCallback: trying to intersect '{}'", prims[entry].name);
#endif
            prims[entry].intersectPoint(point, aInfo);
        }

        ModelInstance* prims;
        AreaInfo aInfo;
    };

    class LocationInfoCallback
    {
    public:
        LocationInfoCallback(ModelInstance* val, LocationInfo& info): prims(val), locInfo(info), result(false) {}
        void operator()(const Vector3& point, uint32 entry)
        {
#if defined(VMAP_DEBUG)
            LOG_DEBUG("maps", "LocationInfoCallback: trying to intersect '{}'", prims[entry].name);
#endif
            if (prims[entry].GetLocationInfo(point, locInfo))
            {
                result = true;
            }
        }

        ModelInstance* prims;
        LocationInfo& locInfo;
        bool result;
    };

    //=========================================================

    std::string StaticMapTree::getTileFileName(uint32 mapID, uint32 tileX, uint32 tileY)
    {
        std::stringstream tilefilename;
        tilefilename.fill('0');
        tilefilename << std::setw(3) << mapID << '_';
        //tilefilename << std::setw(2) << tileX << '_' << std::setw(2) << tileY << ".vmtile";
        tilefilename << std::setw(2) << tileY << '_' << std::setw(2) << tileX << ".vmtile";
        return tilefilename.str();
    }

    bool StaticMapTree::GetAreaInfo(Vector3& pos, uint32& flags, int32& adtId, int32& rootId, int32& groupId) const
    {
        AreaInfoCallback intersectionCallBack(iTreeValues);
        iTree.intersectPoint(pos, intersectionCallBack);
        if (intersectionCallBack.aInfo.result)
        {
            flags = intersectionCallBack.aInfo.flags;
            adtId = intersectionCallBack.aInfo.adtId;
            rootId = intersectionCallBack.aInfo.rootId;
            groupId = intersectionCallBack.aInfo.groupId;
            pos.z = intersectionCallBack.aInfo.ground_Z;
            return true;
        }
        return false;
    }

    bool StaticMapTree::GetLocationInfo(const Vector3& pos, LocationInfo& info) const
    {
        LocationInfoCallback intersectionCallBack(iTreeValues, info);
        iTree.intersectPoint(pos, intersectionCallBack);
        return intersectionCallBack.result;
    }

    StaticMapTree::StaticMapTree(uint32 mapID, const std::string& basePath)
        : iMapID(mapID), iIsTiled(false), iTreeValues(0), iBasePath(basePath)
    {
        if (iBasePath.length() > 0 && iBasePath[iBasePath.length() - 1] != '/' && iBasePath[iBasePath.length() - 1] != '\\')
        {
            iBasePath.push_back('/');
        }
    }

    //=========================================================
    //! Make sure to call unloadMap() to unregister acquired model references before destroying
    StaticMapTree::~StaticMapTree()
    {
        delete[] iTreeValues;
    }

    //=========================================================
    /**
    If intersection is found within pMaxDist, sets pMaxDist to intersection distance and returns true.
    Else, pMaxDist is not modified and returns false;
    */

    bool StaticMapTree::GetIntersectionTime(const G3D::Ray& pRay, float& pMaxDist, bool StopAtFirstHit, ModelIgnoreFlags ignoreFlags) const
    {
        float distance = pMaxDist;
        MapRayCallback intersectionCallBack(iTreeValues, ignoreFlags);
        iTree.intersectRay(pRay, intersectionCallBack, distance, StopAtFirstHit);
        if (intersectionCallBack.didHit())
        {
            pMaxDist = distance;
        }
        return intersectionCallBack.didHit();
    }
    //=========================================================

    bool StaticMapTree::isInLineOfSight(const Vector3& pos1, const Vector3& pos2, ModelIgnoreFlags ignoreFlags) const
    {
        float maxDist = (pos2 - pos1).magnitude();
        // return false if distance is over max float, in case of cheater teleporting to the end of the universe
        if (maxDist == std::numeric_limits<float>::max() || !std::isfinite(maxDist))
        {
            return false;
        }

        // valid map coords should *never ever* produce float overflow, but this would produce NaNs too
        ASSERT(maxDist < std::numeric_limits<float>::max());
        // prevent NaN values which can cause BIH intersection to enter infinite loop
        if (maxDist < 1e-10f)
        {
            return true;
        }
        // direction with length of 1
        G3D::Ray ray = G3D::Ray::fromOriginAndDirection(pos1, (pos2 - pos1) / maxDist);

        return !GetIntersectionTime(ray, maxDist, true, ignoreFlags);
    }
    //=========================================================
    /**
    When moving from pos1 to pos2 check if we hit an object. Return true and the position if we hit one
    Return the hit pos or the original dest pos
    */

    bool StaticMapTree::GetObjectHitPos(const Vector3& pPos1, const Vector3& pPos2, Vector3& pResultHitPos, float pModifyDist) const
    {
        bool result = false;
        float maxDist = (pPos2 - pPos1).magnitude();
        // valid map coords should *never ever* produce float overflow, but this would produce NaNs too
        ASSERT(maxDist < std::numeric_limits<float>::max());
        // prevent NaN values which can cause BIH intersection to enter infinite loop
        if (maxDist < 1e-10f)
        {
            pResultHitPos = pPos2;
            return false;
        }
        Vector3 dir = (pPos2 - pPos1) / maxDist;            // direction with length of 1
        G3D::Ray ray(pPos1, dir);
        float dist = maxDist;
        if (GetIntersectionTime(ray, dist, false, ModelIgnoreFlags::Nothing))
        {
            pResultHitPos = pPos1 + dir * dist;
            if (pModifyDist < 0)
            {
                if ((pResultHitPos - pPos1).magnitude() > -pModifyDist)
                {
                    pResultHitPos = pResultHitPos + dir * pModifyDist;
                }
                else
                {
                    pResultHitPos = pPos1;
                }
            }
            else
            {
                pResultHitPos = pResultHitPos + dir * pModifyDist;
            }
            result = true;
        }
        else
        {
            pResultHitPos = pPos2;
            result = false;
        }
        return result;
    }

    //=========================================================

    float StaticMapTree::getHeight(const Vector3& pPos, float maxSearchDist) const
    {
        float height = G3D::finf();
        Vector3 dir = Vector3(0, 0, -1);
        G3D::Ray ray(pPos, dir);   // direction with length of 1
        float maxDist = maxSearchDist;
        if (GetIntersectionTime(ray, maxDist, false, ModelIgnoreFlags::Nothing))
        {
            height = pPos.z - maxDist;
        }
        return (height);
    }

    //=========================================================

    LoadResult StaticMapTree::CanLoadMap(const std::string& vmapPath, uint32 mapID, uint32 tileX, uint32 tileY)
    {
        std::string basePath = vmapPath;
        if (basePath.length() > 0 && basePath[basePath.length() - 1] != '/' && basePath[basePath.length() - 1] != '\\')
        {
            basePath.push_back('/');
        }
        std::string fullname = basePath + VMapMgr2::getMapFileName(mapID);

        LoadResult result = LoadResult::Success;

        FILE* rf = fopen(fullname.c_str(), "rb");
        if (!rf)
        {
            return LoadResult::FileNotFound;
        }

        char tiled;
        char chunk[8];
        if (!readChunk(rf, chunk, VMAP_MAGIC, 8) || fread(&tiled, sizeof(char), 1, rf) != 1)
        {
            fclose(rf);
            return LoadResult::VersionMismatch;
        }
        if (tiled)
        {
            std::string tilefile = basePath + getTileFileName(mapID, tileX, tileY);
            FILE* tf = fopen(tilefile.c_str(), "rb");
            if (!tf)
            {
                result = LoadResult::FileNotFound;
            }
            else
            {
                if (!readChunk(tf, chunk, VMAP_MAGIC, 8))
                {
                    result = LoadResult::VersionMismatch;
                }
                fclose(tf);
            }
        }
        fclose(rf);
        return result;
    }

    //=========================================================

    bool StaticMapTree::InitMap(const std::string& fname, VMapMgr2* vm)
    {
        //VMAP_DEBUG_LOG(LOG_FILTER_MAPS, "StaticMapTree::InitMap() : initializing StaticMapTree '{}'", fname);
        bool success = false;
        std::string fullname = iBasePath + fname;
        FILE* rf = fopen(fullname.c_str(), "rb");
        if (!rf)
        {
            return false;
        }

        char chunk[8];
        char tiled = '\0';

        if (readChunk(rf, chunk, VMAP_MAGIC, 8) && fread(&tiled, sizeof(char), 1, rf) == 1 &&
                readChunk(rf, chunk, "NODE", 4) && iTree.readFromFile(rf))
        {
            iNTreeValues = iTree.primCount();
            iTreeValues = new ModelInstance[iNTreeValues];
            success = readChunk(rf, chunk, "GOBJ", 4);
        }

        iIsTiled = bool(tiled);

        // global model spawns
        // only non-tiled maps have them, and if so exactly one (so far at least...)
        ModelSpawn spawn;
#ifdef VMAP_DEBUG
        //LOG_DEBUG(LOG_FILTER_MAPS, "StaticMapTree::InitMap() : map isTiled: {}", static_cast<uint32>(iIsTiled));
#endif
        if (!iIsTiled && ModelSpawn::readFromFile(rf, spawn))
        {
            WorldModel* model = vm->acquireModelInstance(iBasePath, spawn.name, spawn.flags);
            //VMAP_DEBUG_LOG(LOG_FILTER_MAPS, "StaticMapTree::InitMap() : loading {}", spawn.name);
            if (model)
            {
                // assume that global model always is the first and only tree value (could be improved...)
                iTreeValues[0] = ModelInstance(spawn, model);
                iLoadedSpawns[0] = 1;
            }
            else
            {
                success = false;
                //VMAP_ERROR_LOG(LOG_FILTER_GENERAL, "StaticMapTree::InitMap() : could not acquire WorldModel pointer for '{}'", spawn.name);
            }
        }

        fclose(rf);
        return success;
    }

    //=========================================================

    void StaticMapTree::UnloadMap(VMapMgr2* vm)
    {
        for (loadedSpawnMap::iterator i = iLoadedSpawns.begin(); i != iLoadedSpawns.end(); ++i)
        {
            iTreeValues[i->first].setUnloaded();
            for (uint32 refCount = 0; refCount < i->second; ++refCount)
            {
                vm->releaseModelInstance(iTreeValues[i->first].name);
            }
        }
        iLoadedSpawns.clear();
        iLoadedTiles.clear();
    }

    //=========================================================

    bool StaticMapTree::LoadMapTile(uint32 tileX, uint32 tileY, VMapMgr2* vm)
    {
        if (!iIsTiled)
        {
            // currently, core creates grids for all maps, whether it has terrain tiles or not
            // so we need "fake" tile loads to know when we can unload map geometry
            iLoadedTiles[packTileID(tileX, tileY)] = false;
            return true;
        }
        if (!iTreeValues)
        {
            LOG_ERROR("maps", "StaticMapTree::LoadMapTile() : tree has not been initialized [{}, {}]", tileX, tileY);
            return false;
        }
        bool result = true;

        std::string tilefile = iBasePath + getTileFileName(iMapID, tileX, tileY);
        FILE* tf = fopen(tilefile.c_str(), "rb");
        if (tf)
        {
            char chunk[8];

            if (!readChunk(tf, chunk, VMAP_MAGIC, 8))
            {
                result = false;
            }
            uint32 numSpawns = 0;
            if (result && fread(&numSpawns, sizeof(uint32), 1, tf) != 1)
            {
                result = false;
            }
            for (uint32 i = 0; i < numSpawns && result; ++i)
            {
                // read model spawns
                ModelSpawn spawn;
                result = ModelSpawn::readFromFile(tf, spawn);
                if (result)
                {
                    // acquire model instance
                    WorldModel* model = vm->acquireModelInstance(iBasePath, spawn.name, spawn.flags);
                    if (!model)
                    {
                        LOG_ERROR("maps", "StaticMapTree::LoadMapTile() : could not acquire WorldModel pointer [{}, {}]", tileX, tileY);
                    }

                    // update tree
                    uint32 referencedVal;

                    if (fread(&referencedVal, sizeof(uint32), 1, tf) == 1)
                    {
                        if (!iLoadedSpawns.count(referencedVal))
                        {
#if defined(VMAP_DEBUG)
                            if (referencedVal > iNTreeValues)
                            {
                                LOG_DEBUG("maps", "StaticMapTree::LoadMapTile() : invalid tree element ({}/{})", referencedVal, iNTreeValues);
                                continue;
                            }
#endif
                            // Debugging: Add logging
                            LOG_DEBUG("maps", "Pre-Assignment: Checking Validity");

                            // Debugging: Check if 'model' is valid
                            if (!model)
                            {
                                LOG_ERROR("maps", "StaticMapTree::LoadMapTile() : 'model' is null");
                                continue;  // Skip this iteration
                            }

                            iTreeValues[referencedVal] = ModelInstance(spawn, model);
                            iLoadedSpawns[referencedVal] = 1;

                            // Debugging: Add logging
                            LOG_DEBUG("maps", "Post-Assignment: Successfully assigned");
                            }
                        else
                        {
                            ++iLoadedSpawns[referencedVal];
#if defined(VMAP_DEBUG)
                            if (iTreeValues[referencedVal].ID != spawn.ID)
                            {
                                LOG_DEBUG("maps", "StaticMapTree::LoadMapTile() : trying to load wrong spawn in node");
                            }
                            else if (iTreeValues[referencedVal].name != spawn.name)
                            {
                                LOG_DEBUG("maps", "StaticMapTree::LoadMapTile() : name collision on GUID={}", spawn.ID);
                            }
#endif
                        }
                    }
                    else
                    {
                        result = false;
                    }
                }
            }
            iLoadedTiles[packTileID(tileX, tileY)] = true;
            fclose(tf);
        }
        else
        {
            iLoadedTiles[packTileID(tileX, tileY)] = false;
        }

        METRIC_EVENT("map_events", "LoadMapTile",
            "Map: " + std::to_string(iMapID) + " TileX: " + std::to_string(tileX) + " TileY: " + std::to_string(tileY));

        return result;
    }

    //=========================================================

    void StaticMapTree::UnloadMapTile(uint32 tileX, uint32 tileY, VMapMgr2* vm)
    {
        uint32 tileID = packTileID(tileX, tileY);
        loadedTileMap::iterator tile = iLoadedTiles.find(tileID);
        if (tile == iLoadedTiles.end())
        {
            LOG_ERROR("maps", "StaticMapTree::UnloadMapTile() : trying to unload non-loaded tile - Map:{} X:{} Y:{}", iMapID, tileX, tileY);
            return;
        }
        if (tile->second) // file associated with tile
        {
            std::string tilefile = iBasePath + getTileFileName(iMapID, tileX, tileY);
            FILE* tf = fopen(tilefile.c_str(), "rb");
            if (tf)
            {
                bool result = true;
                char chunk[8];
                if (!readChunk(tf, chunk, VMAP_MAGIC, 8))
                {
                    result = false;
                }
                uint32 numSpawns;
                if (fread(&numSpawns, sizeof(uint32), 1, tf) != 1)
                {
                    result = false;
                }
                for (uint32 i = 0; i < numSpawns && result; ++i)
                {
                    // read model spawns
                    ModelSpawn spawn;
                    result = ModelSpawn::readFromFile(tf, spawn);
                    if (result)
                    {
                        // release model instance
                        vm->releaseModelInstance(spawn.name);

                        // update tree
                        uint32 referencedNode;

                        if (fread(&referencedNode, sizeof(uint32), 1, tf) != 1)
                        {
                            result = false;
                        }
                        else
                        {
                            if (!iLoadedSpawns.count(referencedNode))
                            {
                                LOG_ERROR("maps", "StaticMapTree::UnloadMapTile() : trying to unload non-referenced model '{}' (ID:{})", spawn.name, spawn.ID);
                            }
                            else if (--iLoadedSpawns[referencedNode] == 0)
                            {
                                iTreeValues[referencedNode].setUnloaded();
                                iLoadedSpawns.erase(referencedNode);
                            }
                        }
                    }
                }
                fclose(tf);
            }
        }
        iLoadedTiles.erase(tile);

        METRIC_EVENT("map_events", "UnloadMapTile",
            "Map: " + std::to_string(iMapID) + " TileX: " + std::to_string(tileX) + " TileY: " + std::to_string(tileY));
    }

    void StaticMapTree::GetModelInstances(ModelInstance*& models, uint32& count)
    {
        models = iTreeValues;
        count = iNTreeValues;
    }
}
