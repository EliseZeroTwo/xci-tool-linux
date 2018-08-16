#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include "xci2.h"

int main ()
{
  //Initialize variables used throughout dump
  FILE *file = fopen("game.xci", "rb");
  if (file == NULL) {
    fprintf(stderr, "Cant open game.xci...");
    return 0;
  }
  // Seek to the end to get length of file
  fseek(file, 0, SEEK_END);
  unsigned int length = ftell(file);
  xci_ctx_t xcifile;
  memset(&xcifile, 0, sizeof(xcifile));
  xcifile.file = file;
  fseek(file, 0, SEEK_SET);
  xciprocess(&xcifile);
  fclose(file);
  return 0;
}

void xciprocess(xci_ctx_t *xci) {
  if (xci->file == NULL) {
    fprintf(stderr, "FiLe CoRruPT or smth we just cant read it :(\nexiting...\n");
    return;
  }
  fseek(xci->file, 0, SEEK_SET);
  if(fread(&(xci->header), 1, 0x200, xci->file) != 0x200){
    fprintf(stderr, "XCI header was corrupt\n");
    return;
  }
  if(strncmp((char*)&(xci->header.magic), "HEAD", 4)) {
    fprintf(stderr, "xci header corrupt. magic was wrong");
    return;
  }
  xci->partition_ctx.offset = 0xF000;
  xci->partition_ctx.file = xci->file;
  xci->partition_ctx.name = "root";
  hfs0_header_t header;
  fseek(xci->file, 0xF000, SEEK_SET);
  fread(&header, 1, sizeof(header), xci->file);
  fseek(xci->file, 0, SEEK_SET);
  mkdir("./output", 0755);
  for (int i = 0; i < header.num_files; i++) {
    hfs0_file_entry_t * file = hfs0_get_file_entry(&header, i);
    if(i == 0){

        xci->update_ctx.file = xci->file;
        xci->update_ctx.name = "update";
        xci->update_ctx.offset = 0;
        #if defined(_WIN32)
          _mkdir("./output/update");
        #else
          mkdir("./output/update", 0755);
        #endif
        printf("made: %s\n", xci->update_ctx.name);
        hfs0process(&(xci->update_ctx));
      }
       else if (i == 1)
      {
        fseek(xci->file, 0xF050, SEEK_SET);
        fread(&(xci->normal_ctx.offset), 1, 0x8, xci->file);
        xci->normal_ctx.file = xci->file;
        xci->normal_ctx.name = "normal";
        // xci->normal_ctx.offset = file->offset;
        #if defined(_WIN32)
          _mkdir("./output/normal");
        #else
          mkdir("./output/normal", 0755);
        #endif
        printf("made: %s\n", xci->normal_ctx.name);
        hfs0process(&(xci->normal_ctx));
      }
       else if (i == 2)
      {
        fseek(xci->file, 0xF090, SEEK_SET);
        fread(&(xci->secure_ctx.offset), 1, sizeof(xci->secure_ctx.offset), xci->file);
        xci->secure_ctx.file = xci->file;
        xci->secure_ctx.name = "secure";
        // xci->secure_ctx.offset = file->offset;
        #if defined(_WIN32)
          _mkdir("./output/secure");
        #else
          mkdir("./output/secure", 0755);
        #endif
        printf("made: %s\n", xci->secure_ctx.name);
        hfs0process(&(xci->secure_ctx));
      }
       else if (i == 3)
      {
        fseek(xci->file, 0xF0D0, SEEK_SET);
        fread(&(xci->logo_ctx.offset), 1, sizeof(xci->logo_ctx.offset), xci->file);
        xci->logo_ctx.file = xci->file;
        xci->logo_ctx.name = "logo";
        // xci->logo_ctx.offset = file->offset;
        #if defined(_WIN32)
          _mkdir("./output/logo");
        #else
          mkdir("./output/logo", 0755);
        #endif
        printf("made: %s\n", xci->logo_ctx.name);
        hfs0process(&(xci->logo_ctx));
      } else {
        printf("out of int in for loop??? (Check line 96)\n");
      }
    }
  }

