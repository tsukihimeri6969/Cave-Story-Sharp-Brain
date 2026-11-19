#undef fopen
#undef remove
#undef rename
#undef stat
#undef getenv

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <windows.h>

// "\Storage Card\アプリ\CaveStory\" encoded in UTF-8 bytes
static const char* BASE_DIR = "\\Storage Card\\\xE3\x82\xA2\xE3\x83\x97\xE3\x83\xAA\\CaveStory\\";

static void resolve_wince_path(const char* in_path, char* out_path, size_t max_len) {
    if (!in_path || in_path[0] == '\\' || in_path[0] == '/') {
        snprintf(out_path, max_len, "%s", in_path ? in_path : "");
        return;
    }
    const char* clean = (strncmp(in_path, "./", 2) == 0) ? in_path + 2 : in_path;
    snprintf(out_path, max_len, "%s%s", BASE_DIR, clean);
    for (size_t i = 0; out_path[i] != '\0'; ++i) {
        if (out_path[i] == '/') out_path[i] = '\\';
    }
}

extern "C" {

FILE* wince_fopen(const char* path, const char* mode) {
    char full_path[MAX_PATH];
    resolve_wince_path(path, full_path, sizeof(full_path));
    return fopen(full_path, mode);
}

int wince_remove(const char* path) {
    char full_path[MAX_PATH];
    resolve_wince_path(path, full_path, sizeof(full_path));
    wchar_t wpath[MAX_PATH];
    MultiByteToWideChar(CP_UTF8, 0, full_path, -1, wpath, MAX_PATH);
    return DeleteFileW(wpath) ? 0 : -1;
}

int wince_rename(const char* oldpath, const char* newpath) {
    char full_old[MAX_PATH], full_new[MAX_PATH];
    resolve_wince_path(oldpath, full_old, sizeof(full_old));
    resolve_wince_path(newpath, full_new, sizeof(full_new));
    wchar_t wold[MAX_PATH], wnew[MAX_PATH];
    MultiByteToWideChar(CP_UTF8, 0, full_old, -1, wold, MAX_PATH);
    MultiByteToWideChar(CP_UTF8, 0, full_new, -1, wnew, MAX_PATH);
    return MoveFileW(wold, wnew) ? 0 : -1;
}

int wince_stat(const char* path, struct stat* buf) {
    char full_path[MAX_PATH];
    resolve_wince_path(path, full_path, sizeof(full_path));
    return stat(full_path, buf);
}

char* wince_getenv(const char* name) {
    if (name && strcmp(name, "SDL_VIDEODRIVER") == 0) {
        static char drv[] = "windib";
        return drv;
    }
    return NULL;
}

}
