#include <stdio.h>

#include "system.h"


#if defined(__WIN32__)
#include <windows.h>

const char *get_open_file_name(const char *filter) {
    static char buffer[512];
    OPENFILENAME ofn;

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = buffer;
    /*
     * Set lpstrFile[0] to '\0' so that GetOpenFileName does not
     * use the contents of szFile to initialize itself.
     */
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(buffer);
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn) == TRUE) {
        return ofn.lpstrFile;
    }

    snprintf(buffer, 512, "\"%s\" is not a valid path.", ofn.lpstrFile);
    MessageBox(NULL, buffer, "Error", MB_OK);

    return NULL;
}

const char *get_save_file_name(const char *filter) {
    static char buffer[512];
    OPENFILENAME ofn;

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = buffer;
    /*
     * Set lpstrFile[0] to '\0' so that GetOpenFileName does not
     * use the contents of szFile to initialize itself.
     */
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(buffer);
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetSaveFileName(&ofn) == TRUE) {
        return ofn.lpstrFile;
    }

    snprintf(buffer, 512, "\"%s\" is not a valid path.", ofn.lpstrFile);
    MessageBox(NULL, buffer, "Error", MB_OK);

    return NULL;
}

#endif /* defined(__WIN32__) */
