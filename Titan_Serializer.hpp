#ifndef TITAN_SERIALIZER_HPP
#define TITAN_SERIALIZER_HPP
#include "Titan_ECS.hpp"
#include <cstdio>
namespace Titan::Core {
    struct SceneSerializer {
        static void Save(ECS::World& w, const char* fName) {
            SnapshotStorage ss; ss.Reset();
            u32 c = (u32)w.entities.size(); ss.Write(c);
            for(ECS::EntityID e : w.entities) {
                u32 mask = 0;
                if(w.GetComponent<TransformComponent>(e, COMP_TRANSFORM)) mask |= (1<<0);
                if(w.GetComponent<RigidBodyComponent>(e, COMP_RIGIDBODY)) mask |= (1<<1);
                ss.Write(mask);
                if(mask & (1<<0)) ss.Write(*w.GetComponent<TransformComponent>(e, COMP_TRANSFORM));
                if(mask & (1<<1)) ss.Write(*w.GetComponent<RigidBodyComponent>(e, COMP_RIGIDBODY));
            }
            FILE* f = fopen(fName, "wb"); if(f){ fwrite(ss.GetData(),1,ss.GetSize(),f); fclose(f); }
        }
        static void Load(ECS::World& w, const char* fName) {
            FILE* f = fopen(fName, "rb"); if(!f) return;
            fseek(f,0,SEEK_END); long sz=ftell(f); rewind(f); SnapshotStorage ss; ss.buffer.resize(sz);
            fread(ss.buffer.data(),1,sz,f); fclose(f); ss.writePos=sz; w.Clear();
            u32 c; ss.Read(c);
            for(u32 i=0;i<c;++i) {
                ECS::EntityID e = w.CreateEntity(); u32 mask; ss.Read(mask);
                if(mask&(1<<0)) { TransformComponent t; ss.Read(t); w.AddComponent(e, COMP_TRANSFORM, t); }
                if(mask&(1<<1)) { RigidBodyComponent rb; ss.Read(rb); w.AddComponent(e, COMP_RIGIDBODY, rb); }
            }
        }
    };
}
#endif
