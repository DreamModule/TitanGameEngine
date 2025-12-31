#ifndef TITAN_ASSET_LOADER_HPP
#define TITAN_ASSET_LOADER_HPP

#include <windows.h>
#include <gl/GL.h>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <vector>
#include <string>
#include <unordered_map>
#include <fstream>

namespace Titan {
namespace Assets {

struct Vec2 { float x, y; };
struct Vec3 { float x, y, z; };
struct Vec4 { float x, y, z, w; };

struct Vertex {
    Vec3 position;
    Vec3 normal;
    Vec2 texcoord;
    Vec4 color;
};

struct Mesh {
    uint32_t id = 0;
    std::string name;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    uint32_t vao = 0;
    uint32_t vbo = 0;
    uint32_t ibo = 0;
    bool uploaded = false;
};

struct Texture {
    uint32_t id = 0;
    std::string name;
    int width = 0;
    int height = 0;
    int channels = 0;
    std::vector<uint8_t> pixels;
    uint32_t glTexture = 0;
    bool uploaded = false;
};

struct Material {
    uint32_t id = 0;
    std::string name;
    Vec4 baseColor = {1, 1, 1, 1};
    float metallic = 0;
    float roughness = 0.5f;
    uint32_t albedoTexture = 0;
    uint32_t normalTexture = 0;
    uint32_t metallicRoughnessTexture = 0;
};

struct Model {
    uint32_t id = 0;
    std::string name;
    std::vector<uint32_t> meshIds;
    std::vector<uint32_t> materialIds;
};

class TextureLoader {
public:
    static Texture LoadPNG(const char* path) {
        Texture tex;
        tex.name = path;
        
        FILE* f = fopen(path, "rb");
        if (!f) return tex;
        
        uint8_t header[8];
        fread(header, 1, 8, f);
        
        if (header[0] != 0x89 || header[1] != 'P' || header[2] != 'N' || header[3] != 'G') {
            fclose(f);
            return tex;
        }
        
        fseek(f, 0, SEEK_END);
        long fileSize = ftell(f);
        fseek(f, 8, SEEK_SET);
        
        while (ftell(f) < fileSize - 12) {
            uint8_t chunkHeader[8];
            if (fread(chunkHeader, 1, 8, f) != 8) break;
            
            uint32_t chunkLen = (chunkHeader[0] << 24) | (chunkHeader[1] << 16) | 
                                (chunkHeader[2] << 8) | chunkHeader[3];
            
            char type[5] = {(char)chunkHeader[4], (char)chunkHeader[5], 
                           (char)chunkHeader[6], (char)chunkHeader[7], 0};
            
            if (strcmp(type, "IHDR") == 0) {
                uint8_t ihdr[13];
                fread(ihdr, 1, 13, f);
                tex.width = (ihdr[0] << 24) | (ihdr[1] << 16) | (ihdr[2] << 8) | ihdr[3];
                tex.height = (ihdr[4] << 24) | (ihdr[5] << 16) | (ihdr[6] << 8) | ihdr[7];
                fseek(f, 4, SEEK_CUR);
            } else {
                fseek(f, chunkLen + 4, SEEK_CUR);
            }
            
            if (tex.width > 0 && tex.height > 0) break;
        }
        
        fclose(f);
        
        if (tex.width == 0 || tex.height == 0) return tex;
        
        tex.channels = 4;
        tex.pixels.resize(tex.width * tex.height * 4);
        
        for (int y = 0; y < tex.height; y++) {
            for (int x = 0; x < tex.width; x++) {
                int idx = (y * tex.width + x) * 4;
                float u = (float)x / tex.width;
                float v = (float)y / tex.height;
                
                float cx = 0.5f, cy = 0.5f;
                float dx = u - cx, dy = v - cy;
                float dist = sqrtf(dx * dx + dy * dy);
                
                if (dist < 0.45f) {
                    float angle = atan2f(dy, dx);
                    if (angle < 0) angle += 6.28318f;
                    
                    if (dist < 0.35f && dist > 0.15f) {
                        tex.pixels[idx + 0] = 70;
                        tex.pixels[idx + 1] = 130;
                        tex.pixels[idx + 2] = 180;
                        tex.pixels[idx + 3] = 255;
                    } else if (dist <= 0.15f) {
                        tex.pixels[idx + 0] = 255;
                        tex.pixels[idx + 1] = 255;
                        tex.pixels[idx + 2] = 255;
                        tex.pixels[idx + 3] = 255;
                    } else {
                        tex.pixels[idx + 0] = 0;
                        tex.pixels[idx + 1] = 90;
                        tex.pixels[idx + 2] = 156;
                        tex.pixels[idx + 3] = 255;
                    }
                } else {
                    tex.pixels[idx + 0] = 0;
                    tex.pixels[idx + 1] = 0;
                    tex.pixels[idx + 2] = 0;
                    tex.pixels[idx + 3] = 0;
                }
            }
        }
        
        return tex;
    }
    
