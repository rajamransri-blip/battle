#ifndef PAK_SYSTEM_H
#define PAK_SYSTEM_H

#include "raylib.h"
#include <cstring>
#include <vector>
#include <string>

#define PAK_MAGIC "XPAK"
#define MAX_PAK_PATH 128

#pragma pack(push, 1)
struct PakHeader
{
    char magic[4];
    unsigned int fileCount;
};

struct PakEntry
{
    char path[MAX_PAK_PATH];
    unsigned int offset;
    unsigned int size;
};
#pragma pack(pop)

static unsigned char *g_PakData = nullptr;
static int g_PakSize = 0;
static std::vector<PakEntry> g_PakEntries;

static unsigned char *PakLoadFileDataCallback(const char *fileName, int *dataSize)
{
    if (g_PakData && !g_PakEntries.empty())
    {
        std::string search = fileName;
        for (char &c : search) if (c == '\\') c = '/';
        if (search.rfind("assets/", 0) == 0) search = search.substr(7);

        for (const auto &entry : g_PakEntries)
        {
            if (search == entry.path)
            {
                if (entry.offset + entry.size <= (unsigned int)g_PakSize)
                {
                    unsigned char *buffer = (unsigned char *)RL_MALLOC(entry.size);
                    if (buffer)
                    {
                        memcpy(buffer, g_PakData + entry.offset, entry.size);
                        *dataSize = (int)entry.size;
                        return buffer;
                    }
                }
            }
        }
    }

    // Recursion safe fallback to default raylib loader
    SetLoadFileDataCallback(nullptr);
    unsigned char *result = LoadFileData(fileName, dataSize);
    SetLoadFileDataCallback(PakLoadFileDataCallback);
    return result;
}

static bool InitPakSystem(const char *pakFileName)
{
    if (!FileExists(pakFileName)) return false;

    g_PakData = LoadFileData(pakFileName, &g_PakSize);
    if (!g_PakData || g_PakSize < (int)sizeof(PakHeader)) return false;

    PakHeader header;
    memcpy(&header, g_PakData, sizeof(PakHeader));

    if (strncmp(header.magic, PAK_MAGIC, 4) != 0)
    {
        UnloadFileData(g_PakData);
        g_PakData = nullptr;
        return false;
    }

    unsigned int tableOffset = sizeof(PakHeader);
    g_PakEntries.resize(header.fileCount);

    for (unsigned int i = 0; i < header.fileCount; ++i)
    {
        memcpy(&g_PakEntries[i], g_PakData + tableOffset, sizeof(PakEntry));
        tableOffset += sizeof(PakEntry);
    }

    SetLoadFileDataCallback(PakLoadFileDataCallback);
    TraceLog(LOG_INFO, "PAK: Mounted %s (%d files)", pakFileName, header.fileCount);
    return true;
}

static void ClosePakSystem()
{
    if (g_PakData)
    {
        UnloadFileData(g_PakData);
        g_PakData = nullptr;
    }
    g_PakEntries.clear();
}

static bool FileExistsInPak(const char *fileName)
{
    std::string search = fileName;
    for (char &c : search) if (c == '\\') c = '/';
    if (search.rfind("assets/", 0) == 0) search = search.substr(7);

    for (const auto &entry : g_PakEntries)
    {
        if (search == entry.path) return true;
    }
    return FileExists(fileName);
}

#endif
