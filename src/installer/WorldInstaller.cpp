/*#include "WorldInstaller.h"
#include "common/File.h"
#include "common/TypeDefine.h"

WorldInstaller::ReadMapFile(std::string file_name){
    File file = File(file_name);
    u32 size = file.GetFileSize();
    if (size % 16 != 0){
        // TODO: Invalid WM file, skip.
        return;
    }
    int block_count = size / 32;
    for (int i = 0; i < block_count; i ++){
        Mesh mesh;
        mesh.triangle_count = file.readU16LE();
        mesh.vertex_count = file.readU16LE();
        for (int d = 0; d < 0xB800; d ++){ // Block size, fixed.
            mesh.compressed_data[d] = file.readU8();
        }
        // TODO: Flip endiannes, use OGRE.
        mesh.data = DecompressLZSSData(mesh.compressed_data);
    }

}
*/
