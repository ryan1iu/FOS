#include <dirent.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

int main(int argc, char* argv[]) {
    char* dir_path = "/home/ryan/fos/obj/user";
    if (argc > 1) {
        dir_path = argv[1];
    }

    char* file_names[256];
    char* file_paths[256];
    int file_sector[256];

    // List all programs.
    DIR* d = opendir(dir_path);
    struct dirent* dir;
    int num = 0;
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (strcmp(dir->d_name, ".") == 0 ||
                strcmp(dir->d_name, "..") == 0) {
                continue;
            }

            char* filename = (char*)malloc(strlen(dir->d_name) + 1);
            strcat(filename, dir->d_name);
            file_names[num] = filename;

            char* filepath =
                (char*)malloc(strlen(dir->d_name) + strlen(dir_path) + 2);
            strcat(filepath, dir_path);
            strcat(filepath, "/");
            strcat(filepath, dir->d_name);
            file_paths[num] = filepath;

            num++;
        }
        closedir(d);
    }

    // Create disk image file.
    FILE* image_file;
    if ((image_file = fopen("/home/ryan/fos/obj/disk/user_disk", "w+")) == 0) {
        printf("create image file failed");
        exit(1);
    }

    char char_end = '\0';

    uint32_t sector_index = 10;
    uint8_t zero = 0;
    int file_size[num];
    int file_data_offsets[num];
    uint8_t magic = 0xAA;

    // 用0填充第一个扇区
    fwrite(&zero, sizeof(uint8_t), 512, image_file);

    // Write meta data.
    for (int i = 0; i < num; i++) {
        // 初始化所在扇区
        file_sector[i] = sector_index;

        char* file_name = file_names[i];
        char* file_path = file_paths[i];

        // 获取文件大小
        struct stat st;
        stat(file_path, &st);
        int size = st.st_size;
        file_size[i] = size;

        // 写入magic
        fwrite(&magic, sizeof(uint8_t), 1, image_file);
        // 写入文件名
        fwrite(file_name, sizeof(char), strlen(file_name), image_file);
        // 剩下的部分用'\n'填充
        fwrite(&zero, sizeof(char), 64 - strlen(file_name), image_file);

        // 写入文件id
        fwrite(&i, sizeof(uint8_t), 1, image_file);

        // 写入所在扇区
        fwrite(&sector_index, sizeof(int), 1, image_file);

        sector_index += size / 512 + 1;

        // 写入文件大小
        fwrite(&size, sizeof(int), 1, image_file);
    }

    // meta 信息占用四个扇区，多余的区域用0填充
    int meta_size = num * (1 + 1 + 64 + 4 + 4);
    int defsize = 4096 - meta_size;
    fwrite((void*)&zero, sizeof(uint8_t), defsize, image_file);

    // Write file data.
    for (int i = 0; i < num; i++) {
        char* file_path = file_paths[i];
        int size = file_size[i];

        FILE* prog_file;
        if ((prog_file = fopen(file_path, "r")) == 0) {
            printf("open %s failed!", file_path);
            exit(1);
        }
        char* buffer = (char*)malloc(size);
        fread(buffer, 1, size, prog_file);

        // 将文件写入镜像
        fwrite(buffer, 1, size, image_file);

        // 剩余扇区用0填充
        fwrite(&zero, sizeof(uint8_t), (512 - (size % 512)), image_file);
        free(buffer);
        fclose(prog_file);
    }

    // Close.
    fclose(image_file);
}
