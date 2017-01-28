#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct item {
    char filename[13];
    uint32_t offset;
    uint32_t len;
};

int analyse(const char * filename, struct item *** output) {
    // Set up FD
    FILE *fd;
    fd = fopen(filename, "rb");
    
    // Read and decipher the indexcount
    uint32_t indexcount = 0;
    fread(&indexcount, 4, 1, fd);
    indexcount ^= 0x3BD7A59A;
    
    // Read the enctypted index
    uint8_t *indexe;
    indexe = malloc(indexcount * 17);
    fread(indexe, 17, indexcount, fd);
    
    // Decrypt the index
    uint8_t *index;
    index = malloc(indexcount * 17);
    for(int b = 0; b < indexcount * 17; b++){
        uint8_t tmp = indexe[b];
        tmp = tmp - 39;
        tmp = tmp ^ 0xA5;
        tmp = tmp - (27 + b);
        index[b] = tmp;
    }
    
    //    create array of items large enough to hold all indexed items
    //
    struct item **out_tmp;
    out_tmp = malloc(indexcount * sizeof(struct item *));
    
    //    indexcount times
    //
    for (int i = 0; i < indexcount; i++) {
        //    read 17 bytes from index[0+offset] in to temporary string
        //
        struct item *tmp_item;
        tmp_item = malloc(sizeof(struct item));
        uint8_t tmp[17];
        memcpy(tmp, &index[0+(i*17)], 17);
        memcpy(tmp_item->filename, &tmp[0], 13);
        memcpy(&tmp_item->offset, &tmp[13], 4);
        out_tmp[i] = tmp_item;
    }
    
    //    calculate the file lengths
    //
    for (int i = 0; i < indexcount-1; i++) {
        out_tmp[i]->len = out_tmp[i+1]->offset - out_tmp[i]->offset - 1;
    }


    free(index);
    free(indexe);
    fclose(fd);
    (*output) = out_tmp;
    return indexcount;
}

int extract(char *filename, char *destination) {
    FILE *src;
    src = fopen(filename, "rb");
    
    struct item **items;
    int itemc = 0;
    
    itemc = analyse(filename, &items);
    
    for(int i = 0; i < itemc - 1; i++){
        printf("Extracting: %s\n", items[i]->filename);
        char *outpath;
        outpath = malloc(255);
        memset(outpath, 0, 255);
        sprintf(outpath, "%s/%s", destination ,items[i]->filename);
        fflush(stdout);
        
        FILE *out;
        out = fopen(outpath, "wb");
        if (out == NULL) {
            printf("Fail\n");
            return 1;
        }
        
        fseek(src, items[i]->offset, SEEK_SET);
        uint8_t *buffer;
        buffer = malloc(items[i]->len);
        fread(buffer, 1, items[i]->len, src);
        fwrite(buffer, 1, items[i]->len, out);
        free(outpath);
        free(buffer);
        fclose(out);
    }
    
    
    fclose(src);
    return 0;
}

int main(int argc, const char * argv[]) {

     extract("/Path/to/.fil", "/extraction/dir");
    
//    struct item **items;
//    int itemc = 0;
//    
//    itemc = analyse("/Path/to/.fil", &items);
//    
//    for(int i = 0; i < itemc - 1; i++){
//        printf("%s\n", items[i]->filename);
//    }
//    
    
    
    return 0;
}