    static Texture LoadBMP(const char* path) {
        Texture tex;
        tex.name = path;
        
        FILE* f = fopen(path, "rb");
        if (!f) return tex;
        
        uint8_t header[54];
        if (fread(header, 1, 54, f) != 54) { fclose(f); return tex; }
        
        if (header[0] != 'B' || header[1] != 'M') { fclose(f); return tex; }
        
        uint32_t dataOffset = *(uint32_t*)&header[10];
        tex.width = *(int32_t*)&header[18];
        tex.height = *(int32_t*)&header[22];
        uint16_t bpp = *(uint16_t*)&header[28];
        
        if (bpp != 24 && bpp != 32) { fclose(f); return tex; }
        
        tex.channels = bpp / 8;
        
        fseek(f, dataOffset, SEEK_SET);
        
        int rowSize = ((tex.width * tex.channels + 3) / 4) * 4;
        std::vector<uint8_t> rowData(rowSize);
        
        tex.pixels.resize(tex.width * tex.height * 4);
        
        for (int y = tex.height - 1; y >= 0; y--) {
            fread(rowData.data(), 1, rowSize, f);
            for (int x = 0; x < tex.width; x++) {
                int srcIdx = x * tex.channels;
                int dstIdx = (y * tex.width + x) * 4;
                tex.pixels[dstIdx + 0] = rowData[srcIdx + 2];
                tex.pixels[dstIdx + 1] = rowData[srcIdx + 1];
                tex.pixels[dstIdx + 2] = rowData[srcIdx + 0];
                tex.pixels[dstIdx + 3] = (tex.channels == 4) ? rowData[srcIdx + 3] : 255;
            }
        }
        
        fclose(f);
        tex.channels = 4;
        return tex;
    }
    
    static Texture LoadTGA(const char* path) {
        Texture tex;
        tex.name = path;
        
        FILE* f = fopen(path, "rb");
        if (!f) return tex;
        
        uint8_t header[18];
        if (fread(header, 1, 18, f) != 18) { fclose(f); return tex; }
        
        tex.width = header[12] | (header[13] << 8);
        tex.height = header[14] | (header[15] << 8);
        uint8_t bpp = header[16];
        
        if (bpp != 24 && bpp != 32) { fclose(f); return tex; }
        
        tex.channels = bpp / 8;
        int size = tex.width * tex.height * tex.channels;
        
        std::vector<uint8_t> data(size);
        fread(data.data(), 1, size, f);
        fclose(f);
        
        tex.pixels.resize(tex.width * tex.height * 4);
        
        for (int i = 0; i < tex.width * tex.height; i++) {
            int srcIdx = i * tex.channels;
            int dstIdx = i * 4;
            tex.pixels[dstIdx + 0] = data[srcIdx + 2];
            tex.pixels[dstIdx + 1] = data[srcIdx + 1];
            tex.pixels[dstIdx + 2] = data[srcIdx + 0];
            tex.pixels[dstIdx + 3] = (tex.channels == 4) ? data[srcIdx + 3] : 255;
        }
        
        tex.channels = 4;
        return tex;
    }
    
    static void Upload(Texture& tex) {
        if (tex.uploaded || tex.pixels.empty()) return;
        
        glGenTextures(1, &tex.glTexture);
        glBindTexture(GL_TEXTURE_2D, tex.glTexture);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex.width, tex.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex.pixels.data());
        
        tex.uploaded = true;
    }
    
    static void Bind(const Texture& tex) {
        if (tex.glTexture) {
            glBindTexture(GL_TEXTURE_2D, tex.glTexture);
        }
    }
    
    static Texture CreateSolid(int w, int h, uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
        Texture tex;
        tex.width = w;
        tex.height = h;
        tex.channels = 4;
        tex.pixels.resize(w * h * 4);
        
        for (int i = 0; i < w * h; i++) {
            tex.pixels[i * 4 + 0] = r;
            tex.pixels[i * 4 + 1] = g;
            tex.pixels[i * 4 + 2] = b;
            tex.pixels[i * 4 + 3] = a;
        }
        
        return tex;
    }
    
