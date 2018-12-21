/** Chloe Kaubisch **/

#include "bfd.h"
#include<string.h>
#include<stdlib.h>
#include<stdio.h>

/** Find sections contained in a binary and print them to standard output **/

/**  int hex_to_char(int, char*)

     Takes an integer, transforms it to an array
     of chars representing its hexadecimal value,
     and appends it to the buffer buf
**/

int hex_to_char(int num, char* buf){

  char ret[15];
  int x = 0;
  int val, mod;
  int div = 16;

  /** If num is zero, simply return "00" **/
  if (num == 0){
    strcat(buf, "00");
    return 2;
  }

  /** Find largest power of 16 which **/
  while ((num/div) >= 16){
    div *= 16;
  }

  /** Get next highest-order byte and calculate char equivalent **/
  while (div >= 1) {
    val = num/div;
    mod = num%div;
    div /= 16;
    if (val < 10){
      ret[x++] = '0' + val;
    } else {
        switch(val) {
      case 10:
        ret[x++] = 'a';
        break;
      case 11:
        ret[x++] = 'b';
        break;
      case 12:
        ret[x++] = 'c';
        break;
      case 13:
        ret[x++] = 'd';
	break;
      case 14:
        ret[x++] = 'e';
        break;
      case 15:
        ret[x++] = 'f';
        break;
      }
    }
    num = mod;
  }
  strcat(buf, ret);
  return x;
}


int write_stdout(const void *buf, int total){
  if (write(1, buf, total) != total) {
    write(2, "There was an error writing to standard out\n", 44);
    return -1;
  }
}


void section_func(bfd *abfd, asection *sect, void *obj){

  char buffer[100] = "";
  int index, x, i;
  x = 0;
  i = 0;
  char em = ' ';

  /** Get section information **/
  const char *nameptr = (*sect).name;
  int name_size = sizeof(nameptr);
  int vma = (*sect).vma;
  int cookedsize = (*sect).size;
  int rawsize = (*sect).rawsize;

  /** TEST **/
  int filepos = (*sect).filepos;

  if (rawsize == 0){
    rawsize = cookedsize;
  }

  /** Create buffer to be written to output**/

  index = (*sect).index;

  char ind = '0' + index;
  buffer[x++] = em;
  buffer[x++] = em;
  buffer[x++] = ind;
  buffer[x++] = em;
  buffer[x++] = em;

  while (nameptr[i] != '\0'){
    buffer[x++] = nameptr[i++];
  }
  buffer[x++] = em;
  x += hex_to_char(vma, buffer);
  strcat(buffer, "       ");
  x += 3 + hex_to_char(cookedsize, buffer);
  strcat(buffer, "       ");
  x += 4 + hex_to_char(rawsize, buffer);
  strcat(buffer, "       ");
  x += 3 + hex_to_char(filepos, buffer);
  buffer[x++] = '\n';
  write_stdout(buffer, x);
}


void *getsectioninfo(const char *filename, const char *target) {
  bfd *abfd;
  abfd = bfd_openr(filename, target);
  int bool = bfd_check_format(abfd, bfd_object);
  if (!bool) {
    bfd_error_type errtype = bfd_get_error();
    const char *err = bfd_errmsg(errtype);
    bfd_perror(err);
  }

  void (*fn)(bfd *abfd, asection *sect, void *obj) = section_func;
  void *ptr;

  //char header[100] = "";
  //strcat(header, "INDEX   NAME   VMA   COOKEDSIZE   RAWSIZE   POSITION\n");
  //write_stdout(header, 60);
  printf("INDEX   NAME   VMA   COOKEDSIZE   RAWSIZE   POSITION\n");
  bfd_map_over_sections(abfd, section_func, ptr);
}


