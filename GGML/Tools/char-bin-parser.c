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
  // Debug counters used for stats
  /*
  int total_codes = 0;
  int b11 = 0;
  int b011 = 0;
  int b0101 = 0;
  int b0100 = 0;
  int b101 = 0;
  int b100 = 0;
  int b00xx = 0;
  */
  

  uint8_t *orig_dest = dest;

  uint32_t a2 = 0;
  uint32_t a6 = 0;
  uint32_t a7 = 0;
  uint32_t cur_word = 0;
  uint32_t bit_counter = 0;
  uint8_t *cur_ptr = encoded_sprite + 8;
  uint32_t t9 = 0;
  uint32_t t6;
  uint32_t v0 = 0;
  uint32_t v1 = 0;
  uint32_t s1 = 0;
  uint32_t s2 = 0;

  uint32_t s0 = 0;

  uint32_t even_line = 0;
  uint32_t odd_line = 0;
  uint32_t stride = 0;
  uint32_t cur_pxl = 0;
  uint32_t line_pairs_remaining = 0;
  uint32_t bpp4_width = 0;

  if (*(uint8_t *)encoded_sprite) {
    uint32_t s6 = 0;
    uint32_t s4 = 0;
    uint32_t t4 = 0;
    uint32_t t5 = 0;
    uint32_t s3 = *(uint8_t *)(encoded_sprite + 3);
    uint32_t s5 = *(uint8_t *)(encoded_sprite + 2) >> 2;
    uint8_t *a0 = *(uint16_t *)(encoded_sprite + 6) + encoded_sprite;
    uint8_t *t8 = *(uint16_t *)(encoded_sprite + 4) + encoded_sprite;
    uint32_t a3 = 0;
    uint32_t a5 = 0;
    uint32_t a4 = 0;
    uint32_t t7 = 0;
    s4 = s5 << 2;
    while (s3 != 0) {
      t9 = s5 << 2;

      while (t9 != 0) {
        if (s1 == 0) {
          if (get_bit(&bit_counter, &cur_word, &cur_ptr) == 0) {
            if (get_bit(&bit_counter, &cur_word, &cur_ptr) == 0) {
              if (get_bit(&bit_counter, &cur_word, &cur_ptr) == 0) {
                a3 |= t4;
                a5 |= a4;
              } else {
                if (t5 == 0) {
                  t6 = *(uint32_t *)t8;
                  t8 += sizeof(uint32_t);
                  t5 = 8;
                }
                a3 |= t4;
                a5 |= a4;
                s1 += ((t6 & 0xF) + 2);
                t6 = t6 >> 4;
                t5--;
              }
            } else {
              if (get_bit(&bit_counter, &cur_word, &cur_ptr) == 0) {
                a4 = (t7 << 24) | (t7 << 28);
                t4 = a4;
                a3 |= a4;
                a5 |= a4;
              } else {
                t7 = t6 & 0xF;
                if (t5 == 0) {
                  t6 = *(uint32_t *)t8;
                  t8 += sizeof(uint32_t);
                  t5 = 8;
                  t7 = t6 & 0xF;
                }
                t6 = t6 >> 4;
                t5--;
                a4 = (t7 << 24) | (t7 << 28);
                t4 = a4;
                a3 |= a4;
                a5 |= a4;
              }
            }
          } else {
            t4 = s0 << 24;
            if (s2 == 0) {
              s0 = *(uint32_t *)a0;
              a0 += sizeof(uint32_t);
              t4 = s0 << 24;
            }
            a4 = (s0 & 0xFF00) << 16;
            s0 = s0 >> 16;
            s2 = s2 ^ 0x01;
            a3 |= t4;
            a5 |= a4;
            t7 = t4 >> 28;
          }
        } else {
          a3 |= t4;
          a5 |= a4;
          s1--;
        }

        if ((t9 & 3) != 1) {
          a3 = a3 >> 8;
          a5 = a5 >> 8;
        } else {
          *(uint32_t *)(dest) = a3;
          *(uint32_t *)(dest + s4) = a5;
          a3 = 0;
          a5 = 0;
          dest += sizeof(uint32_t);
        }
        t9--;
      }

      s3--;
      dest += s4;
    }
  } else {
    uint32_t a1 = 0;
    uint32_t t8 = 0;
    bpp4_width = *(uint8_t *)(encoded_sprite + 2);
    line_pairs_remaining = *(uint8_t *)(encoded_sprite + 3);
    uint8_t *t7 = encoded_sprite + *(uint16_t *)(encoded_sprite + 4);
    uint8_t *s3 = encoded_sprite + *(uint16_t *)(encoded_sprite + 6);
    stride = bpp4_width & 0xFC;

    while (line_pairs_remaining != 0) {
      for (cur_pxl = stride; cur_pxl != 0; cur_pxl--) {
        if (t9 == 0) {
          if (get_bit(&bit_counter, &cur_word, &cur_ptr) == 0) {
            if (get_bit(&bit_counter, &cur_word, &cur_ptr) == 0) {
              if (get_bit(&bit_counter, &cur_word, &cur_ptr) == 1) {
                // The following block here is an idiom that is repeated
                // throughout this algorithm. We read 4 bits at a time out of
                // the data pointed to by t7, and maintain a counter for when we
                // need to refresh this data. It is similar to the idiom for
                // reading the bitstream, but operating on nibbles rather than
                // bits.
                t8 = t6 & 0x0F;
                if (a7 == 0) {
                  t6 = *(uint32_t *)t7;
                  t7 += sizeof(uint32_t);
                  a7 = 8;
                  t8 = t6 & 0x0F;
                }
                t6 = t6 >> 4;
                a7--;

                // Taking a step back, the fact that this idiom appears in
                // isolation here is notable. This means that we are updating t8
                // with new data at this point in the decoding.
              }
              if (get_bit(&bit_counter, &cur_word, &cur_ptr) == 1) {
                // Here, the idiom occurs again, but with a1 as the destination
                // register. This is notable because this is the only place that
                // a1 is directly assigned with data dereferenced from the file.
                // a1 is assigned in one other place, with the result of a
                // shift.
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

              // t9 is used elsewhere to control RLE, but here it is used with
              // this conditional
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
              even_line |= a6;
              odd_line |= a2;
              // t9 is reset to zero here, so we know we will not see any RLE
              // skipping of decoding here.
              t9 = 0;

              //b00xx++;
              //total_codes++;
            } else {
              if (get_bit(&bit_counter, &cur_word, &cur_ptr) == 0) {
                if (get_bit(&bit_counter, &cur_word, &cur_ptr) == 0) {
                  v1 = t8 << 24;
                  v0 = t8 << 28;

                  //b0100++;
                } else {
                  v1 = a1 << 24;
                  v0 = a1 << 28;

                  //b0101++;
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

                //b011++;
              }
              a2 = v1 | v0;
              a6 = a2;
              // Even and Odd here get set to same value, which would suggest
              // 'square' of same color? Note how the value, a2, is ORed from
              // v1/v0, both of which are always formed by taking the same value
              // and shifting it by different amounts. t8 and a1 initial
              // assignment are always 4 bits at a time. These always end up in
              // the highest bits of a2. This branch is always encoding a total
              // of 4 pixels at a time, all the same color. Two next to
              // eachother, stacked on two others. The assignment of these bits
              // comes from the t7 region of the data. This suggests that t7
              // contains some kind of pixel data, and the variable-length data
              // instructs the game on how to lay that pixel data out into the
              // dest buffer.
              even_line |= a2;
              odd_line |= a2;

              //total_codes++;
            }
          } else {
            if (get_bit(&bit_counter, &cur_word, &cur_ptr) == 0) {
              if (get_bit(&bit_counter, &cur_word, &cur_ptr) == 0) {
                // Just repeat the last decode. This is a form of shorter RLE.
                even_line |= a6;
                odd_line |= a2;

                //b100++;
                //total_codes++;
              } else {
                // The familiar idiom occurs here, and the result is used to
                // update t9. t9 is used as a conditional at the start of the
                // loop. If t9 is nonzero, a6 and a2 are ORed into the line
                // buffers, and t9 is decremented. This means that leaving the
                // current iteration with any nonzero t9 value has the same
                // effect on the line buffers, but the writeback to dest occurs
                // anyways. This means that by setting t9 to a nonzero value,
                // some repeating value in the line buffers can be repeatedly
                // written (the number of times is controlled by t9). This makes
                // t9 a sort of RLE control.
                if (a7 == 0) {
                  t6 = *(uint32_t *)t7;
                  t7 += sizeof(uint32_t);
                  a7 = 8;
                }
                even_line |= a6;
                odd_line |= a2;
                v0 = t6 & 0xF;
                // The value of t9 here is how many loop iterations we skip.
                // Each loop iteration corresponds to two pixels being decoded,
                // so at least the next 4 pixels are skipped. No more than 0x11
                // iterations of the loop can be skipped in this manner.
                // Therefore, this mechanism can encode runs of lengths 6 to 34
                // pixels (0x03 to 0x11 bytes) across a single stride. Note that
                // the run MUST occur on two adjacent lines (even and odd), and
                // cannot be longer than the stride.
                t9 = v0 + 2;
                t6 = t6 >> 4;
                a7--;

                //b101++;
                //total_codes++;
              }
            } else {
              // This entire section needs more analysis. It breaks a lot of
              // patterns found elsewhere in this algorithm.

              // The idiom changes here, as we deref s3 instead of the usual t7.
              // The change in the increment of s2 as well as the following bit
              // manipulation makes it clear: the data here represents two bits
              // to be interpreted rather than 4. It seems like this data is
              // always put into the 2 MSBs of the line buffer, which should
              // mean that the data here always represents a 4bpp index in the
              // form 0bxx00
              a6 = s1 << 24;
              if (s2 == 0) {
                s1 = *(uint32_t *)s3;
                s3 += sizeof(uint32_t);
                s2 = 2;
                a6 = s1 << 24;
              }
              v0 = s1 & 0xFF00;
              // We take the upper byte of the lower short here, so after the
              // left shift, we are in the most significant 8 bits of our word
              // when we OR with the line buffer. As a matter of fact, only two
              // bits are preserved here. Why do we only keep 2 bits of what
              // should be a 4bpp color index?
              a2 = v0 << 16;
              // We dispose of half the bits in s1 here!
              s1 = s1 >> 16;
              s2--;
              even_line |= a6;
              odd_line |= a2;
              // Wtf is this assignment?
              t8 = a6 >> 28;
              // The only other assignment of a1. t8 is the most commonly used
              // register to populate the line buffers, but a1 is used here and
              // there.
              a1 = a2 >> 28;

              //b11++;
              //total_codes++;
            }
          }
        } else {
          //printf("cur_pxl = %08X t9 = %08X\n", cur_pxl, t9);
          // We have encoded a run, so just continue to OR in the last decoded
          // values (usually, but not always 0x00)
          even_line |= a6;
          odd_line |= a2;
          t9--;
        }

        // Due to the ANDing (shifting in the MIPS asm), cur_pxl always starts
        // out ending with 0b00 This means that with each iteration, cur_pxl & 3
        // will go 0 > 3 > 2 > 1
        //
        // Therefore, we only write the line buffer regs to the destination
        // every 4 loops. This makes sense. 4bpp * 4 loops * up to 2 pixels
        // written per iteration = 32 bits, the width of our register. We shift
        // by 8 otherwise, to accomodate the next 2 pixels in each line buffer
        // register during the following iteration.
        //
        // Note that this means that all newly decoded pixels should be ORed
        // into the line buffer registers at the most significant 8 bits. This
        // explains all of the left shifts by 28/24, getting ORed in.
        if ((cur_pxl & 3) != 1) {
          even_line = even_line >> 8;
          odd_line = odd_line >> 8;
        } else {
          *(uint32_t *)(dest) = even_line;         // Even line
          *(uint32_t *)(dest + stride) = odd_line; // Odd line
          //printf(
          //    "Writes at dest + %08lX (%08X) and dest + %08lX + %08X (%08X)\n",
          //    dest - orig_dest, even_line, dest - orig_dest, stride, odd_line);
          even_line = 0;
          odd_line = 0;
          dest += 4;
        }
      }

      line_pairs_remaining--;
      dest += stride;
    }
  }

  //printf("Image contained a total of %d variable-length codes\n", total_codes);
  //printf("b11: %d\nb011: %d\nb0101: %d\nb0100: %d\nb101: %d\nb100: %d\nb00xx: %d\n", b11, b011, b0101, b0100, b101, b100, b00xx);
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

int main(int argc, char **argv) {
  int fd;
  struct stat sb;

  if (argc < 3) {
    printf("Usage: ./rip [filename] [output dir]\n");
    exit(1);
  }

  fd = open(argv[1], O_RDONLY);
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
      snprintf(filename, 0x40, "%s/%08X.TIM", argv[2], cur_file_offset);
      write_tim(filename, dest, img_size, clut, 0x2C, w, h, x, y);
      free(dest);
    }

    cur_file_offset += obj_sizes[i];
  }

  free(clut);
  return 0;
}