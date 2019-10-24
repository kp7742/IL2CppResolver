#include "IL2CppResolver/SigUtils.h"
#include "IL2CppResolver/Resolver.h"

#include <unistd.h>
#include <pthread.h>


#define InitUFunc(x, y) *reinterpret_cast<void **>(&x) = IL2CppCallResolver::ResolveCall(y)


bool isLibraryLoaded(const char *libraryName) {
    char line[512] = {0};
    FILE *fp = fopen("/proc/self/maps", "rt");
    if (fp != NULL) {
        while (fgets(line, sizeof(line), fp)) {
            if (strstr(line, libraryName))
                return true;
        }
        fclose(fp);
	}
    return false;
}

void *InitGameThread(void *) {
    do {
        sleep(1);
    } while (!isLibraryLoaded("il2cpp.so"));


    LOGI("Game Thread Loaded");
	
	// only initialize in background thread since scanning may take seconds

    if(IL2CppCallResolver::initialize()) {

        int (*Screen$$get_height)();
        int (*Screen$$get_width)();
        InitUFunc(Screen$$get_height, "UnityEngine.Screen::get_height()");
        InitUFunc(Screen$$get_width, "UnityEngine.Screen::get_width()");
        if (Screen$$get_height && Screen$$get_width) {
            LOGI("%dx%d", Screen$$get_height(), Screen$$get_width());
        }

        uintptr_t (*Camera$$get_main)();
        float (*Camera$$get_fieldofview)(uintptr_t);
        void (*Camera$$set_fieldofview)(uintptr_t, float);

        InitUFunc(Camera$$get_main, "UnityEngine.Camera::get_main()");
        InitUFunc(Camera$$set_fieldofview, "UnityEngine.Camera::set_fieldOfView(System.Single)");
        InitUFunc(Camera$$get_fieldofview, "UnityEngine.Camera::get_fieldOfView()");

        if (Camera$$get_main && Camera$$get_fieldofview && Camera$$set_fieldofview) {
            uintptr_t mainCamera = Camera$$get_main();
            if (mainCamera != 0) {
                float oldFOV = Camera$$get_fieldofview(mainCamera);
                Camera$$set_fieldofview(mainCamera, oldFOV * 2.f);
                float newFOV = Camera$$get_fieldofview(mainCamera);
                LOGI("Camera Ptr: %p  |  oldFOV: %.2f  |  newFOV: %.2f", (void *) mainCamera, oldFOV, newFOV);
            } else {
                LOGE("mainCamera is currently not available!");
            }
        }
    }

    return NULL;
}


__attribute__((constructor))
void onload() {
    pthread_t tid_game;
    pthread_create(&tid_game, NULL, InitGameThread, NULL);
}