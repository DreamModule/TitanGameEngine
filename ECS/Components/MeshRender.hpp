/**
 * Mesh Render Component
 */

#ifndef SRC_ECS_COMPONENTS_MESH_RENDER_HPP
#define SRC_ECS_COMPONENTS_MESH_RENDER_HPP

#include "../../../Titan_Graphics.hpp"

namespace Titan::ECS {

struct FMeshRenderComponent
{
    Graphics::BufferHandle VertexBuffer;
    Graphics::BufferHandle IndexBuffer;
    Graphics::ShaderHandle Shader;
    Graphics::TextureHandle Texture;
    uint32 IndexCount = 0;
    uint32 VertexCount = 0;
    bool bVisible = true;
    bool bCastShadows = true;
    int32 RenderOrder = 0;
};

using MeshRenderComponent = FMeshRenderComponent;

} // namespace Titan::ECS

#endif // SRC_ECS_COMPONENTS_MESH_RENDER_HPP
