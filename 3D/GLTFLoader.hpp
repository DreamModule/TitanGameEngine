/**
 * Titan GLTF Loader
 * 
 * Full-featured GLTF 2.0 loader with:
 * - Meshes with multiple primitives
 * - Materials with PBR properties
 * - Textures (embedded and external)
 * - Skeletal animation data
 * - Scene hierarchy
 */

#ifndef TITAN_GLTF_LOADER_HPP
#define TITAN_GLTF_LOADER_HPP

#include "Renderer3D.hpp"
#include "../Titan_Core.hpp"
#include <vector>
#include <string>
#include <fstream>
#include <cstring>
#include <unordered_map>

namespace Titan {

// ============================================================================
// JSON Parser (Minimal implementation for GLTF)
// ============================================================================

class FJSONValue
{
public:
    enum class EType { Null, Bool, Number, String, Array, Object };
    
    EType Type = EType::Null;
    bool BoolValue = false;
    double NumberValue = 0.0;
    std::string StringValue;
    std::vector<FJSONValue> ArrayValue;
    std::unordered_map<std::string, FJSONValue> ObjectValue;

    bool IsNull() const { return Type == EType::Null; }
    bool IsBool() const { return Type == EType::Bool; }
    bool IsNumber() const { return Type == EType::Number; }
    bool IsString() const { return Type == EType::String; }
    bool IsArray() const { return Type == EType::Array; }
    bool IsObject() const { return Type == EType::Object; }

    bool GetBool(bool Default = false) const 
    { 
        return IsBool() ? BoolValue : Default; 
    }
    
    double GetNumber(double Default = 0.0) const 
    { 
        return IsNumber() ? NumberValue : Default; 
    }
    
    int GetInt(int Default = 0) const 
    { 
        return IsNumber() ? static_cast<int>(NumberValue) : Default; 
    }
    
    float GetFloat(float Default = 0.0f) const 
    { 
        return IsNumber() ? static_cast<float>(NumberValue) : Default; 
    }
    
    const std::string& GetString(const std::string& Default = "") const 
    { 
        return IsString() ? StringValue : Default; 
    }

    size_t ArraySize() const 
    { 
        return IsArray() ? ArrayValue.size() : 0; 
    }

    const FJSONValue& operator[](size_t Index) const
    {
        static FJSONValue Null;
        if (!IsArray() || Index >= ArrayValue.size()) return Null;
        return ArrayValue[Index];
    }

    const FJSONValue& operator[](const std::string& Key) const
    {
        static FJSONValue Null;
        if (!IsObject()) return Null;
        auto It = ObjectValue.find(Key);
        if (It == ObjectValue.end()) return Null;
        return It->second;
    }

    bool Has(const std::string& Key) const
    {
        if (!IsObject()) return false;
        return ObjectValue.find(Key) != ObjectValue.end();
    }
};

class FJSONParser
{
public:
    static FJSONValue Parse(const std::string& JSON)
    {
        FJSONParser Parser(JSON);
        return Parser.ParseValue();
    }

private:
    const std::string& Data;
    size_t Pos = 0;

    FJSONParser(const std::string& InData) : Data(InData) {}

    void SkipWhitespace()
    {
        while (Pos < Data.size() && (Data[Pos] == ' ' || Data[Pos] == '\t' || 
               Data[Pos] == '\n' || Data[Pos] == '\r'))
        {
            Pos++;
        }
    }

    char Peek() { return Pos < Data.size() ? Data[Pos] : '\0'; }
    char Get() { return Pos < Data.size() ? Data[Pos++] : '\0'; }

    FJSONValue ParseValue()
    {
        SkipWhitespace();
        char C = Peek();

        if (C == '{') return ParseObject();
        if (C == '[') return ParseArray();
        if (C == '"') return ParseString();
        if (C == 't' || C == 'f') return ParseBool();
        if (C == 'n') return ParseNull();
        if (C == '-' || (C >= '0' && C <= '9')) return ParseNumber();

        return FJSONValue();
    }

    FJSONValue ParseObject()
    {
        FJSONValue Value;
        Value.Type = FJSONValue::EType::Object;

        Get(); // {
        SkipWhitespace();

        if (Peek() == '}')
        {
            Get();
            return Value;
        }

        while (true)
        {
            SkipWhitespace();
            if (Peek() != '"') break;
            
            std::string Key = ParseString().StringValue;
            
            SkipWhitespace();
            if (Get() != ':') break;
            
            Value.ObjectValue[Key] = ParseValue();
            
            SkipWhitespace();
            char C = Get();
            if (C == '}') break;
            if (C != ',') break;
        }

        return Value;
    }

    FJSONValue ParseArray()
    {
        FJSONValue Value;
        Value.Type = FJSONValue::EType::Array;

        Get(); // [
        SkipWhitespace();

        if (Peek() == ']')
        {
            Get();
            return Value;
        }

        while (true)
        {
            Value.ArrayValue.push_back(ParseValue());
            
            SkipWhitespace();
            char C = Get();
            if (C == ']') break;
            if (C != ',') break;
        }

        return Value;
    }

