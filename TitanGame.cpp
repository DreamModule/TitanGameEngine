#define _CRT_SECURE_NO_WARNINGS
#include "../Titan.hpp"
#include <cstdio>
#include <cmath>

using namespace Titan;

enum class GameState { Menu, Playing, Paused };

GameState g_gameState = GameState::Menu;
float g_time = 0;
bool g_isMultiplayer = false;

struct PhysicsObject {
    Vec3 pos;
    Vec3 vel;
    Vec3 angVel;
    Vec3 rotation;
    Vec3 size;
    float r, g, b;
    float mass;
    bool isGrabbed;
    bool onGround;
    
    float GetInertia() const {
        return mass * (size.x * size.x + size.y * size.y) / 6.0f;
    }
};

std::vector<PhysicsObject> g_objects;
int g_grabbedObject = -1;
float g_grabDistance = 3.0f;

float g_playerVelY = 0;
bool g_playerOnGround = true;

namespace Physics {
    constexpr float GRAVITY = -200.0f;
    constexpr float BOUNCE = 0.35f;
    constexpr float GROUND_FRICTION = 0.85f;
    constexpr float AIR_DRAG = 0.01f;
    constexpr float ANGULAR_DRAG = 0.4f;
    constexpr float MIN_VELOCITY = 0.01f;
    constexpr float RESTITUTION = 0.45f;
    constexpr int SUBSTEPS = 4;
    
    Vec3 Cross(Vec3 a, Vec3 b) {
        return {a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x};
    }
    
