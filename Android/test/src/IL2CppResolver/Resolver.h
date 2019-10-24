#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <vector>
#include <string>

#include "Logger.h"


constexpr char resolve_icall$$sig[]
        {
                0xF0, 0x4D, 0x2D, 0xE9, 0x18, 0xB0, 0x8D, 0xE2,
                0x18, 0xD0, 0x4D, 0xE2, 0x00, 0x40, 0xA0, 0xE1,
                0x10, 0x00, 0x8D, 0xE2, 0x0C, 0x20, 0x8D, 0xE2,
                0x04, 0x10, 0xA0, 0xE1, '?', '?', '?', 0xEB,
                0x5C, 0x02, 0x9F, 0xE5, 0x10, 0x10, 0x8D, 0xE2,
                0x58, 0x72, 0x9F, 0xE5, 0x00, 0x60, 0x8F, 0xE0,
                0x06, 0x00, 0x87, 0xE0, '?', '?', '?', 0xEB,
                0x4C, 0x82, 0x9F, 0xE5, 0x00, 0x50, 0xA0, 0xE1,
                0x10, 0x10, 0x9D, 0xE5, 0x0C, 0x00, 0x41, 0xE2,
                0x06, 0x20, 0x98, 0xE7, 0x02, 0x00, 0x50, 0xE1,
                0x35, 0x00, 0x00, 0x1A, 0x34, 0x02, 0x9F, 0xE5,
                0x00, 0x00, 0x8F, 0xE0, 0x00, 0x00, 0x87, 0xE0
        };

// SigUtils::generateMask(resolve_icall$$sig, sizeof(resolve_icall$$sig), '?');
constexpr char resolve_icall$$mask[] = "xxxxxxxxxxxxxxxxxxxxxxxxxxxx???xxxxxxxxxxxxxxxxxxxxx???xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";


struct IL2CppCachedCall {
public:
    void *addr;
    std::string name;
};


class IL2CppCallResolver {
private:
    void *(*il2cpp_resolve_icall)(const char *);


    static IL2CppCallResolver *getInstance() {
        static IL2CppCallResolver *instance = NULL;
        if (!instance)
            instance = new IL2CppCallResolver();
        return instance;
    }

public:
    static bool initialize() {
        IL2CppCallResolver *inst = getInstance();
        if (!inst) {
            LOGE("Couldn't create instance");
            return false;
        }
        if (!inst->il2cpp_resolve_icall) {
            SigUtils::LibraryInfo info = SigUtils::findLibraryInfo("il2cpp.so");
            if (info.startAddr != 0 && info.endAddr != 0) {
				
                *reinterpret_cast<void **>(&inst->il2cpp_resolve_icall) = SigUtils::tryFind(
                        info.startAddr, info.size,
                        resolve_icall$$sig, resolve_icall$$mask,
                        strlen(resolve_icall$$mask));
						
                LOGI("il2cpp_resolve_icall address: %p", inst->il2cpp_resolve_icall);
                return inst->il2cpp_resolve_icall != NULL;
            } else {
                LOGE("Couldn't find il2cpp map!");
                return false;
            }
        }
		return false;
    }

    static void *ResolveCall(std::string funcName) {
        static std::vector<IL2CppCachedCall> cachedCalls;
        for (int i = 0; i < cachedCalls.size(); i++) {
            if (cachedCalls[i].name == funcName)
                return cachedCalls[i].addr;
        }

        IL2CppCallResolver *inst = getInstance();
        if (!inst) {
            LOGE("Couldn't create instance");
            return NULL;
        }
        if (!inst->il2cpp_resolve_icall) {
            LOGE("il2cpp_resolve_icall is null!");
            return NULL;
        }

        void *ret = inst->il2cpp_resolve_icall(funcName.c_str());
        if (ret) {
            IL2CppCachedCall cache;
            cache.addr = ret;
            cache.name = funcName;
            cachedCalls.push_back(cache);
        }
        LOGI("%s address: %p", funcName.c_str(), ret);
        return ret;
    }
};