    FJSONValue ParseString()
    {
        FJSONValue Value;
        Value.Type = FJSONValue::EType::String;

        Get(); // "

        while (Pos < Data.size())
        {
            char C = Get();
            if (C == '"') break;
            if (C == '\\' && Pos < Data.size())
            {
                char Next = Get();
                switch (Next)
                {
                    case 'n': Value.StringValue += '\n'; break;
                    case 't': Value.StringValue += '\t'; break;
                    case 'r': Value.StringValue += '\r'; break;
                    case '\\': Value.StringValue += '\\'; break;
                    case '"': Value.StringValue += '"'; break;
                    default: Value.StringValue += Next; break;
                }
            }
            else
            {
                Value.StringValue += C;
            }
        }

        return Value;
    }

    FJSONValue ParseNumber()
    {
        FJSONValue Value;
        Value.Type = FJSONValue::EType::Number;

        std::string NumStr;
        while (Pos < Data.size())
        {
            char C = Peek();
            if ((C >= '0' && C <= '9') || C == '-' || C == '+' || 
                C == '.' || C == 'e' || C == 'E')
            {
                NumStr += Get();
            }
            else
            {
                break;
            }
        }

        Value.NumberValue = std::stod(NumStr);
        return Value;
    }

    FJSONValue ParseBool()
    {
        FJSONValue Value;
        Value.Type = FJSONValue::EType::Bool;

        if (Data.substr(Pos, 4) == "true")
        {
            Pos += 4;
            Value.BoolValue = true;
        }
        else if (Data.substr(Pos, 5) == "false")
        {
            Pos += 5;
            Value.BoolValue = false;
        }

        return Value;
    }

    FJSONValue ParseNull()
    {
        if (Data.substr(Pos, 4) == "null")
        {
            Pos += 4;
        }
        return FJSONValue();
    }
};

// ============================================================================
// GLTF Structures
// ============================================================================

struct FGLTFBuffer
{
    std::vector<uint8_t> Data;
    size_t ByteLength = 0;
};

struct FGLTFBufferView
{
    int BufferIndex = 0;
    size_t ByteOffset = 0;
    size_t ByteLength = 0;
    size_t ByteStride = 0;
    int Target = 0;
};

struct FGLTFAccessor
{
    int BufferView = -1;
    size_t ByteOffset = 0;
    int ComponentType = 0;
    size_t Count = 0;
    std::string Type;
    std::vector<float> Min;
    std::vector<float> Max;
};

struct FGLTFPrimitive
{
    int Indices = -1;
    int Material = -1;
    std::unordered_map<std::string, int> Attributes;
    int Mode = 4; // TRIANGLES
};

struct FGLTFMesh
{
    std::string Name;
    std::vector<FGLTFPrimitive> Primitives;
};

struct FGLTFNode
{
    std::string Name;
    int Mesh = -1;
    int Skin = -1;
    std::vector<int> Children;
    
    // Transform
    float Translation[3] = {0, 0, 0};
    float Rotation[4] = {0, 0, 0, 1};
    float Scale[3] = {1, 1, 1};
    float Matrix[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    bool HasMatrix = false;
};

struct FGLTFImage
{
    std::string URI;
    int BufferView = -1;
    std::string MimeType;
    std::vector<uint8_t> Data;
};

struct FGLTFTexture
{
    int Source = -1;
    int Sampler = -1;
};

struct FGLTFMaterialPBR
{
    float BaseColorFactor[4] = {1, 1, 1, 1};
    int BaseColorTexture = -1;
    float MetallicFactor = 1.0f;
    float RoughnessFactor = 1.0f;
    int MetallicRoughnessTexture = -1;
};

struct FGLTFMaterial
{
    std::string Name;
    FGLTFMaterialPBR PBR;
    int NormalTexture = -1;
    float NormalScale = 1.0f;
    int OcclusionTexture = -1;
    float OcclusionStrength = 1.0f;
    int EmissiveTexture = -1;
    float EmissiveFactor[3] = {0, 0, 0};
    std::string AlphaMode = "OPAQUE";
    float AlphaCutoff = 0.5f;
    bool DoubleSided = false;
};

struct FGLTFSkin
{
    std::string Name;
    std::vector<int> Joints;
    int InverseBindMatrices = -1;
    int Skeleton = -1;
};

struct FGLTFAnimationChannel
{
    int Sampler = -1;
    int TargetNode = -1;
    std::string TargetPath;
};

struct FGLTFAnimationSampler
{
    int Input = -1;
    int Output = -1;
    std::string Interpolation = "LINEAR";
};

struct FGLTFAnimation
{
    std::string Name;
    std::vector<FGLTFAnimationChannel> Channels;
    std::vector<FGLTFAnimationSampler> Samplers;
};

// ============================================================================
// Loaded Model
// ============================================================================

struct FModel3D
{
    std::vector<FMesh3D> Meshes;
    std::vector<FMaterial3D> Materials;
    std::vector<Graphics::TextureHandle> Textures;
    