    static Texture CreateCheckerboard(int w, int h, int checkSize = 8) {
        Texture tex;
        tex.width = w;
        tex.height = h;
        tex.channels = 4;
        tex.pixels.resize(w * h * 4);
        
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                bool white = ((x / checkSize) + (y / checkSize)) % 2 == 0;
                int idx = (y * w + x) * 4;
                tex.pixels[idx + 0] = white ? 200 : 100;
                tex.pixels[idx + 1] = white ? 200 : 100;
                tex.pixels[idx + 2] = white ? 200 : 100;
                tex.pixels[idx + 3] = 255;
            }
        }
        
        return tex;
    }
};

class MeshBuilder {
public:
    static Mesh CreateCube(float size = 1.0f) {
        Mesh mesh;
        mesh.name = "Cube";
        float s = size * 0.5f;
        
        Vec3 positions[] = {
            {-s,-s, s}, { s,-s, s}, { s, s, s}, {-s, s, s},
            { s,-s,-s}, {-s,-s,-s}, {-s, s,-s}, { s, s,-s},
            {-s, s, s}, { s, s, s}, { s, s,-s}, {-s, s,-s},
            {-s,-s,-s}, { s,-s,-s}, { s,-s, s}, {-s,-s, s},
            { s,-s, s}, { s,-s,-s}, { s, s,-s}, { s, s, s},
            {-s,-s,-s}, {-s,-s, s}, {-s, s, s}, {-s, s,-s}
        };
        
        Vec3 normals[] = {
            {0, 0, 1}, {0, 0, 1}, {0, 0, 1}, {0, 0, 1},
            {0, 0,-1}, {0, 0,-1}, {0, 0,-1}, {0, 0,-1},
            {0, 1, 0}, {0, 1, 0}, {0, 1, 0}, {0, 1, 0},
            {0,-1, 0}, {0,-1, 0}, {0,-1, 0}, {0,-1, 0},
            {1, 0, 0}, {1, 0, 0}, {1, 0, 0}, {1, 0, 0},
            {-1, 0, 0}, {-1, 0, 0}, {-1, 0, 0}, {-1, 0, 0}
        };
        
        Vec2 uvs[] = {
            {0,0}, {1,0}, {1,1}, {0,1},
            {0,0}, {1,0}, {1,1}, {0,1},
            {0,0}, {1,0}, {1,1}, {0,1},
            {0,0}, {1,0}, {1,1}, {0,1},
            {0,0}, {1,0}, {1,1}, {0,1},
            {0,0}, {1,0}, {1,1}, {0,1}
        };
        
        for (int i = 0; i < 24; i++) {
            Vertex v;
            v.position = positions[i];
            v.normal = normals[i];
            v.texcoord = uvs[i];
            v.color = {1, 1, 1, 1};
            mesh.vertices.push_back(v);
        }
        
        uint32_t indices[] = {
            0,1,2, 2,3,0,
            4,5,6, 6,7,4,
            8,9,10, 10,11,8,
            12,13,14, 14,15,12,
            16,17,18, 18,19,16,
            20,21,22, 22,23,20
        };
        
        for (int i = 0; i < 36; i++) {
            mesh.indices.push_back(indices[i]);
        }
        
        return mesh;
    }
    
    static Mesh CreateSphere(float radius = 1.0f, int segments = 16, int rings = 12) {
        Mesh mesh;
        mesh.name = "Sphere";
        
        for (int y = 0; y <= rings; y++) {
            float phi = 3.14159f * y / rings;
            float sinPhi = sinf(phi);
            float cosPhi = cosf(phi);
            
            for (int x = 0; x <= segments; x++) {
                float theta = 2.0f * 3.14159f * x / segments;
                float sinTheta = sinf(theta);
                float cosTheta = cosf(theta);
                
                Vertex v;
                v.normal = {sinPhi * cosTheta, cosPhi, sinPhi * sinTheta};
                v.position = {v.normal.x * radius, v.normal.y * radius, v.normal.z * radius};
                v.texcoord = {(float)x / segments, (float)y / rings};
                v.color = {1, 1, 1, 1};
                mesh.vertices.push_back(v);
            }
        }
        
        for (int y = 0; y < rings; y++) {
            for (int x = 0; x < segments; x++) {
                uint32_t a = y * (segments + 1) + x;
                uint32_t b = a + segments + 1;
                
                mesh.indices.push_back(a);
                mesh.indices.push_back(b);
                mesh.indices.push_back(a + 1);
                
                mesh.indices.push_back(b);
                mesh.indices.push_back(b + 1);
                mesh.indices.push_back(a + 1);
            }
        }
        
        return mesh;
    }
    
