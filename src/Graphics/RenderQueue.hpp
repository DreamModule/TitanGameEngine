#pragma once

#include “Material.hpp”
#include “Mesh.hpp”
#include “../ECS/Components/Transform.hpp”
#include <vector>
#include <algorithm>

namespace Titan::Graphics {

using namespace ECS::Components;

struct RenderCommand {
Mesh* mesh;
Material* material;
Mat4 modelMatrix;
uint32_t subMeshIndex;
float distanceToCamera;
uint64_t sortKey;

```
RenderCommand()
    : mesh(nullptr)
    , material(nullptr)
    , modelMatrix(Mat4::Identity())
    , subMeshIndex(0)
    , distanceToCamera(0.0f)
    , sortKey(0) {}
```

};

class RenderQueue {
public:
RenderQueue() = default;

```
void Clear() {
    commands.clear();
}

void Submit(Mesh* mesh, Material* material, const Mat4& modelMatrix, 
            uint32_t subMeshIndex = 0, float distanceToCamera = 0.0f) {
    if (!mesh || !material) return;

    RenderCommand cmd;
    cmd.mesh = mesh;
    cmd.material = material;
    cmd.modelMatrix = modelMatrix;
    cmd.subMeshIndex = subMeshIndex;
    cmd.distanceToCamera = distanceToCamera;
    cmd.sortKey = material->GetSortKey();

    commands.push_back(cmd);
}

void Sort(const Vec3& cameraPosition) {
    for (auto& cmd : commands) {
        Vec3 objectPos(cmd.modelMatrix.m[12], cmd.modelMatrix.m[13], cmd.modelMatrix.m[14]);
        Vec3 delta = objectPos - cameraPosition;
        cmd.distanceToCamera = delta.LengthSquared();
    }

    std::stable_sort(commands.begin(), commands.end(), [](const RenderCommand& a, const RenderCommand& b) {
        if (a.material->IsTransparent() != b.material->IsTransparent()) {
            return !a.material->IsTransparent();
        }

        if (a.material->IsTransparent()) {
            return a.distanceToCamera > b.distanceToCamera;
        }

        return a.sortKey < b.sortKey;
    });
}

const std::vector<RenderCommand>& GetCommands() const {
    return commands;
}

size_t GetCommandCount() const {
    return commands.size();
}
```

private:
std::vector<RenderCommand> commands;
};

class RenderBatcher {
public:
struct Batch {
Material* material;
Mesh* mesh;
uint32_t subMeshIndex;
std::vector<Mat4> modelMatrices;
uint32_t instanceCount;

```
    Batch() : material(nullptr), mesh(nullptr), subMeshIndex(0), instanceCount(0) {}
};

RenderBatcher() = default;

void Clear() {
    batches.clear();
}

void AddCommand(const RenderCommand& cmd) {
    bool foundBatch = false;

    for (auto& batch : batches) {
        if (batch.material == cmd.material && 
            batch.mesh == cmd.mesh && 
            batch.subMeshIndex == cmd.subMeshIndex) {
            batch.modelMatrices.push_back(cmd.modelMatrix);
            batch.instanceCount++;
            foundBatch = true;
            break;
        }
    }

    if (!foundBatch) {
        Batch batch;
        batch.material = cmd.material;
        batch.mesh = cmd.mesh;
        batch.subMeshIndex = cmd.subMeshIndex;
        batch.modelMatrices.push_back(cmd.modelMatrix);
        batch.instanceCount = 1;
        batches.push_back(batch);
    }
}

const std::vector<Batch>& GetBatches() const {
    return batches;
}

size_t GetBatchCount() const {
    return batches.size();
}

uint32_t GetTotalInstances() const {
    uint32_t total = 0;
    for (auto& batch : batches) {
        total += batch.instanceCount;
    }
    return total;
}
```

private:
std::vector<Batch> batches;
};

struct Frustum {
enum Plane {
Left = 0,
Right,
Bottom,
Top,
Near,
Far,
Count
};

```
struct PlaneData {
    Vec3 normal;
    float distance;

    PlaneData() : normal(Vec3::Zero()), distance(0.0f) {}

    float DistanceToPoint(const Vec3& point) const {
        return normal.Dot(point) + distance;
    }
};

PlaneData planes[Count];

void ExtractFromMatrix(const Mat4& viewProjection) {
    const float* m = viewProjection.m;

    planes[Left].normal.x = m[3] + m[0];
    planes[Left].normal.y = m[7] + m[4];
    planes[Left].normal.z = m[11] + m[8];
    planes[Left].distance = m[15] + m[12];
    NormalizePlane(Left);

    planes[Right].normal.x = m[3] - m[0];
    planes[Right].normal.y = m[7] - m[4];
    planes[Right].normal.z = m[11] - m[8];
    planes[Right].distance = m[15] - m[12];
    NormalizePlane(Right);

    planes[Bottom].normal.x = m[3] + m[1];
    planes[Bottom].normal.y = m[7] + m[5];
    planes[Bottom].normal.z = m[11] + m[9];
    planes[Bottom].distance = m[15] + m[13];
    NormalizePlane(Bottom);

    planes[Top].normal.x = m[3] - m[1];
    planes[Top].normal.y = m[7] - m[5];
    planes[Top].normal.z = m[11] - m[9];
    planes[Top].distance = m[15] - m[13];
    NormalizePlane(Top);

    planes[Near].normal.x = m[3] + m[2];
    planes[Near].normal.y = m[7] + m[6];
    planes[Near].normal.z = m[11] + m[10];
    planes[Near].distance = m[15] + m[14];
    NormalizePlane(Near);

    planes[Far].normal.x = m[3] - m[2];
    planes[Far].normal.y = m[7] - m[6];
    planes[Far].normal.z = m[11] - m[10];
    planes[Far].distance = m[15] - m[14];
    NormalizePlane(Far);
}

bool TestSphere(const Vec3& center, float radius) const {
    for (int i = 0; i < Count; ++i) {
        float distance = planes[i].DistanceToPoint(center);
        if (distance < -radius) {
            return false;
        }
    }
    return true;
}

bool TestAABB(const Vec3& min, const Vec3& max) const {
    for (int i = 0; i < Count; ++i) {
        Vec3 positive(
            planes[i].normal.x > 0 ? max.x : min.x,
            planes[i].normal.y > 0 ? max.y : min.y,
            planes[i].normal.z > 0 ? max.z : min.z
        );

        if (planes[i].DistanceToPoint(positive) < 0) {
            return false;
        }
    }
    return true;
}
```

private:
void NormalizePlane(int index) {
float length = planes[index].normal.Length();
if (length > 0.0001f) {
planes[index].normal = planes[index].normal / length;
planes[index].distance /= length;
}
}
};

}