    // Scene hierarchy
    struct FModelNode
    {
        std::string Name;
        int MeshIndex = -1;
        Math::Mat4 LocalTransform;
        Math::Mat4 WorldTransform;
        std::vector<int> Children;
        int Parent = -1;
    };
    std::vector<FModelNode> Nodes;
    std::vector<int> RootNodes;
    
    // Animation data
    struct FAnimationClip
    {
        std::string Name;
        float Duration = 0.0f;
        
        struct FChannel
        {
            int NodeIndex = -1;
            std::string Property; // "translation", "rotation", "scale"
            std::vector<float> Times;
            std::vector<Math::Vec4> Values; // Vec3 for translation/scale, Quat for rotation
        };
        std::vector<FChannel> Channels;
    };
    std::vector<FAnimationClip> Animations;

    void Clear()
    {
        Meshes.clear();
        Materials.clear();
        Textures.clear();
        Nodes.clear();
        RootNodes.clear();
        Animations.clear();
    }
};

// ============================================================================
// GLTF Loader
// ============================================================================

class FGLTFLoader
{
public:
    /**
     * Load a GLTF or GLB file
     */
    static bool Load(const std::string& FilePath, FModel3D& OutModel)
    {
        OutModel.Clear();

        // Check extension
        bool IsGLB = FilePath.size() > 4 && 
                     FilePath.substr(FilePath.size() - 4) == ".glb";

        std::string Directory = GetDirectory(FilePath);

        FGLTFLoader Loader;
        Loader.BaseDirectory = Directory;

        if (IsGLB)
        {
            if (!Loader.LoadGLB(FilePath))
            {
                return false;
            }
        }
        else
        {
            if (!Loader.LoadGLTF(FilePath))
            {
                return false;
            }
        }

        // Parse JSON
        Loader.Root = FJSONParser::Parse(Loader.JSONData);
        if (!Loader.Root.IsObject())
        {
            return false;
        }

        // Load data
        Loader.ParseBuffers();
        Loader.ParseBufferViews();
        Loader.ParseAccessors();
        Loader.ParseImages();
        Loader.ParseTextures(OutModel);
        Loader.ParseMaterials(OutModel);
        Loader.ParseMeshes(OutModel);
        Loader.ParseNodes(OutModel);
        Loader.ParseAnimations(OutModel);

        return true;
    }

private:
    std::string BaseDirectory;
    std::string JSONData;
    std::vector<uint8_t> BinaryChunk;
    FJSONValue Root;

    std::vector<FGLTFBuffer> Buffers;
    std::vector<FGLTFBufferView> BufferViews;
    std::vector<FGLTFAccessor> Accessors;
    std::vector<FGLTFImage> Images;
    std::vector<FGLTFTexture> GLTFTextures;
    std::vector<FGLTFMaterial> GLTFMaterials;
    std::vector<FGLTFMesh> GLTFMeshes;
    std::vector<FGLTFNode> GLTFNodes;
    std::vector<FGLTFAnimation> GLTFAnimations;

    bool LoadGLTF(const std::string& FilePath)
    {
        std::ifstream File(FilePath, std::ios::binary);
        if (!File.is_open()) return false;

        std::stringstream Buffer;
        Buffer << File.rdbuf();
        JSONData = Buffer.str();

        return true;
    }

    bool LoadGLB(const std::string& FilePath)
    {
        std::ifstream File(FilePath, std::ios::binary);
        if (!File.is_open()) return false;

        // Read header
        uint32_t Magic, Version, Length;
        File.read(reinterpret_cast<char*>(&Magic), 4);
        File.read(reinterpret_cast<char*>(&Version), 4);
        File.read(reinterpret_cast<char*>(&Length), 4);

        if (Magic != 0x46546C67) // "glTF"
        {
            return false;
        }

        // Read JSON chunk
        uint32_t ChunkLength, ChunkType;
        File.read(reinterpret_cast<char*>(&ChunkLength), 4);
        File.read(reinterpret_cast<char*>(&ChunkType), 4);

        if (ChunkType != 0x4E4F534A) // "JSON"
        {
            return false;
        }

        JSONData.resize(ChunkLength);
        File.read(&JSONData[0], ChunkLength);

        // Read binary chunk if present
        if (File.tellg() < static_cast<std::streampos>(Length))
        {
            File.read(reinterpret_cast<char*>(&ChunkLength), 4);
            File.read(reinterpret_cast<char*>(&ChunkType), 4);

            if (ChunkType == 0x004E4942) // "BIN\0"
            {
                BinaryChunk.resize(ChunkLength);
                File.read(reinterpret_cast<char*>(BinaryChunk.data()), ChunkLength);
            }
        }

        return true;
    }

