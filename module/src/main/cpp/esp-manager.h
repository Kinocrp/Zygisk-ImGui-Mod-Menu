//
// Created by Kino on 10/31/2025.
//

#ifndef ZYGISK_IMGUI_MOD_MENU_ESP_MANAGER_H
#define ZYGISK_IMGUI_MOD_MENU_ESP_MANAGER_H

#include <vector>

class ESPManager {
private:
    void* mainCamera = nullptr;

    struct ESPStruct {
        int objID;
        void* obj;
        uint32_t gchandle;
        float x, y, z;
    };
    std::vector<ESPStruct> ESPObjects;

public:
    void* get_Camera() const { return mainCamera; }
    void set_Camera(void* Camera) { mainCamera = Camera; }

    std::vector<ESPStruct>& get_ESPObjects() { return ESPObjects; }
    const std::vector<ESPStruct>& get_ESPObjects() const { return ESPObjects; }

    void modifyObj(int objID, void* obj, uint32_t gchandle, float x, float y, float z) {
        auto it = std::find_if(ESPObjects.begin(), ESPObjects.end(), [&](const ESPStruct& o) { return o.objID == objID; });
        if (it != ESPObjects.end()) {
            it->obj = obj;
            it->gchandle = gchandle;
            it->x = x;
            it->y = y;
            it->z = z;
        } else {
            ESPObjects.push_back({ objID, obj, gchandle, x, y, z });
        }
    }

    size_t removeObj(int objID) {
        auto it = std::find_if(ESPObjects.begin(), ESPObjects.end(), [&](const ESPStruct& o) { return o.objID == objID; });
        if (it != ESPObjects.end()) {
            size_t ret = it->gchandle;
            ESPObjects.erase(it);
            return ret;
        }
        return 0;
    }
};

#endif //ZYGISK_IMGUI_MOD_MENU_ESP_MANAGER_H