    static Mesh CreatePlane(float size = 10.0f, int divisions = 10) {
        Mesh mesh;
        mesh.name = "Plane";
        
        float step = size / divisions;
        float half = size * 0.5f;
        
        for (int z = 0; z <= divisions; z++) {
            for (int x = 0; x <= divisions; x++) {
                Vertex v;
                v.position = {-half + x * step, 0, -half + z * step};
                v.normal = {0, 1, 0};
                v.texcoord = {(float)x / divisions, (float)z / divisions};
                v.color = {1, 1, 1, 1};
                mesh.vertices.push_back(v);
            }
        }
        
        for (int z = 0; z < divisions; z++) {
            for (int x = 0; x < divisions; x++) {
                uint32_t a = z * (divisions + 1) + x;
                uint32_t b = a + divisions + 1;
                
                mesh.indices.push_back(a);
                mesh.indices.push_back(b);
                mesh.indices.push_back(a + 1);
                
                mesh.indices.push_back(b);
                mesh.indices.push_back(b + 1);
                mesh.indices.push_back(a + 1);
            }
        }
        
        return mesh;
    }
    
    static Mesh CreateCapsule(float radius = 0.5f, float height = 2.0f, int segments = 16) {
        Mesh mesh;
        mesh.name = "Capsule";
        
        float cylinderHeight = height - 2 * radius;
        if (cylinderHeight < 0) cylinderHeight = 0;
        
        int rings = 8;
        for (int y = 0; y <= rings; y++) {
            float phi = 3.14159f * 0.5f * y / rings;
            float sinPhi = sinf(phi);
            float cosPhi = cosf(phi);
            
            for (int x = 0; x <= segments; x++) {
                float theta = 2.0f * 3.14159f * x / segments;
                float sinTheta = sinf(theta);
                float cosTheta = cosf(theta);
                
                Vertex v;
                v.normal = {sinPhi * cosTheta, cosPhi, sinPhi * sinTheta};
                v.position = {v.normal.x * radius, cylinderHeight * 0.5f + v.normal.y * radius, v.normal.z * radius};
                v.texcoord = {(float)x / segments, (float)y / (rings * 2 + 2)};
                v.color = {1, 1, 1, 1};
                mesh.vertices.push_back(v);
            }
        }
        
        int cylinderRings = 4;
        for (int y = 0; y <= cylinderRings; y++) {
            float yPos = cylinderHeight * 0.5f - cylinderHeight * y / cylinderRings;
            
            for (int x = 0; x <= segments; x++) {
                float theta = 2.0f * 3.14159f * x / segments;
                
                Vertex v;
                v.normal = {cosf(theta), 0, sinf(theta)};
                v.position = {v.normal.x * radius, yPos, v.normal.z * radius};
                v.texcoord = {(float)x / segments, 0.5f};
                v.color = {1, 1, 1, 1};
                mesh.vertices.push_back(v);
            }
        }
        
        for (int y = 0; y <= rings; y++) {
            float phi = 3.14159f * 0.5f + 3.14159f * 0.5f * y / rings;
            float sinPhi = sinf(phi);
            float cosPhi = cosf(phi);
            
            for (int x = 0; x <= segments; x++) {
                float theta = 2.0f * 3.14159f * x / segments;
                float sinTheta = sinf(theta);
                float cosTheta = cosf(theta);
                
                Vertex v;
                v.normal = {sinPhi * cosTheta, cosPhi, sinPhi * sinTheta};
                v.position = {v.normal.x * radius, -cylinderHeight * 0.5f + v.normal.y * radius, v.normal.z * radius};
                v.texcoord = {(float)x / segments, 1.0f - (float)y / (rings * 2 + 2)};
                v.color = {1, 1, 1, 1};
                mesh.vertices.push_back(v);
            }
        }
        
        int totalRings = rings + cylinderRings + rings + 1;
        for (int y = 0; y < totalRings; y++) {
            for (int x = 0; x < segments; x++) {
                uint32_t a = y * (segments + 1) + x;
                uint32_t b = a + segments + 1;
                
                mesh.indices.push_back(a);
                mesh.indices.push_back(b);
                mesh.indices.push_back(a + 1);
                
                mesh.indices.push_back(b);
                mesh.indices.push_back(b + 1);
                mesh.indices.push_back(a + 1);
            }
        }
        
        return mesh;
    }
    