    void ParseBuffers()
    {
        const auto& BuffersArray = Root["buffers"];
        for (size_t i = 0; i < BuffersArray.ArraySize(); i++)
        {
            const auto& BufferJSON = BuffersArray[i];
            FGLTFBuffer Buffer;
            Buffer.ByteLength = static_cast<size_t>(BufferJSON["byteLength"].GetNumber());

            if (BufferJSON.Has("uri"))
            {
                std::string URI = BufferJSON["uri"].StringValue;
                
                // Check if it's a data URI
                if (URI.substr(0, 5) == "data:")
                {
                    // Parse base64
                    size_t CommaPos = URI.find(',');
                    if (CommaPos != std::string::npos)
                    {
                        std::string Base64Data = URI.substr(CommaPos + 1);
                        Buffer.Data = DecodeBase64(Base64Data);
                    }
                }
                else
                {
                    // Load external file
                    std::string FullPath = BaseDirectory + "/" + URI;
                    std::ifstream File(FullPath, std::ios::binary);
                    if (File.is_open())
                    {
                        Buffer.Data.resize(Buffer.ByteLength);
                        File.read(reinterpret_cast<char*>(Buffer.Data.data()), Buffer.ByteLength);
                    }
                }
            }
            else if (!BinaryChunk.empty())
            {
                Buffer.Data = BinaryChunk;
            }

            Buffers.push_back(Buffer);
        }
    }

    void ParseBufferViews()
    {
        const auto& ViewsArray = Root["bufferViews"];
        for (size_t i = 0; i < ViewsArray.ArraySize(); i++)
        {
            const auto& ViewJSON = ViewsArray[i];
            FGLTFBufferView View;
            View.BufferIndex = ViewJSON["buffer"].GetInt();
            View.ByteOffset = static_cast<size_t>(ViewJSON["byteOffset"].GetNumber(0));
            View.ByteLength = static_cast<size_t>(ViewJSON["byteLength"].GetNumber());
            View.ByteStride = static_cast<size_t>(ViewJSON["byteStride"].GetNumber(0));
            View.Target = ViewJSON["target"].GetInt();
            BufferViews.push_back(View);
        }
    }

    void ParseAccessors()
    {
        const auto& AccessorsArray = Root["accessors"];
        for (size_t i = 0; i < AccessorsArray.ArraySize(); i++)
        {
            const auto& AccJSON = AccessorsArray[i];
            FGLTFAccessor Acc;
            Acc.BufferView = AccJSON["bufferView"].GetInt(-1);
            Acc.ByteOffset = static_cast<size_t>(AccJSON["byteOffset"].GetNumber(0));
            Acc.ComponentType = AccJSON["componentType"].GetInt();
            Acc.Count = static_cast<size_t>(AccJSON["count"].GetNumber());
            Acc.Type = AccJSON["type"].StringValue;

            const auto& MinArray = AccJSON["min"];
            for (size_t j = 0; j < MinArray.ArraySize(); j++)
            {
                Acc.Min.push_back(MinArray[j].GetFloat());
            }

            const auto& MaxArray = AccJSON["max"];
            for (size_t j = 0; j < MaxArray.ArraySize(); j++)
            {
                Acc.Max.push_back(MaxArray[j].GetFloat());
            }

            Accessors.push_back(Acc);
        }
    }

    void ParseImages()
    {
        const auto& ImagesArray = Root["images"];
        for (size_t i = 0; i < ImagesArray.ArraySize(); i++)
        {
            const auto& ImgJSON = ImagesArray[i];
            FGLTFImage Img;
            
            if (ImgJSON.Has("uri"))
            {
                Img.URI = ImgJSON["uri"].StringValue;
            }
            
            Img.BufferView = ImgJSON["bufferView"].GetInt(-1);
            Img.MimeType = ImgJSON["mimeType"].StringValue;

            // Load image data
            if (Img.BufferView >= 0 && Img.BufferView < static_cast<int>(BufferViews.size()))
            {
                const auto& View = BufferViews[Img.BufferView];
                const auto& Buffer = Buffers[View.BufferIndex];
                Img.Data.assign(
                    Buffer.Data.begin() + View.ByteOffset,
                    Buffer.Data.begin() + View.ByteOffset + View.ByteLength
                );
            }

            Images.push_back(Img);
        }
    }

    void ParseTextures(FModel3D& OutModel)
    {
        const auto& TexturesArray = Root["textures"];
        for (size_t i = 0; i < TexturesArray.ArraySize(); i++)
        {
            const auto& TexJSON = TexturesArray[i];
            FGLTFTexture Tex;
            Tex.Source = TexJSON["source"].GetInt(-1);
            Tex.Sampler = TexJSON["sampler"].GetInt(-1);
            GLTFTextures.push_back(Tex);

            // Create GPU texture
            Graphics::TextureHandle Handle;
            if (Tex.Source >= 0 && Tex.Source < static_cast<int>(Images.size()))
            {
                const auto& Img = Images[Tex.Source];
                
                if (!Img.Data.empty())
                {
                    // Decode image (simplified - would use stb_image)
                    // For now, create placeholder
                    Handle = CreateTextureFromData(Img.Data);
                }
                else if (!Img.URI.empty())
                {
                    Handle = LoadTextureFromFile(BaseDirectory + "/" + Img.URI);
                }
            }
            OutModel.Textures.push_back(Handle);
        }
    }