void hfs0process (hfs0_ctx_t *xci) {
  hfs0_header_t header;
  fseek(xci->file, 0xF200, SEEK_SET);
  fseek(xci->file, xci->offset, SEEK_CUR);
  fread(&header, 1, sizeof(header), xci->file);
  if(strncmp((char*)(&header.magic), "HFS0", 4)) {
    fprintf(stderr, "HSF0 header corrupt but at offset of %lu\n", (xci->offset+0xF200));
    return;
  }
  fseek(xci->file, 0xF200, SEEK_SET);
  fseek(xci->file, xci->offset, SEEK_CUR);
  uint64_t headersize = hfs0_get_header_size(&header);
  xci->header = malloc(headersize);

  if(fread(xci->header, 1, headersize, xci->file) != headersize) {
    fprintf(stderr, "header corrupt or something idk\n");
    return;
  }/*
  xci->header->magic = header.magic;
  xci->header->num_files = header.num_files;
  xci->header->string_table_size = header.string_table_size;
  xci->header->reserved = header.reserved;*/
  printf("starting save %s\n", xci->name);
  hfs0have(xci);
}


void hfs0have (hfs0_ctx_t *xci) {
  fseek(xci->file, 0xF200, SEEK_SET);
  fseek(xci->file, xci->offset, SEEK_CUR);
  uint32_t *offsets = (uint32_t*) malloc(xci->header->num_files * sizeof(uint32_t));
  for (int i = 0; i < xci->header->num_files; i++) {
    fseek(xci->file, 0x10, SEEK_CUR);
    fread(&(offsets[i]), 1, 0x4, xci->file);
    fseek(xci->file, xci->offset, SEEK_SET);
    fseek(xci->file, (i+1)*0x40, SEEK_CUR);
    printf("going through files...%d\n", i);
  }
  printf(" filename %s\n", hfs0_get_file_name(xci->header, 1));
  for (int i = 0; i < xci->header->num_files; i++) {
    hfs0_file_entry_t * file = hfs0_get_file_entry(xci->header, i);
    fseek(xci->file, 0xF200, SEEK_SET);
    fseek(xci->file, xci->offset, SEEK_CUR);
    fseek(xci->file, 0x10, SEEK_CUR);
    fseek(xci->file, xci->header->num_files*0x40, SEEK_CUR);
    fseek(xci->file, xci->header->string_table_size, SEEK_CUR);
    fseek(xci->file, file->offset, SEEK_CUR);
    printf("file offset: %lu\n", file->offset);
    char *output;
    output = malloc(file->size);
    fread(output, 1, file->size, xci->file);
    char filedestination[0x40];
    char filen[0x20];
    fseek(xci->file, xci->offset+0x10, SEEK_SET);
    fseek(xci->file, (xci->header->num_files*0x40)+file->string_table_offset, SEEK_CUR);
    fread(filen, 1, 0x30, xci->file);
    printf("Saving %s\n", hfs0_get_file_name(xci->header, i));
    char * new_str ;
    if((new_str = malloc(strlen("./output/")+strlen(xci->name)+strlen(hfs0_get_file_name(xci->header, i))+2)) != NULL){
        new_str[0] = '\0';   // ensures the memory is an empty string
        strcat(new_str,"./output/");
        strcat(new_str,xci->name);
        strcat(new_str,"/");
        strcat(new_str,hfs0_get_file_name(xci->header, i));
    } else {
        fprintf(stderr,"malloc failed!\n");
        return;
    }
    FILE * outputfile = fopen(new_str, "wb");
    if (outputfile == NULL) {
      fprintf(stderr, "Output file null...\n exiting...\n");
    }
    fwrite(output , 1 , file->size , outputfile);
    fclose(outputfile);
  }
}
