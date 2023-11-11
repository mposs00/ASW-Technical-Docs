#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static bool inline get_bit(uint32_t *bit_counter, uint32_t *cur_word,
                           uint8_t **cur_ptr) {
  bool result = *cur_word & 0x01;
  if (*bit_counter == 0) {
    *cur_word = *(uint32_t *)(*cur_ptr);
    *cur_ptr += sizeof(uint32_t);
    *bit_counter = 0x20;
    result = *cur_word & 0x01;
  }
  *cur_word = *cur_word >> 1;
  *bit_counter = *bit_counter - 1;
  return result;
}

// WARNING: This function is basically a 1:1 recreation of the MIPS assembly.
// It is not readable. I don't really know how it works. It's some kind of
// variable-length bit coding. The behavior at each leaf node of the binary tree
// is strange and would take more effort to understand than I care to put in
// right now.
//
// Here be Dragons
void decode_sprite(uint8_t *encoded_sprite, uint8_t *dest) {
  uint32_t a1 = 0;
  uint32_t a2 = 0;
  uint32_t a6 = 0;
  uint32_t a7 = 0;
  uint32_t cur_word = 0;
  uint32_t bit_counter = 0;
  uint32_t t4 = 0;
  uint32_t t5 = 0;
  uint32_t t9 = 0;
  uint32_t t6;
  uint32_t t8 = 0;
  uint32_t v0 = 0;
  uint32_t v1 = 0;
  uint32_t s1 = 0;
  uint32_t s2 = 0;
  uint32_t a0 = 0;

  if (*(uint8_t *)encoded_sprite) {

  } else {
    uint8_t *cur_ptr = encoded_sprite + 8;

    uint8_t s6 = *(uint8_t *)(encoded_sprite + 2) >> 2;
    uint8_t s4 = *(uint8_t *)(encoded_sprite + 3);
    uint8_t *t7 = encoded_sprite + *(uint16_t *)(encoded_sprite + 4);
    uint8_t *s3 = encoded_sprite + *(uint16_t *)(encoded_sprite + 6);
    uint8_t s5 = s6 << 2;

    while (s4 != 0) {
      a0 = s6 << 2;

      while (a0 != 0) {
        if (t9 == 0) {
          if (get_bit(&bit_counter, &cur_word, &cur_ptr) == 0) {
            if (get_bit(&bit_counter, &cur_word, &cur_ptr) == 0) {
              if (get_bit(&bit_counter, &cur_word, &cur_ptr) == 1) {
                t8 = t6 & 0x0F;
                if (a7 == 0) {
                  t6 = *(uint32_t *)t7;
                  t7 += sizeof(uint32_t);
                  a7 = 8;
                  t8 = t6 & 0x0F;
                }
                t6 = t6 >> 4;
                a7--;
              }
              if (get_bit(&bit_counter, &cur_word, &cur_ptr) == 1) {
                a1 = t6 & 0x0F;
                if (a7 == 0) {
                  t6 = *(uint32_t *)t7;
                  t7 += sizeof(uint32_t);
                  a7 = 8;
                  a1 = t6 & 0x0F;
                }
                t6 = t6 >> 4;
                a7--;
              }
              t9 = t6 & 0x0F;
              if (a7 == 0) {
                t6 = *(uint32_t *)t7;
                t7 += sizeof(uint32_t);
                a7 = 8;
                t9 = t6 & 0x0F;
              }
              t6 = t6 >> 4;
              a7--;
              if ((t9 & 0x01) == 0) {
                v1 = t8 << 4;
              } else {
                v1 = a1 << 4;
              }
              v0 = v1 | a1;
              if ((t9 & 2) == 0) {
                v0 = v1 | t8;
              }
              a2 = v0 << 24;
              v1 = a1 << 4;
              if ((t9 & 4) == 0) {
                v1 = t8 << 4;
              }
              v0 = v1 | a1;
              if ((t9 & 8) == 0) {
                v0 = v1 | t8;
              }
              a6 = v0 << 24;
              t4 |= a6;
              t5 |= a2;
              t9 = 0;
            } else {
              if (get_bit(&bit_counter, &cur_word, &cur_ptr) == 0) {
                if (get_bit(&bit_counter, &cur_word, &cur_ptr) == 0) {
                  v1 = t8 << 24;
                  v0 = t8 << 28;
                } else {
                  v1 = a1 << 24;
                  v0 = a1 << 28;
                }

              } else {
                t8 = t6 & 0xF;
                if (a7 == 0) {
                  t6 = *(uint32_t *)t7;
                  t7 += sizeof(uint32_t);
                  a7 = 8;
                  t8 = t6 & 0xF;
                }
                t6 = t6 >> 4;
                a7--;
                v1 = t8 << 24;
                v0 = t8 << 28;
              }
              a2 = v1 | v0;
              a6 = a2;
              t4 |= a2;
              t5 |= a2;
            }
          } else {
            if (get_bit(&bit_counter, &cur_word, &cur_ptr) == 0) {
              if (get_bit(&bit_counter, &cur_word, &cur_ptr) == 0) {
                t4 |= a6;
                t5 |= a2;
              } else {
                if (a7 == 0) {
                  t6 = *(uint32_t *)t7;
                  t7 += sizeof(uint32_t);
                  a7 = 8;
                }
                t4 |= a6;
                t5 |= a2;
                v0 = t6 & 0xF;
                t9 = v0 + 2;
                t6 = t6 >> 4;
                a7--;
              }
            } else {
              a6 = s1 << 24;
              if (s2 == 0) {
                s1 = *(uint32_t *)s3;
                s3 += sizeof(uint32_t);
                s2 = 2;
                a6 = s1 << 24;
              }
              v0 = s1 & 0xFF00;
              a2 = v0 << 16;
              s1 = s1 >> 16;
              s2--;
              t4 |= a6;
              t5 |= a2;
              t8 = a6 >> 28;
              a1 = a2 >> 28;
            }
          }
        } else {
          t4 |= a6;
          t5 |= a2;
          t9--;
        }

        if ((a0 & 3) != 1) {
          t4 = t4 >> 8;
          t5 = t5 >> 8;
        } else {
          *(uint32_t *)(dest) = t4;
          *(uint32_t *)(dest + s5) = t5;
          t4 = 0;
          t5 = 0;
          dest += 4;
        }

        a0--;
      }

      s4--;
      dest += s5;
    }
  }
}