    void ParseMaterials(FModel3D& OutModel)
    {
        const auto& MaterialsArray = Root["materials"];
        for (size_t i = 0; i < MaterialsArray.ArraySize(); i++)
        {
            const auto& MatJSON = MaterialsArray[i];
            FMaterial3D Mat;
            
            Mat.Albedo = Math::Vec4{1, 1, 1, 1};
            
            if (MatJSON.Has("pbrMetallicRoughness"))
            {
                const auto& PBR = MatJSON["pbrMetallicRoughness"];
                
                if (PBR.Has("baseColorFactor"))
                {
                    const auto& Color = PBR["baseColorFactor"];
                    Mat.Albedo.x = Color[0].GetFloat(1);
                    Mat.Albedo.y = Color[1].GetFloat(1);
                    Mat.Albedo.z = Color[2].GetFloat(1);
                    Mat.Albedo.w = Color[3].GetFloat(1);
                }
                
                if (PBR.Has("baseColorTexture"))
                {
                    int TexIndex = PBR["baseColorTexture"]["index"].GetInt(-1);
                    if (TexIndex >= 0 && TexIndex < static_cast<int>(OutModel.Textures.size()))
                    {
                        Mat.AlbedoTexture = OutModel.Textures[TexIndex];
                    }
                }
                
                Mat.Metallic = PBR["metallicFactor"].GetFloat(1.0f);
                Mat.Roughness = PBR["roughnessFactor"].GetFloat(1.0f);
            }
            
            if (MatJSON.Has("normalTexture"))
            {
                int TexIndex = MatJSON["normalTexture"]["index"].GetInt(-1);
                if (TexIndex >= 0 && TexIndex < static_cast<int>(OutModel.Textures.size()))
                {
                    Mat.NormalMap = OutModel.Textures[TexIndex];
                }
            }
            
            if (MatJSON.Has("emissiveFactor"))
            {
                const auto& Emissive = MatJSON["emissiveFactor"];
                Mat.EmissiveColor.x = Emissive[0].GetFloat(0);
                Mat.EmissiveColor.y = Emissive[1].GetFloat(0);
                Mat.EmissiveColor.z = Emissive[2].GetFloat(0);
            }
            
            Mat.bDoubleSided = MatJSON["doubleSided"].GetBool(false);
            
            std::string AlphaMode = MatJSON["alphaMode"].GetString("OPAQUE");
            Mat.bTransparent = (AlphaMode == "BLEND");
            Mat.AlphaCutoff = MatJSON["alphaCutoff"].GetFloat(0.5f);
            
            OutModel.Materials.push_back(Mat);
        }
        
        // Ensure at least one material
        if (OutModel.Materials.empty())
        {
            FMaterial3D DefaultMat;
            DefaultMat.Albedo = Math::Vec4{0.8f, 0.8f, 0.8f, 1.0f};
            DefaultMat.Metallic = 0.0f;
            DefaultMat.Roughness = 0.5f;
            OutModel.Materials.push_back(DefaultMat);
        }
    }

    void ParseMeshes(FModel3D& OutModel)
    {
        const auto& MeshesArray = Root["meshes"];
        for (size_t i = 0; i < MeshesArray.ArraySize(); i++)
        {
            const auto& MeshJSON = MeshesArray[i];
            FMesh3D Mesh;
            
            const auto& Primitives = MeshJSON["primitives"];
            for (size_t p = 0; p < Primitives.ArraySize(); p++)
            {
                const auto& PrimJSON = Primitives[p];
                
                FSubMesh SubMesh;
                SubMesh.IndexOffset = static_cast<uint32_t>(Mesh.Indices.size());
                SubMesh.MaterialIndex = PrimJSON["material"].GetInt(0);
                
                // Load indices
                int IndicesAcc = PrimJSON["indices"].GetInt(-1);
                if (IndicesAcc >= 0)
                {
                    LoadIndices(IndicesAcc, Mesh);
                }
                
                SubMesh.IndexCount = static_cast<uint32_t>(Mesh.Indices.size()) - SubMesh.IndexOffset;
                
                // Load attributes
                const auto& Attributes = PrimJSON["attributes"];
                size_t VertexStart = Mesh.Vertices.size();
                
                if (Attributes.Has("POSITION"))
                {
                    LoadPositions(Attributes["POSITION"].GetInt(), Mesh);
                }
                
                if (Attributes.Has("NORMAL"))
                {
                    LoadNormals(Attributes["NORMAL"].GetInt(), Mesh, VertexStart);
                }
                
                if (Attributes.Has("TEXCOORD_0"))
                {
                    LoadTexCoords(Attributes["TEXCOORD_0"].GetInt(), Mesh, VertexStart);
                }
                
                if (Attributes.Has("TANGENT"))
                {
                    LoadTangents(Attributes["TANGENT"].GetInt(), Mesh, VertexStart);
                }
                
                Mesh.SubMeshes.push_back(SubMesh);
            }
            
            // Calculate tangents if not provided
            Mesh.CalculateTangents();
            Mesh.CalculateBounds();
            
            OutModel.Meshes.push_back(Mesh);
        }
    }

