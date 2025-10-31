//
// Created by Kino on 10/31/2025.
//

#ifndef ZYGISK_IMGUI_MOD_MENU_ESP_MANAGER_H
#define ZYGISK_IMGUI_MOD_MENU_ESP_MANAGER_H

#include <vector>
#include <thread>
#include <chrono>

class ESPManager {
private:
    int ESP_FPS = 60;
    void* mainCamera = nullptr;
    std::mutex espMutex;

    struct ESPStruct {
        void* espObj;
        int objID;
        float x, y;
    };
    std::vector<ESPStruct> ESPObjects;

public:
    int get_FPS() const { return ESP_FPS; }
    void set_FPS(int FPS) { ESP_FPS = FPS; }

    void* get_Camera() const { return mainCamera; }
    void set_Camera(void* Camera) { mainCamera = Camera; }

    std::vector<ESPStruct>& get_ESPObjects() { return ESPObjects; }
    const std::vector<ESPStruct>& get_ESPObjects() const { return ESPObjects; }

    void addObj(void* espObj, int objID, float x, float y) {
        std::lock_guard<std::mutex> lock(espMutex);
        ESPObjects.push_back({ espObj, objID, x, y });
    }

    void removeObj(int objID) {
        std::lock_guard<std::mutex> lock(espMutex);
        auto it = std::find_if(ESPObjects.begin(), ESPObjects.end(), [&](const ESPStruct& obj) { return obj.objID == objID; });
        if (it != ESPObjects.end()) { ESPObjects.erase(it); }
    }

    void clearAllObj() {
        std::lock_guard<std::mutex> lock(espMutex);
        ESPObjects.clear();
    }
};

#endif //ZYGISK_IMGUI_MOD_MENU_ESP_MANAGER_H
