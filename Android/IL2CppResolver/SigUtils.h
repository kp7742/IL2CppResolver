#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <vector>
#include <string>

#include "Logger.h"


namespace SigUtils {

    struct LibraryInfo {
        uintptr_t startAddr;
        uintptr_t endAddr;
        intptr_t size;
        std::string path;
    };

    constexpr uint8_t _skipChar = '?';
    constexpr uint8_t _solidChar = 'x';

    inline LibraryInfo findLibraryInfo(const char *libraryName) {
        LibraryInfo retMap = {};
        char line[512] = {0};

        FILE *fp = fopen("/proc/self/maps", "rt");
        if (fp != NULL) {
            while (fgets(line, sizeof(line), fp)) {
                if (strstr(line, libraryName)) {
                    LibraryInfo tmpMap;
                    char tmpPathname[400] = {0};
                    // parse a line in maps file
                    // (format) startAddress-endAddress perms offset dev inode pathname
                    sscanf(line, "%llx-%llx %*s %*ld %*s %*d %s",
                           (long long unsigned *) &tmpMap.startAddr,
                           (long long unsigned *) &tmpMap.endAddr,
                           tmpPathname);

                    if (retMap.startAddr == 0)
                        retMap.startAddr = tmpMap.startAddr;

                    retMap.endAddr = tmpMap.endAddr;
                    retMap.size = retMap.endAddr - retMap.startAddr;

                    if (retMap.path.empty())
                        retMap.path = tmpPathname;
                }
            }
            fclose(fp);
        }
        return retMap;
    }

    inline std::string generateMask(const char *sig, intptr_t len) {
        std::string mask;
        for (intptr_t i = 0; i < len; i++) {
            if ((uint8_t) sig[i] == _skipChar) {
                mask += _skipChar;
            } else {
                mask += _solidChar;
            }
        }
        return mask;
    }

    inline bool compare(const char *d1, const char *d2, const char *mask, intptr_t len) {
        for (intptr_t i = 0; i < len; i++)
            if ((uint8_t) d1[i] != (uint8_t) d2[i] && mask[i] != _skipChar){
                return false;
        }
        return true;
    }

    inline void *
    tryFind(uintptr_t start, intptr_t size, const char *sig, const char *mask, intptr_t cmpLen) {
        uintptr_t end = (start + size) - cmpLen;
        for (uintptr_t curr = start; curr < end; curr++) {
            if (compare((const char *) curr, sig, mask, cmpLen))
                return (void *) curr;
        }
        return NULL;
    }

};