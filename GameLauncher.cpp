#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>
#include <cstdio>

typedef int (*GameMainFunc)(int argc, char* argv[]);

int main(int argc, char* argv[]) {
    printf("============================================\n");
    printf("  Titan Game Launcher v2.0\n");
    printf("============================================\n\n");
    
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    char* lastSlash = strrchr(exePath, '\\');
    if (lastSlash) *(lastSlash + 1) = '\0';
    
    char dllPath[MAX_PATH];
    snprintf(dllPath, MAX_PATH, "%sTitanGame.dll", exePath);
    
    printf("[Launcher] Loading: %s\n", dllPath);
    
    HMODULE hGame = LoadLibraryA(dllPath);
    if (!hGame) {
        printf("[Launcher] Failed to load TitanGame.dll! Error: %lu\n", GetLastError());
        printf("Press Enter to exit...\n");
        getchar();
        return 1;
    }
    
    printf("[Launcher] DLL loaded successfully!\n");
    
    GameMainFunc GameMain = (GameMainFunc)GetProcAddress(hGame, "GameMain");
    if (!GameMain) {
        printf("[Launcher] GameMain not found!\n");
        FreeLibrary(hGame);
        printf("Press Enter to exit...\n");
        getchar();
        return 1;
    }
    
    printf("[Launcher] Starting game...\n\n");
    
    int result = GameMain(argc, argv);
    
    FreeLibrary(hGame);
    
    printf("\n[Launcher] Game exited with code %d\n", result);
    printf("Press Enter to exit...\n");
    getchar();
    
    return result;
}