    float Clamp(float v, float lo, float hi) {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    
    bool TestAABB(Vec3 p1, Vec3 s1, Vec3 p2, Vec3 s2, Vec3& n, float& d) {
        Vec3 h1 = s1 * 0.5f, h2 = s2 * 0.5f;
        float dx = p2.x - p1.x, dy = p2.y - p1.y, dz = p2.z - p1.z;
        float ox = h1.x + h2.x - fabsf(dx);
        float oy = h1.y + h2.y - fabsf(dy);
        float oz = h1.z + h2.z - fabsf(dz);
        if (ox <= 0 || oy <= 0 || oz <= 0) return false;
        if (ox < oy && ox < oz) { d = ox; n = {dx > 0 ? -1.f : 1.f, 0, 0}; }
        else if (oy < oz) { d = oy; n = {0, dy > 0 ? -1.f : 1.f, 0}; }
        else { d = oz; n = {0, 0, dz > 0 ? -1.f : 1.f}; }
        return true;
    }
    
    void Integrate(PhysicsObject& o, float dt) {
        o.vel.y += GRAVITY * dt;
        float linDamp = expf(-AIR_DRAG * dt);
        float angDamp = expf(-ANGULAR_DRAG * dt);
        o.vel.x *= linDamp; o.vel.z *= linDamp;
        o.pos = o.pos + o.vel * dt;
        o.angVel = o.angVel * angDamp;
        o.rotation = o.rotation + o.angVel * dt;
    }
    
    void ResolveGround(PhysicsObject& o) {
        float halfH = o.size.y * 0.5f;
        if (o.pos.y >= halfH) { o.onGround = false; return; }
        o.pos.y = halfH;
        o.onGround = true;
        if (o.vel.y < -0.5f) {
            o.vel.y = -o.vel.y * BOUNCE;
            o.angVel.z -= o.vel.x * 0.2f / o.mass;
            o.angVel.x += o.vel.z * 0.2f / o.mass;
        } else {
            o.vel.y = 0;
        }
        o.vel.x *= GROUND_FRICTION;
        o.vel.z *= GROUND_FRICTION;
    }
    
    void ResolveCollision(PhysicsObject& a, PhysicsObject& b, float dt) {
        Vec3 n; float d;
        Vec3 sa = {a.size.x, a.size.y, a.size.x};
        Vec3 sb = {b.size.x, b.size.y, b.size.x};
        if (!TestAABB(a.pos, sa, b.pos, sb, n, d)) return;
        
        float invMassA = 1.0f / a.mass, invMassB = 1.0f / b.mass;
        float totalInvMass = invMassA + invMassB;
        a.pos = a.pos + n * (d * invMassA / totalInvMass);
        b.pos = b.pos - n * (d * invMassB / totalInvMass);
        
        Vec3 relVel = a.vel - b.vel;
        float vn = relVel.Dot(n);
        if (vn >= 0) return;
        
        float j = -(1.0f + RESTITUTION) * vn / totalInvMass;
        a.vel = a.vel + n * (j * invMassA);
        b.vel = b.vel - n * (j * invMassB);
        
        Vec3 ra = {b.pos.x - a.pos.x, b.pos.y - a.pos.y, b.pos.z - a.pos.z};
        Vec3 rb = {a.pos.x - b.pos.x, a.pos.y - b.pos.y, a.pos.z - b.pos.z};
        Vec3 ta = Cross(ra, n * j * 0.15f);
        Vec3 tb = Cross(rb, n * (-j) * 0.15f);
        float ia = a.GetInertia(), ib = b.GetInertia();
        if (ia > 0.01f) a.angVel = a.angVel + ta * (1.0f / ia);
        if (ib > 0.01f) b.angVel = b.angVel + tb * (1.0f / ib);
        
        if (n.y > 0.8f) {
            Vec3 off = {b.pos.x - a.pos.x, 0, b.pos.z - a.pos.z};
            float offLen = sqrtf(off.x*off.x + off.z*off.z);
            float support = a.size.x * 0.35f;
            if (offLen > support && offLen > 0.01f) {
                float tip = (offLen - support) * 12.0f * (b.mass / a.mass);
                Vec3 tipDir = {off.x / offLen, 0, off.z / offLen};
                b.angVel.z += tipDir.x * tip * dt;
                b.angVel.x -= tipDir.z * tip * dt;
                b.vel.x += tipDir.x * tip * 0.3f * dt;
                b.vel.z += tipDir.z * tip * 0.3f * dt;
            }
        }
    }
    
    void Update(std::vector<PhysicsObject>& objects, float dt) {
        float subDt = dt / SUBSTEPS;
        for (int s = 0; s < SUBSTEPS; s++) {
            for (auto& o : objects) {
                if (!o.isGrabbed) Integrate(o, subDt);
            }
            for (auto& o : objects) {
                if (!o.isGrabbed) ResolveGround(o);
            }
            for (size_t i = 0; i < objects.size(); i++) {
                if (objects[i].isGrabbed) continue;
                for (size_t j = i + 1; j < objects.size(); j++) {
                    if (objects[j].isGrabbed) continue;
                    ResolveCollision(objects[i], objects[j], subDt);
                }
            }
        }
    }
}

const float JUMP_FORCE = 14.0f;

void Log(const char* ns, const char* func) {
    char buf[256];
    snprintf(buf, 256, "%s::%s", ns, func);
    Logger::Add(buf, 255, 230, 100);
}

void StartSingleplayer() {
    g_gameState = GameState::Playing;
    g_isMultiplayer = false;
    Input::SetMouseLocked(true);
    
    Camera3D::SetPosition(0, 2.0f, 15);
    Camera3D::SetFOV(70.0f);
    Camera3D::SetSpeed(5.0f);
    Camera3D::SetSensitivity(0.003f);
    
    g_objects.clear();
    g_playerVelY = 0;
    g_playerOnGround = true;
    
    for (int i = 0; i < 5; i++) {
        PhysicsObject obj;
        obj.pos = {(float)(i - 2) * 2.0f, 10.0f + i * 2.5f, 0};
        obj.vel = {0, 0, 0};
        obj.angVel = {0, 0, 0};
        obj.rotation = {0, 0, 0};
        obj.size = {1.0f, 1.0f, 1.0f};
        obj.r = 0.8f; obj.g = 0.3f; obj.b = 0.2f;
        obj.mass = 2.0f + i * 1.0f;
        obj.isGrabbed = false;
        obj.onGround = false;
        g_objects.push_back(obj);
    }
    
    PhysicsObject heavy;
    heavy.pos = {0, 18, 0};
    heavy.vel = {0, 0, 0};
    heavy.angVel = {0, 0, 0};
    heavy.rotation = {0, 0, 0};
    heavy.size = {2.0f, 2.0f, 2.0f};
    heavy.r = 0.2f; heavy.g = 0.5f; heavy.b = 0.9f;
    heavy.mass = 15.0f;
    heavy.isGrabbed = false;
    heavy.onGround = false;
    g_objects.push_back(heavy);
    
    PhysicsObject light;
    light.pos = {5, 8, 0};
    light.vel = {0, 0, 0};
    light.angVel = {0, 0, 0};
    light.rotation = {0, 0, 0};
    light.size = {0.6f, 0.6f, 0.6f};
    light.r = 0.9f; light.g = 0.9f; light.b = 0.2f;
    light.mass = 0.8f;
    light.isGrabbed = false;
    light.onGround = false;
    g_objects.push_back(light);
    
    Log("Game", "StartSingleplayer()");
}

void JoinServer(const char* ip, uint16_t port) {
    g_gameState = GameState::Playing;
    g_isMultiplayer = true;
    Input::SetMouseLocked(true);
    Camera3D::SetPosition(0, 2.0f, 15);
    g_playerVelY = 0;
    g_playerOnGround = true;
    
    char buf[128];
    snprintf(buf, 128, "Net::Connect(%s:%d)", ip, port);
    Log("Network", buf);
}

void HostServer(const char* name) {
    g_gameState = GameState::Playing;
    g_isMultiplayer = true;
    Input::SetMouseLocked(true);
    Camera3D::SetPosition(0, 2.0f, 15);
    g_playerVelY = 0;
    g_playerOnGround = true;
    
    Log("Network", "HostServer()");
}

void OnResume() {
    g_gameState = GameState::Playing;
    Input::SetMouseLocked(true);
    Log("UI", "Resume()");
}

void OnSettings() {
    MainMenu::g_state = MainMenu::State::Settings;
    MainMenu::g_selected = 0;
}

void OnMainMenu() {
    g_gameState = GameState::Menu;
    Input::SetMouseLocked(false);
    MainMenu::Reset();
    Log("UI", "MainMenu()");
}

void OnQuit() {
    Window::Close();
}

int RaycastObject(Vec3 origin, Vec3 dir, float maxDist) {
    int closest = -1;
    float closestDist = maxDist;
    
    for (size_t i = 0; i < g_objects.size(); i++) {
        Vec3 toObj = g_objects[i].pos - origin;
        float t = toObj.Dot(dir);
        if (t < 0 || t > closestDist) continue;
        
        Vec3 closestPt = origin + dir * t;
        float dist = (closestPt - g_objects[i].pos).Length();
        float hitRadius = fmaxf(g_objects[i].size.x, g_objects[i].size.y) * 0.6f;
        
        if (dist < hitRadius && t < closestDist) {
            closestDist = t;
            closest = (int)i;
        }
    }
    return closest;
}

extern "C" __declspec(dllexport) int GameMain(int argc, char* argv[]) {
    printf("[TitanGame] Starting...\n");
    
    if (!Window::Create("Titan Game", 1280, 720)) {
        printf("[TitanGame] Window creation failed\n");
        return 1;
    }
    
    Logger::SetFadeTime(8.0f);
    Logger::Enable(true);
    Log("Window", "Create(1280, 720)");
    Log("Engine", "Initialize()");
    
    MainMenu::SetOnSingleplayer(StartSingleplayer);
    MainMenu::SetOnJoinServer(JoinServer);
    MainMenu::SetOnHostServer(HostServer);
    
    UI::SetPauseTitle("PAUSED");
    UI::AddPauseButton("Resume", OnResume);
    UI::AddPauseButton("Settings", OnSettings);
    UI::AddPauseButton("Main Menu", OnMainMenu);
    UI::AddPauseButton("Quit", OnQuit);
    
    Light::Enable();
    Light::AddDirectional(0.5f, -1.0f, 0.3f, 1.0f, 0.95f, 0.9f, 1.0f);
    
    while (Window::IsOpen()) {
        float dt = Time::GetDelta();
        if (dt > 0.05f) dt = 0.05f;
        g_time += dt;
        
        switch (g_gameState) {
            case GameState::Menu: {
                MainMenu::Update();
                Graphics::Clear(0.02f, 0.02f, 0.08f);
                MainMenu::Render();
                break;
            }
            
            case GameState::Playing: {
                if (Input::GetKeyDown(KeyCode::Escape)) {
                    g_gameState = GameState::Paused;
                    Input::SetMouseLocked(false);
                    Log("UI", "Pause()");
                }
                
                if (Input::GetKeyDown(KeyCode::F1)) {
                    static bool logOn = true;
                    logOn = !logOn;
                    Logger::Enable(logOn);
                    if (logOn) Log("Logger", "Enable(true)");
                }
                
                Vec2 mouse = Input::GetMouseDelta();
                Camera3D::Rotate(mouse.x, mouse.y);
                
                float forward = 0, strafe = 0;
                if (Input::GetKey(KeyCode::W)) forward += 1;
                if (Input::GetKey(KeyCode::S)) forward -= 1;
                if (Input::GetKey(KeyCode::A)) strafe -= 1;
                if (Input::GetKey(KeyCode::D)) strafe += 1;
                
                float speed = Input::GetKey(KeyCode::Shift) ? 2.0f : 1.0f;
                
                Vec3 camPos = Camera3D::GetPosition();
                
                g_playerVelY += Physics::GRAVITY * dt;
                camPos.y += g_playerVelY * dt;
                
                if (camPos.y < 2.0f) {
                    camPos.y = 2.0f;
                    g_playerVelY = 0;
                    g_playerOnGround = true;
                } else {
                    g_playerOnGround = false;
                }
                
                if (Input::GetKeyDown(KeyCode::Space) && g_playerOnGround) {
                    g_playerVelY = JUMP_FORCE;
                    g_playerOnGround = false;
                    Log("Player", "Jump()");
                }
                
                Camera3D::SetPosition(camPos.x, camPos.y, camPos.z);
                Camera3D::Move(forward * speed, strafe * speed, 0, dt);
                
                Vec3 camFwd = Camera3D::GetForward();
                camPos = Camera3D::GetPosition();
                
                if (Input::GetKey(KeyCode::MouseLeft)) {
                    if (g_grabbedObject == -1) {
                        int hit = RaycastObject(camPos, camFwd, 8.0f);
                        if (hit >= 0) {
                            g_grabbedObject = hit;
                            g_objects[hit].isGrabbed = true;
                            g_objects[hit].vel = {0, 0, 0};
                            g_objects[hit].angVel = {0, 0, 0};
                            g_grabDistance = (g_objects[hit].pos - camPos).Length();
                            Log("Physics", "GrabObject()");
                        }
                    }
                    
                    if (g_grabbedObject >= 0) {
                        Vec3 targetPos = camPos + camFwd * g_grabDistance;
                        auto& obj = g_objects[g_grabbedObject];
                        Vec3 diff = targetPos - obj.pos;
                        obj.vel = diff * 18.0f;
                        obj.pos = obj.pos + diff * (dt * 15.0f);
                        obj.rotation = obj.rotation * 0.9f;
                    }
                } else {
                    if (g_grabbedObject >= 0) {
                        auto& obj = g_objects[g_grabbedObject];
                        obj.isGrabbed = false;
                        float throwPower = obj.vel.Length() * 0.3f;
                        obj.angVel.x = (obj.vel.z / (obj.mass * 0.5f)) * throwPower * 0.1f;
                        obj.angVel.z = -(obj.vel.x / (obj.mass * 0.5f)) * throwPower * 0.1f;
                        Log("Physics", "ReleaseObject()");
                        g_grabbedObject = -1;
                    }
                }
                
                if (Input::GetKey(KeyCode::MouseRight)) {
                    g_grabDistance = fminf(g_grabDistance + dt * 5.0f, 15.0f);
                }
                if (Input::GetKeyDown(KeyCode::Q)) {
                    g_grabDistance = fmaxf(g_grabDistance - 1.0f, 2.0f);
                }
                
                Physics::Update(g_objects, dt);
                
                Graphics::Clear(0.08f, 0.1f, 0.15f);
                Camera3D::Apply();
                
                glColor3f(0.2f, 0.22f, 0.28f);
                glBegin(GL_QUADS);
                glNormal3f(0, 1, 0);
                glVertex3f(-50, 0, -50);
                glVertex3f(50, 0, -50);
                glVertex3f(50, 0, 50);
                glVertex3f(-50, 0, 50);
                glEnd();
                
                glColor3f(0.12f, 0.14f, 0.18f);
                glBegin(GL_LINES);
                for (int i = -50; i <= 50; i += 4) {
                    glVertex3f((float)i, 0.01f, -50);
                    glVertex3f((float)i, 0.01f, 50);
                    glVertex3f(-50, 0.01f, (float)i);
                    glVertex3f(50, 0.01f, (float)i);
                }
                glEnd();
                
                for (size_t i = 0; i < g_objects.size(); i++) {
                    auto& obj = g_objects[i];
                    float rc = obj.r, gc = obj.g, bc = obj.b;
                    if (obj.isGrabbed) { rc = 1; gc = 1; bc = 0.3f; }
                    
                    glPushMatrix();
                    glTranslatef(obj.pos.x, obj.pos.y, obj.pos.z);
                    glRotatef(obj.rotation.x * 57.3f, 1, 0, 0);
                    glRotatef(obj.rotation.y * 57.3f, 0, 1, 0);
                    glRotatef(obj.rotation.z * 57.3f, 0, 0, 1);
                    Mesh::DrawCube(0, 0, 0, obj.size.x, rc, gc, bc);
                    glPopMatrix();
                }
                
                Light::Disable();
                Debug::DrawCrosshair(12.0f);
                
                glMatrixMode(GL_PROJECTION);
                glPushMatrix();
                glLoadIdentity();
                glOrtho(0, Internal::g_width, Internal::g_height, 0, -1, 1);
                glMatrixMode(GL_MODELVIEW);
                glPushMatrix();
                glLoadIdentity();
                
                if (g_grabbedObject >= 0) {
                    glColor3f(1.0f, 1.0f, 0.3f);
                    char massStr[32];
                    snprintf(massStr, 32, "HOLDING (%.1fkg)", g_objects[g_grabbedObject].mass);
                    Graphics::DrawText(Internal::g_width/2 - 60, Internal::g_height/2 + 25, massStr);
                }
                
                glPopMatrix();
                glMatrixMode(GL_PROJECTION);
                glPopMatrix();
                
                Light::Enable();
                break;
            }
            
            case GameState::Paused: {
                if (Input::GetKeyDown(KeyCode::Escape)) {
                    if (MainMenu::g_state != MainMenu::State::Main) {
                        MainMenu::g_state = MainMenu::State::Main;
                    } else {
                        g_gameState = GameState::Playing;
                        Input::SetMouseLocked(true);
                    }
                }
                
                Graphics::Clear(0.05f, 0.05f, 0.1f);
                
                glMatrixMode(GL_PROJECTION);
                glPushMatrix();
                glLoadIdentity();
                glOrtho(0, Internal::g_width, Internal::g_height, 0, -1, 1);
                glMatrixMode(GL_MODELVIEW);
                glPushMatrix();
                glLoadIdentity();
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                
                float w = (float)Internal::g_width;
                float h = (float)Internal::g_height;
                
                if (MainMenu::g_state == MainMenu::State::Settings) {
                    MainMenu::Update();
                    
                    glColor3f(1, 1, 1);
                    Graphics::DrawText(w/2 - 50, 100, "SETTINGS");
                    
                    float btnW = 300, btnH = 45, btnX = (w - btnW)/2, btnY = 160, gap = 12;
                    
                    char buf[64];
                    snprintf(buf, 64, "Sensitivity: %.2f", MainMenu::g_mouseSens);
                    MainMenu::DrawButton(btnX, btnY, btnW, btnH, buf, MainMenu::g_selected == 0, false);
                    
                    snprintf(buf, 64, "Volume: %.0f%%", MainMenu::g_volume * 100);
                    MainMenu::DrawButton(btnX, btnY + btnH + gap, btnW, btnH, buf, MainMenu::g_selected == 1, false);
                    
                    MainMenu::DrawButton(btnX, btnY + (btnH+gap)*2, btnW, btnH, 
                        MainMenu::g_fullscreen ? "Fullscreen: ON" : "Fullscreen: OFF", MainMenu::g_selected == 2, false);
                    MainMenu::DrawButton(btnX, btnY + (btnH+gap)*3, btnW, btnH,
                        MainMenu::g_vsync ? "VSync: ON" : "VSync: OFF", MainMenu::g_selected == 3, false);
                    MainMenu::DrawButton(btnX, btnY + (btnH+gap)*4, btnW, btnH, "BACK", MainMenu::g_selected == 4, false);
                } else {
                    glColor3f(1, 1, 1);
                    Graphics::DrawText(w/2 - 40, 100, "PAUSED");
                    
                    float btnW = 250, btnH = 45, btnX = (w - btnW)/2, btnY = 180, gap = 12;
                    static int sel = 0;
                    
                    if (Input::GetKeyDown(KeyCode::Up)) sel = (sel + 3) % 4;
                    if (Input::GetKeyDown(KeyCode::Down)) sel = (sel + 1) % 4;
                    
                    MainMenu::DrawButton(btnX, btnY, btnW, btnH, "RESUME", sel == 0, false);
                    MainMenu::DrawButton(btnX, btnY + btnH + gap, btnW, btnH, "SETTINGS", sel == 1, false);
                    MainMenu::DrawButton(btnX, btnY + (btnH+gap)*2, btnW, btnH, "MAIN MENU", sel == 2, false);
                    MainMenu::DrawButton(btnX, btnY + (btnH+gap)*3, btnW, btnH, "QUIT", sel == 3, false);
                    
                    if (Input::GetKeyDown(KeyCode::Enter) || Input::GetKeyDown(KeyCode::Space)) {
                        if (sel == 0) OnResume();
                        else if (sel == 1) OnSettings();
                        else if (sel == 2) OnMainMenu();
                        else if (sel == 3) OnQuit();
                    }
                }
                
                glDisable(GL_BLEND);
                glPopMatrix();
                glMatrixMode(GL_PROJECTION);
                glPopMatrix();
                break;
            }
        }
        
        Logger::Update();
        Logger::Render();
        
        Window::SwapBuffers();
    }
    
    return 0;
}
