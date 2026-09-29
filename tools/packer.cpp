#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <cstring>
#include <algorithm>

namespace fs = std::filesystem;

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

int main(int argc, char* argv[])
{
    std::string assetsDir = (argc > 1) ? argv[1] : "assets";
    std::string outputFile = (argc > 2) ? argv[2] : "game.pak";

    std::vector<fs::path> filePaths;

    // 1. Scan assets folder
    if (fs::exists(assetsDir))
    {
        for (const auto &entry : fs::recursive_directory_iterator(assetsDir))
        {
            if (fs::is_regular_file(entry.path()))
            {
                std::string ext = entry.path().extension().string();
                if (ext != ".gitkeep")
                {
                    filePaths.push_back(entry.path());
                }
            }
        }
    }

    // 2. Scan root directory for directly uploaded 3D models (.glb, .gltf)
    for (const auto &entry : fs::directory_iterator("."))
    {
        if (fs::is_regular_file(entry.path()))
        {
            std::string ext = entry.path().extension().string();
            for (char &c : ext) c = (char)::tolower(c);
            if (ext == ".glb" || ext == ".gltf")
            {
                filePaths.push_back(entry.path());
            }
        }
    }

    std::cout << "[PACKER] Found " << filePaths.size() << " assets to pack into " << outputFile << "\n";

    std::vector<PakEntry> entries;
    std::vector<std::vector<char>> fileBuffers;
    unsigned int dataOffset = sizeof(PakHeader) + (filePaths.size() * sizeof(PakEntry));

    for (const auto &p : filePaths)
    {
        std::ifstream file(p, std::ios::binary | std::ios::ate);
        if (!file.is_open()) continue;

        size_t size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<char> buffer(size);
        file.read(buffer.data(), size);

        std::string relPath;
        std::string pStr = p.generic_string();
        if (pStr.rfind(assetsDir + "/", 0) == 0)
        {
            relPath = fs::relative(p, assetsDir).generic_string();
        }
        else if (pStr.rfind("./", 0) == 0)
        {
            relPath = p.filename().generic_string();
        }
        else
        {
            relPath = p.generic_string();
        }

        PakEntry entry;
        std::memset(&entry, 0, sizeof(PakEntry));
        std::strncpy(entry.path, relPath.c_str(), MAX_PAK_PATH - 1);
        entry.offset = dataOffset;
        entry.size = (unsigned int)size;

        entries.push_back(entry);
        fileBuffers.push_back(std::move(buffer));

        std::cout << "  Packed: " << relPath << " (" << size << " bytes)\n";
        dataOffset += size;
    }

    std::ofstream out(outputFile, std::ios::binary);
    PakHeader header;
    std::memcpy(header.magic, PAK_MAGIC, 4);
    header.fileCount = (unsigned int)entries.size();

    out.write(reinterpret_cast<char*>(&header), sizeof(PakHeader));
    for (const auto &e : entries)
    {
        out.write(reinterpret_cast<const char*>(&e), sizeof(PakEntry));
    }
    for (const auto &buf : fileBuffers)
    {
        out.write(buf.data(), buf.size());
    }

    std::cout << "[PACKER] game.pak created successfully (" << dataOffset << " bytes).\n";
    return 0;
}
