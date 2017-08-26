#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "themes.h"
#include "unicode.h"
#include "fs.h"

Result single_install(theme theme_to_install)
{
    char *body;
    char *music;
    char *savedata_buf;
    char *thememanage_buf;
    u32 body_size;
    u32 music_size;
    u32 savedata_size;

    savedata_size = file_to_buf(fsMakePath(PATH_ASCII, "/SaveData.dat"), ArchiveHomeExt, &savedata_buf);
    savedata_buf[0x141b] = 0;
    memset(&savedata_buf[0x13b8], 0, 8);
    savedata_buf[0x13bd] = 3;
    savedata_buf[0x13b8] = 0xff;
    u32 size = buf_to_file(savedata_size, "/SaveData.dat", ArchiveHomeExt, savedata_buf);
    printf("Savedata size: %lu\n", savedata_size);
    printf("Return: %lx\n", size);
    free(savedata_buf);

    // Open body cache file. Test if theme is zipped
    if (theme_to_install.is_zip)
    {
        body_size = zip_file_to_buf("body_LZ.bin", theme_to_install.path, &body);
    } else {
        u16 path[0x106] = {0};
        memcpy(path, theme_to_install.path, 0x106 * sizeof(u16));
        struacat(path, "/body_lz.bin");
        body_size = file_to_buf(fsMakePath(PATH_UTF16, path), ArchiveSD, &body);
    }

    if (body_size == 0)
    {
        free(body);
        puts("bodyrip");
        return MAKERESULT(RL_PERMANENT, RS_CANCELED, RM_APPLICATION, RD_NOT_FOUND);
    }

    size = buf_to_file(body_size, "/BodyCache.bin", ArchiveThemeExt, body); // Write body data to file
    free(body);

    if (size == 0) return MAKERESULT(RL_PERMANENT, RS_CANCELED, RM_APPLICATION, RD_NOT_FOUND);

    if (theme_to_install.is_zip) // Same as above but this time with bgm
    {
        music_size = zip_file_to_buf("bgm.bcstm", theme_to_install.path, &music);
    } else {
        u16 path[0x106] = {0};
        memcpy(path, theme_to_install.path, 0x106 * sizeof(16));
        struacat(path, "/bgm.bcstm");
        music_size = file_to_buf(fsMakePath(PATH_UTF16, path), ArchiveSD, &music);
    }

    if (music_size == 0)
    {
        free(music);
        music = calloc(1, 3371008);
    } else if (music_size > 3371008) {
        free(music);
        puts("musicrip");
        return MAKERESULT(RL_PERMANENT, RS_CANCELED, RM_APPLICATION, RD_TOO_LARGE);
    }

    size = buf_to_file(music_size, "/BgmCache.bin", ArchiveThemeExt, music);
    free(music);

    if (size == 0) return MAKERESULT(RL_PERMANENT, RS_CANCELED, RM_APPLICATION, RD_NOT_FOUND);

    file_to_buf(fsMakePath(PATH_ASCII, "/ThemeManage.bin"), ArchiveThemeExt, &thememanage_buf);
    thememanage_buf[0x00] = 1;
    thememanage_buf[0x01] = 0;
    thememanage_buf[0x02] = 0;
    thememanage_buf[0x03] = 0;
    thememanage_buf[0x04] = 0;
    thememanage_buf[0x05] = 0;
    thememanage_buf[0x06] = 0;
    thememanage_buf[0x07] = 0;

    u32 *body_size_location = (u32*)(&thememanage_buf[0x8]);
    u32 *music_size_location = (u32*)(&thememanage_buf[0xC]);
    *body_size_location = body_size;
    *music_size_location = music_size;

    thememanage_buf[0x10] = 0xFF;
    thememanage_buf[0x14] = 0x01;
    thememanage_buf[0x18] = 0xFF;
    thememanage_buf[0x1D] = 0x02;

    memset(&thememanage_buf[0x338], 0, 4);
    memset(&thememanage_buf[0x340], 0, 4);
    memset(&thememanage_buf[0x360], 0, 4);
    memset(&thememanage_buf[0x368], 0, 4);
    size = buf_to_file(0x800, "/ThemeManage.bin", ArchiveThemeExt, thememanage_buf);
    free(thememanage_buf);

    return 0;
}