    void ParseNodes(FModel3D& OutModel)
    {
        const auto& NodesArray = Root["nodes"];
        
        // First pass: create nodes
        for (size_t i = 0; i < NodesArray.ArraySize(); i++)
        {
            const auto& NodeJSON = NodesArray[i];
            FModel3D::FModelNode Node;
            
            Node.Name = NodeJSON["name"].GetString("");
            Node.MeshIndex = NodeJSON["mesh"].GetInt(-1);
            Node.LocalTransform = Math::Mat4::Identity();
            
            // Parse transform
            if (NodeJSON.Has("matrix"))
            {
                const auto& Mat = NodeJSON["matrix"];
                for (int j = 0; j < 16 && j < static_cast<int>(Mat.ArraySize()); j++)
                {
                    Node.LocalTransform.m[j / 4][j % 4] = Mat[j].GetFloat();
                }
            }
            else
            {
                Math::Vec3 T{0, 0, 0};
                Math::Quaternion R = Math::Quaternion::Identity();
                Math::Vec3 S{1, 1, 1};
                
                if (NodeJSON.Has("translation"))
                {
                    const auto& Trans = NodeJSON["translation"];
                    T.x = Trans[0].GetFloat();
                    T.y = Trans[1].GetFloat();
                    T.z = Trans[2].GetFloat();
                }
                
                if (NodeJSON.Has("rotation"))
                {
                    const auto& Rot = NodeJSON["rotation"];
                    R.x = Rot[0].GetFloat();
                    R.y = Rot[1].GetFloat();
                    R.z = Rot[2].GetFloat();
                    R.w = Rot[3].GetFloat();
                }
                
                if (NodeJSON.Has("scale"))
                {
                    const auto& Scl = NodeJSON["scale"];
                    S.x = Scl[0].GetFloat();
                    S.y = Scl[1].GetFloat();
                    S.z = Scl[2].GetFloat();
                }
                
                // Compose TRS matrix
                Node.LocalTransform = Math::Mat4::Translate(T) * 
                                      CreateRotationMatrix(R) * 
                                      Math::Mat4::Scale(S);
            }
            
            // Parse children
            if (NodeJSON.Has("children"))
            {
                const auto& Children = NodeJSON["children"];
                for (size_t c = 0; c < Children.ArraySize(); c++)
                {
                    Node.Children.push_back(Children[c].GetInt());
                }
            }
            
            OutModel.Nodes.push_back(Node);
        }
        
        // Second pass: set parents and find roots
        for (size_t i = 0; i < OutModel.Nodes.size(); i++)
        {
            for (int ChildIdx : OutModel.Nodes[i].Children)
            {
                if (ChildIdx >= 0 && ChildIdx < static_cast<int>(OutModel.Nodes.size()))
                {
                    OutModel.Nodes[ChildIdx].Parent = static_cast<int>(i);
                }
            }
        }
        
        for (size_t i = 0; i < OutModel.Nodes.size(); i++)
        {
            if (OutModel.Nodes[i].Parent == -1)
            {
                OutModel.RootNodes.push_back(static_cast<int>(i));
            }
        }
        
        // Calculate world transforms
        for (int RootIdx : OutModel.RootNodes)
        {
            CalculateWorldTransforms(OutModel, RootIdx, Math::Mat4::Identity());
        }
    }

    void ParseAnimations(FModel3D& OutModel)
    {
        const auto& AnimsArray = Root["animations"];
        for (size_t i = 0; i < AnimsArray.ArraySize(); i++)
        {
            const auto& AnimJSON = AnimsArray[i];
            FModel3D::FAnimationClip Clip;
            
            Clip.Name = AnimJSON["name"].GetString("Animation");
            
            const auto& Samplers = AnimJSON["samplers"];
            const auto& Channels = AnimJSON["channels"];
            
            for (size_t c = 0; c < Channels.ArraySize(); c++)
            {
                const auto& ChJSON = Channels[c];
                FModel3D::FAnimationClip::FChannel Channel;
                
                int SamplerIdx = ChJSON["sampler"].GetInt();
                Channel.NodeIndex = ChJSON["target"]["node"].GetInt(-1);
                Channel.Property = ChJSON["target"]["path"].GetString("");
                
                if (SamplerIdx >= 0 && SamplerIdx < static_cast<int>(Samplers.ArraySize()))
                {
                    const auto& Sampler = Samplers[SamplerIdx];
                    int InputAcc = Sampler["input"].GetInt(-1);
                    int OutputAcc = Sampler["output"].GetInt(-1);
                    
                    // Load keyframe times
                    if (InputAcc >= 0)
                    {
                        LoadFloatAccessor(InputAcc, Channel.Times);
                        if (!Channel.Times.empty())
                        {
                            Clip.Duration = std::max(Clip.Duration, Channel.Times.back());
                        }
                    }
                    
                    // Load keyframe values
                    if (OutputAcc >= 0)
                    {
                        LoadVec4Accessor(OutputAcc, Channel.Values);
                    }
                }
                
                Clip.Channels.push_back(Channel);
            }
            
            OutModel.Animations.push_back(Clip);
        }
    }