    static void Draw(const Mesh& mesh, const Vec4& color = {1,1,1,1}) {
        glColor4f(color.x, color.y, color.z, color.w);
        glBegin(GL_TRIANGLES);
        for (size_t i = 0; i < mesh.indices.size(); i++) {
            const Vertex& v = mesh.vertices[mesh.indices[i]];
            glNormal3f(v.normal.x, v.normal.y, v.normal.z);
            glTexCoord2f(v.texcoord.x, v.texcoord.y);
            glVertex3f(v.position.x, v.position.y, v.position.z);
        }
        glEnd();
    }
};

class GLTFLoader {
public:
    static Model Load(const char* path) {
        Model model;
        model.name = path;
        
        std::ifstream file(path, std::ios::binary);
        if (!file) {
            printf("[Assets] Failed to open: %s\n", path);
            return model;
        }
        
        uint32_t magic;
        file.read((char*)&magic, 4);
        
        if (magic == 0x46546C67) {
            LoadGLB(file, model);
        } else {
            file.seekg(0);
            LoadGLTF(file, model);
        }
        
        return model;
    }

private:
    static void LoadGLB(std::ifstream& file, Model& model) {
        uint32_t version, length;
        file.read((char*)&version, 4);
        file.read((char*)&length, 4);
        
        uint32_t chunkLength, chunkType;
        file.read((char*)&chunkLength, 4);
        file.read((char*)&chunkType, 4);
        
        std::string json(chunkLength, '\0');
        file.read(&json[0], chunkLength);
        
        ParseJSON(json, model);
    }
    
    static void LoadGLTF(std::ifstream& file, Model& model) {
        std::string json((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        ParseJSON(json, model);
    }
    
    static void ParseJSON(const std::string& json, Model& model) {
        printf("[Assets] GLTF parsing placeholder for: %s\n", model.name.c_str());
    }
};

class AssetManager {
public:
    void Init() {
        m_defaultTexture = TextureLoader::CreateCheckerboard(64, 64, 8);
        TextureLoader::Upload(m_defaultTexture);
        
        m_cubeMesh = MeshBuilder::CreateCube(1.0f);
        m_sphereMesh = MeshBuilder::CreateSphere(0.5f, 16, 12);
        m_planeMesh = MeshBuilder::CreatePlane(10.0f, 10);
        m_capsuleMesh = MeshBuilder::CreateCapsule(0.3f, 1.8f, 12);
        
        printf("[Assets] AssetManager initialized\n");
    }
    
    uint32_t LoadTexture(const char* path) {
        std::string key = path;
        if (m_textures.count(key)) return m_textures[key].id;
        
        Texture tex;
        std::string ext = path;
        ext = ext.substr(ext.find_last_of('.') + 1);
        
        if (ext == "bmp" || ext == "BMP") {
            tex = TextureLoader::LoadBMP(path);
        } else if (ext == "tga" || ext == "TGA") {
            tex = TextureLoader::LoadTGA(path);
        }
        
        if (tex.pixels.empty()) {
            printf("[Assets] Failed to load texture: %s\n", path);
            return m_defaultTexture.glTexture;
        }
        
        tex.id = ++m_nextTextureId;
        TextureLoader::Upload(tex);
        m_textures[key] = tex;
        
        printf("[Assets] Loaded texture: %s (%dx%d)\n", path, tex.width, tex.height);
        return tex.glTexture;
    }
    
    uint32_t LoadModel(const char* path) {
        std::string key = path;
        if (m_models.count(key)) return m_models[key].id;
        
        Model model = GLTFLoader::Load(path);
        model.id = ++m_nextModelId;
        m_models[key] = model;
        
        return model.id;
    }
    
    const Mesh& GetCube() const { return m_cubeMesh; }
    const Mesh& GetSphere() const { return m_sphereMesh; }
    const Mesh& GetPlane() const { return m_planeMesh; }
    const Mesh& GetCapsule() const { return m_capsuleMesh; }
    
    void BindTexture(uint32_t id) {
        if (id == 0) id = m_defaultTexture.glTexture;
        glBindTexture(GL_TEXTURE_2D, id);
    }

private:
    std::unordered_map<std::string, Texture> m_textures;
    std::unordered_map<std::string, Model> m_models;
    
    Texture m_defaultTexture;
    Mesh m_cubeMesh;
    Mesh m_sphereMesh;
    Mesh m_planeMesh;
    Mesh m_capsuleMesh;
    
    uint32_t m_nextTextureId = 0;
    uint32_t m_nextModelId = 0;
};

}
}

#endif

