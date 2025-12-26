#ifndef TITAN_SCHEDULER_HPP
#define TITAN_SCHEDULER_HPP
#include "Titan_ECS.hpp"
#include <vector>
#include <algorithm>
namespace Titan::ECS {
    struct ISystem { virtual ~ISystem()=default; virtual void Init(World& w){} virtual void Update(World& w,f32 dt)=0; virtual void Shutdown(World& w){} virtual int GetPriority()const{return 0;}};
    struct Scheduler {
        std::vector<ISystem*> s;
        void Register(ISystem* y){ s.push_back(y); std::sort(s.begin(),s.end(),[](ISystem* a,ISystem* b){return a->GetPriority()<b->GetPriority();});}
        void Init(World& w){for(auto* i:s)i->Init(w);} void Update(World& w,f32 dt){for(auto* i:s)i->Update(w,dt);}
        void Shutdown(World& w){for(auto it=s.rbegin();it!=s.rend();++it){(*it)->Shutdown(w);delete *it;} s.clear();}
    };
}
#endif