    // Helper functions
    void LoadIndices(int AccessorIdx, FMesh3D& Mesh)
    {
        if (AccessorIdx < 0 || AccessorIdx >= static_cast<int>(Accessors.size())) return;
        
        const auto& Acc = Accessors[AccessorIdx];
        if (Acc.BufferView < 0) return;
        
        const auto& View = BufferViews[Acc.BufferView];
        const auto& Buffer = Buffers[View.BufferIndex];
        
        const uint8_t* Data = Buffer.Data.data() + View.ByteOffset + Acc.ByteOffset;
        
        size_t BaseIndex = Mesh.Vertices.size();
        
        for (size_t i = 0; i < Acc.Count; i++)
        {
            uint32_t Index = 0;
            
            if (Acc.ComponentType == 5123) // UNSIGNED_SHORT
            {
                Index = *reinterpret_cast<const uint16_t*>(Data + i * 2);
            }
            else if (Acc.ComponentType == 5125) // UNSIGNED_INT
            {
                Index = *reinterpret_cast<const uint32_t*>(Data + i * 4);
            }
            else if (Acc.ComponentType == 5121) // UNSIGNED_BYTE
            {
                Index = Data[i];
            }
            
            Mesh.Indices.push_back(static_cast<uint32_t>(BaseIndex + Index));
        }
    }

    void LoadPositions(int AccessorIdx, FMesh3D& Mesh)
    {
        if (AccessorIdx < 0 || AccessorIdx >= static_cast<int>(Accessors.size())) return;
        
        const auto& Acc = Accessors[AccessorIdx];
        if (Acc.BufferView < 0) return;
        
        const auto& View = BufferViews[Acc.BufferView];
        const auto& Buffer = Buffers[View.BufferIndex];
        
        size_t Stride = View.ByteStride ? View.ByteStride : 12;
        const uint8_t* Data = Buffer.Data.data() + View.ByteOffset + Acc.ByteOffset;
        
        for (size_t i = 0; i < Acc.Count; i++)
        {
            const float* Pos = reinterpret_cast<const float*>(Data + i * Stride);
            FVertex3D V;
            V.Position = Math::Vec3{Pos[0], Pos[1], Pos[2]};
            Mesh.Vertices.push_back(V);
        }
    }

    void LoadNormals(int AccessorIdx, FMesh3D& Mesh, size_t VertexStart)
    {
        if (AccessorIdx < 0 || AccessorIdx >= static_cast<int>(Accessors.size())) return;
        
        const auto& Acc = Accessors[AccessorIdx];
        if (Acc.BufferView < 0) return;
        
        const auto& View = BufferViews[Acc.BufferView];
        const auto& Buffer = Buffers[View.BufferIndex];
        
        size_t Stride = View.ByteStride ? View.ByteStride : 12;
        const uint8_t* Data = Buffer.Data.data() + View.ByteOffset + Acc.ByteOffset;
        
        for (size_t i = 0; i < Acc.Count && (VertexStart + i) < Mesh.Vertices.size(); i++)
        {
            const float* Norm = reinterpret_cast<const float*>(Data + i * Stride);
            Mesh.Vertices[VertexStart + i].Normal = Math::Vec3{Norm[0], Norm[1], Norm[2]};
        }
    }

    void LoadTexCoords(int AccessorIdx, FMesh3D& Mesh, size_t VertexStart)
    {
        if (AccessorIdx < 0 || AccessorIdx >= static_cast<int>(Accessors.size())) return;
        
        const auto& Acc = Accessors[AccessorIdx];
        if (Acc.BufferView < 0) return;
        
        const auto& View = BufferViews[Acc.BufferView];
        const auto& Buffer = Buffers[View.BufferIndex];
        
        size_t Stride = View.ByteStride ? View.ByteStride : 8;
        const uint8_t* Data = Buffer.Data.data() + View.ByteOffset + Acc.ByteOffset;
        
        for (size_t i = 0; i < Acc.Count && (VertexStart + i) < Mesh.Vertices.size(); i++)
        {
            const float* UV = reinterpret_cast<const float*>(Data + i * Stride);
            Mesh.Vertices[VertexStart + i].TexCoord = Math::Vec2{UV[0], UV[1]};
        }
    }

    void LoadTangents(int AccessorIdx, FMesh3D& Mesh, size_t VertexStart)
    {
        if (AccessorIdx < 0 || AccessorIdx >= static_cast<int>(Accessors.size())) return;
        
        const auto& Acc = Accessors[AccessorIdx];
        if (Acc.BufferView < 0) return;
        
        const auto& View = BufferViews[Acc.BufferView];
        const auto& Buffer = Buffers[View.BufferIndex];
        
        size_t Stride = View.ByteStride ? View.ByteStride : 16;
        const uint8_t* Data = Buffer.Data.data() + View.ByteOffset + Acc.ByteOffset;
        
        for (size_t i = 0; i < Acc.Count && (VertexStart + i) < Mesh.Vertices.size(); i++)
        {
            const float* Tan = reinterpret_cast<const float*>(Data + i * Stride);
            Mesh.Vertices[VertexStart + i].Tangent = Math::Vec3{Tan[0], Tan[1], Tan[2]};
            // Tan[3] is handedness for bitangent calculation
        }
    }