void write_tim(char *filename, uint8_t *img, size_t img_len, uint8_t *clut,
               size_t clut_len, uint16_t width, uint16_t height, uint16_t x,
               uint16_t y) {
  // For some reason, if width isn't a multiple of 8, things are off by one?
  if (width % 8) {
    int diff = width % 8;
    width += diff;
    img_len += diff * height * 2;
  }
  img_len /= 4;

  // TIM size = header + clut + image
  size_t tim_header_len = 2 * sizeof(uint32_t);
  size_t img_header_len = 6 * sizeof(uint16_t);
  size_t tim_size = (tim_header_len + clut_len + img_header_len + img_len);
  uint8_t *tim_buf = calloc(0x01, tim_size);

  // TIM Header
  *(uint32_t *)(tim_buf) = 0x00000010;
  *(uint32_t *)(tim_buf + sizeof(uint32_t)) = 0x00000008;

  // CLUT
  memcpy(tim_buf + tim_header_len, clut, clut_len);

  // Image
  uint16_t img_header[6] = {(img_len + img_header_len) & 0xFFFF,
                            ((img_len + img_header_len) & 0xFFFF0000) >> 16,
                            x,
                            y,
                            width / 4,
                            height};
  memcpy(tim_buf + tim_header_len + clut_len, img_header, img_header_len);
  memcpy(tim_buf + tim_header_len + clut_len + img_header_len, img, img_len);

  int tim_fd = open(filename, O_RDWR | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH);
  write(tim_fd, tim_buf, tim_size);
  close(tim_fd);
  free(tim_buf);
}

