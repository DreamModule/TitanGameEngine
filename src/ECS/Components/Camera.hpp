#pragma once

#include “Transform.hpp”

namespace Titan {
namespace ECS {
namespace Components {

enum class ProjectionType {
Perspective,
Orthographic
};

struct Camera {
ProjectionType projectionType = ProjectionType::Perspective;

```
float fieldOfView = 60.0f;
float nearClip = 0.1f;
float farClip = 1000.0f;

float orthographicSize = 10.0f;

float aspectRatio = 16.0f / 9.0f;

bool isPrimary = false;

int renderOrder = 0;

Camera() = default;

Camera(ProjectionType type, float fov = 60.0f) 
    : projectionType(type), fieldOfView(fov) {}

void SetPerspective(float fov, float aspect, float nearClip, float farClip) {
    projectionType = ProjectionType::Perspective;
    fieldOfView = fov;
    aspectRatio = aspect;
    this->nearClip = nearClip;
    this->farClip = farClip;
}

void SetOrthographic(float size, float aspect, float nearClip, float farClip) {
    projectionType = ProjectionType::Orthographic;
    orthographicSize = size;
    aspectRatio = aspect;
    this->nearClip = nearClip;
    this->farClip = farClip;
}

static Camera CreatePerspective(float fov = 60.0f, float aspect = 16.0f / 9.0f) {
    Camera cam;
    cam.SetPerspective(fov, aspect, 0.1f, 1000.0f);
    return cam;
}

static Camera CreateOrthographic(float size = 10.0f, float aspect = 16.0f / 9.0f) {
    Camera cam;
    cam.SetOrthographic(size, aspect, -100.0f, 100.0f);
    return cam;
}
```

};

}
}
}