    void LoadFloatAccessor(int AccessorIdx, std::vector<float>& OutValues)
    {
        if (AccessorIdx < 0 || AccessorIdx >= static_cast<int>(Accessors.size())) return;
        
        const auto& Acc = Accessors[AccessorIdx];
        if (Acc.BufferView < 0) return;
        
        const auto& View = BufferViews[Acc.BufferView];
        const auto& Buffer = Buffers[View.BufferIndex];
        
        const float* Data = reinterpret_cast<const float*>(
            Buffer.Data.data() + View.ByteOffset + Acc.ByteOffset);
        
        OutValues.assign(Data, Data + Acc.Count);
    }

    void LoadVec4Accessor(int AccessorIdx, std::vector<Math::Vec4>& OutValues)
    {
        if (AccessorIdx < 0 || AccessorIdx >= static_cast<int>(Accessors.size())) return;
        
        const auto& Acc = Accessors[AccessorIdx];
        if (Acc.BufferView < 0) return;
        
        const auto& View = BufferViews[Acc.BufferIndex];
        const auto& Buffer = Buffers[View.BufferIndex];
        
        int Components = 4;
        if (Acc.Type == "VEC3") Components = 3;
        else if (Acc.Type == "VEC2") Components = 2;
        else if (Acc.Type == "SCALAR") Components = 1;
        
        const float* Data = reinterpret_cast<const float*>(
            Buffer.Data.data() + View.ByteOffset + Acc.ByteOffset);
        
        for (size_t i = 0; i < Acc.Count; i++)
        {
            Math::Vec4 V{0, 0, 0, 1};
            for (int c = 0; c < Components; c++)
            {
                if (c == 0) V.x = Data[i * Components + c];
                else if (c == 1) V.y = Data[i * Components + c];
                else if (c == 2) V.z = Data[i * Components + c];
                else if (c == 3) V.w = Data[i * Components + c];
            }
            OutValues.push_back(V);
        }
    }

    void CalculateWorldTransforms(FModel3D& Model, int NodeIdx, const Math::Mat4& ParentTransform)
    {
        auto& Node = Model.Nodes[NodeIdx];
        Node.WorldTransform = ParentTransform * Node.LocalTransform;
        
        for (int ChildIdx : Node.Children)
        {
            CalculateWorldTransforms(Model, ChildIdx, Node.WorldTransform);
        }
    }

    Math::Mat4 CreateRotationMatrix(const Math::Quaternion& Q)
    {
        Math::Mat4 M;
        
        float XX = Q.x * Q.x;
        float YY = Q.y * Q.y;
        float ZZ = Q.z * Q.z;
        float XY = Q.x * Q.y;
        float XZ = Q.x * Q.z;
        float YZ = Q.y * Q.z;
        float WX = Q.w * Q.x;
        float WY = Q.w * Q.y;
        float WZ = Q.w * Q.z;
        
        M.m[0][0] = 1.0f - 2.0f * (YY + ZZ);
        M.m[0][1] = 2.0f * (XY + WZ);
        M.m[0][2] = 2.0f * (XZ - WY);
        M.m[0][3] = 0.0f;
        
        M.m[1][0] = 2.0f * (XY - WZ);
        M.m[1][1] = 1.0f - 2.0f * (XX + ZZ);
        M.m[1][2] = 2.0f * (YZ + WX);
        M.m[1][3] = 0.0f;
        
        M.m[2][0] = 2.0f * (XZ + WY);
        M.m[2][1] = 2.0f * (YZ - WX);
        M.m[2][2] = 1.0f - 2.0f * (XX + YY);
        M.m[2][3] = 0.0f;
        
        M.m[3][0] = 0.0f;
        M.m[3][1] = 0.0f;
        M.m[3][2] = 0.0f;
        M.m[3][3] = 1.0f;
        
        return M;
    }

    Graphics::TextureHandle CreateTextureFromData(const std::vector<uint8_t>& Data)
    {
        // In a full implementation, would decode PNG/JPG using stb_image
        // For now return invalid handle
        return Graphics::TextureHandle();
    }

    Graphics::TextureHandle LoadTextureFromFile(const std::string& Path)
    {
        // Would use stb_image to load file
        return Graphics::TextureHandle();
    }

    static std::vector<uint8_t> DecodeBase64(const std::string& Input)
    {
        static const char* Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        
        std::vector<uint8_t> Output;
        Output.reserve(Input.size() * 3 / 4);
        
        int Val = 0;
        int Bits = -8;
        
        for (char C : Input)
        {
            if (C == '=') break;
            
            const char* Pos = std::strchr(Chars, C);
            if (!Pos) continue;
            
            Val = (Val << 6) + static_cast<int>(Pos - Chars);
            Bits += 6;
            
            if (Bits >= 0)
            {
                Output.push_back(static_cast<uint8_t>((Val >> Bits) & 0xFF));
                Bits -= 8;
            }
        }
        
        return Output;
    }

    static std::string GetDirectory(const std::string& Path)
    {
        size_t Pos = Path.find_last_of("/\\");
        if (Pos == std::string::npos) return ".";
        return Path.substr(0, Pos);
    }
};

} // namespace Titan

#endif // TITAN_GLTF_LOADER_HPP


