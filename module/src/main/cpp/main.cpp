#include <cstring>
#include <thread>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <cinttypes>
#include <dirent.h>
#include "hack.h"
#include "zygisk.hpp"
#include "game.h"
#include "log.h"

using zygisk::Api;
using zygisk::AppSpecializeArgs;
using zygisk::ServerSpecializeArgs;

bool DirExists(const std::string &path) {
    struct stat info;
    return stat(path.c_str(), &info) == 0 && S_ISDIR(info.st_mode);
}

std::string GetLibDir(const char *package_name) {
    std::string base_path = "/data/app";
    DIR *dir = opendir(base_path.c_str());
    if (!dir) return {};
    std::string found_path = "";
    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_type == DT_DIR) {
            std::string name = entry->d_name;   
            if (name == "." || name == "..") continue;
            if (name.find(package_name) != std::string::npos) {
                found_path = base_path + "/" + name;
                break; 
            }

            if (name.find("~~") == 0) {
                std::string sub_base = base_path + "/" + name;
                DIR *sub_dir = opendir(sub_base.c_str());
                if (sub_dir) {
                    struct dirent *sub_entry;
                    while ((sub_entry = readdir(sub_dir)) != nullptr) {
                        std::string sub_name = sub_entry->d_name;
                        if (sub_name.find(package_name) != std::string::npos) {
                            found_path = sub_base + "/" + sub_name;
                            break;
                        }
                    }
                    closedir(sub_dir);
                }
            }
            if (!found_path.empty()) break;
        }
    }
    closedir(dir);

    if (found_path.empty()) return {};
    
    if (DirExists(found_path + "/lib/arm64")) return "/lib/arm64";
    if (DirExists(found_path + "/lib/arm")) return "/lib/arm";
    if (DirExists(found_path + "/lib/x86_64")) return "/lib/x86_64";
    if (DirExists(found_path + "/lib/x86")) return "/lib/x86";

    return {};
}

class MyModule : public zygisk::ModuleBase {
public:
    void onLoad(Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void preAppSpecialize(AppSpecializeArgs *args) override {
        auto package_name = env->GetStringUTFChars(args->nice_name, nullptr);
        auto app_data_dir = env->GetStringUTFChars(args->app_data_dir, nullptr);
        preSpecialize(package_name, app_data_dir);
        env->ReleaseStringUTFChars(args->nice_name, package_name);
        env->ReleaseStringUTFChars(args->app_data_dir, app_data_dir);
    }

    void postAppSpecialize(const AppSpecializeArgs*) override {
        if (enable_hack) {
            hack_prepare(game_data_dir, data, length, lib_dir);
        }
    }

private:
    Api *api;
    JNIEnv *env;
    bool enable_hack;
    char *game_data_dir;
    void *data;
    size_t length;
    std::string lib_dir;

    void preSpecialize(const char *package_name, const char *app_data_dir) {
        if (strcmp(package_name, GamePackageName) == 0) {
            LOGI("[CHEAT] detect game: %s", package_name);
            enable_hack = true;
            game_data_dir = new char[strlen(app_data_dir) + 1];
            strcpy(game_data_dir, app_data_dir);
            lib_dir = GetLibDir(package_name);

#if defined(__i386__)
            auto path = "zygisk/armeabi-v7a.so";
#endif
#if defined(__x86_64__)
            auto path = "zygisk/arm64-v8a.so";
#endif
#if defined(__i386__) || defined(__x86_64__)
            int dirfd = api->getModuleDir();
            int fd = openat(dirfd, path, O_RDONLY);
            if (fd != -1) {
                struct stat sb{};
                fstat(fd, &sb);
                length = sb.st_size;
                data = mmap(nullptr, length, PROT_READ, MAP_PRIVATE, fd, 0);
                close(fd);
            } else {
                LOGW("Unable to open arm file");
            }
#endif
        } else {
            api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
        }
    }
};

REGISTER_ZYGISK_MODULE(MyModule)