int main() {
  int fd;
  struct stat sb;

  fd = open("./SL.BIN", O_RDONLY);
  fstat(fd, &sb);

  uint8_t *filebuf = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (filebuf == MAP_FAILED) {
    fprintf(stderr, "mmap() failed...\n");
    perror("mmap");
    exit(EXIT_FAILURE);
  }

  int cur_file_offset = 0;

  // First word is number of objects
  uint32_t num_objs = *(uint32_t *)(filebuf + cur_file_offset);
  cur_file_offset += sizeof(uint32_t);
  // Followed by an array of object lengths. Allocate space for that array.
  int32_t *obj_sizes = malloc(num_objs * sizeof(uint32_t));
  // And then copy the sizes in...
  for (int i = 0; i < num_objs; i++) {
    obj_sizes[i] = *(uint32_t *)(filebuf + cur_file_offset);
    cur_file_offset += sizeof(uint32_t);
  }

  char filename[0x40];
  uint8_t *clut = calloc(0x01, 0x2C);
  bool clut_loaded = false;

  printf("File contains %d objects\n", num_objs);
  for (int i = 0; i < num_objs; i++) {
    bool is_sprite = false;
    printf(
        "------------------------------------\n\nOffset: %08X Obj size: %08X\n",
        cur_file_offset, obj_sizes[i]);

    if (i == 0) {
      printf("Type: File-wide collrect info? (unk)\n");
    } else if (i == 1) {
      printf("Type: Character script\n");
    } else if (obj_sizes[i] == 0x20) {
      printf("Type: Likely CLUT based on filesize\n");
      if (!clut_loaded) {
        *(uint32_t *)clut = 0x0000002C; // CLUT size
        *(uint32_t *)(clut + sizeof(uint32_t)) =
            0x00000000; // CLUT x/y loc in framebuffer
        *(uint32_t *)(clut + (2 * sizeof(uint32_t))) =
            0x00010010; // CLUT width/height. technically 2 uint16_t in little
                        // endian
        for (int i = 0; i < 0x20; i += sizeof(uint32_t)) {
          int offset = (3 * sizeof(uint32_t)) + i;
          uint32_t clut_data =
              (*(uint32_t *)(filebuf + cur_file_offset + i)) | 0x80008000;
          if (i == 0) {
            clut_data &= 0xFFFF0000;
          }
          *(uint32_t *)(clut + offset) = clut_data;
        }

        printf("Loaded CLUT\n");
        clut_loaded = true;
      }
    } else {
      printf("Type: Likely sprite/collrect data\n");
      is_sprite = true;
    }

    if (is_sprite) {
      int sprite_offset = 0;

      uint32_t num_collrects =
          *(uint32_t *)(filebuf + cur_file_offset + sprite_offset);
      sprite_offset += sizeof(uint32_t);
      sprite_offset += (8 * num_collrects);

      uint16_t x, y, w, h;
      x = *(uint16_t *)(filebuf + cur_file_offset + sprite_offset);
      sprite_offset += sizeof(uint16_t);
      y = *(uint16_t *)(filebuf + cur_file_offset + sprite_offset);
      sprite_offset += sizeof(uint16_t);
      w = *(uint16_t *)(filebuf + cur_file_offset + sprite_offset);
      sprite_offset += sizeof(uint16_t);
      h = *(uint16_t *)(filebuf + cur_file_offset + sprite_offset);
      sprite_offset += sizeof(uint16_t);

      uint32_t sprite_data_len =
          *(uint32_t *)(filebuf + cur_file_offset + sprite_offset);
      sprite_data_len &= 0x7FFFFFFF;
      sprite_offset += sizeof(uint32_t);

      printf("Sprite has %d collrect%s\nX: %d Y: %d\nWidth: %d Height: "
             "%d\nCompressed data length: %08X\n",
             num_collrects, num_collrects > 1 ? "s" : "", x, y, w, h,
             sprite_data_len);
      int img_size = w * h * 2;
      uint8_t *dest = calloc(0x01, img_size);
      decode_sprite((uint8_t *)(filebuf + cur_file_offset + sprite_offset),
                    dest);
      printf("Decoded sprite (%04X bytes)\n", img_size);
      snprintf(filename, 0x40, "./output/%08X.TIM", cur_file_offset);
      write_tim(filename, dest, img_size, clut, 0x2C, w, h, x, y);
      free(dest);
    }

    cur_file_offset += obj_sizes[i];
  }

  free(clut);
  return 0;
}