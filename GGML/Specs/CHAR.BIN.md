# <CHAR>.BIN

## Overall file structure
```
+---------------------+
|   4-byte Header     |
+---------------------+
|  Objects Array      |
|                     |
+---------------------+
|  Metadata Array     |
+---------------------+
|  Character Script   |
+---------------------+
|     CLUT Arrays     |
+---------------------+
| Sprite & Hitbox     |
|    - Collrects      |
|    - Scale & Type   |
|    - Image Size     |
|    - Compressed     |
|      Data Length    |
|    - Padding        |
|  Compressed Data    |
|    - Algorithm      |
|    - Loop Controls  |
|    - Offsets        |
+---------------------+
```

- The 4-byte header describes the length of the "objects array"
- The objects array is an array of 4-byte words
- Each member in the objects array describes the length, in bytes, of every other "object" in the file

## Metadata array

- First ushort in this chunk describes the size of an array of ushorts that follows
- Appears to be followed by two bytes of padding, which is 0x55 0x55 in most cases
- Unsure what the data in this array means, but in char.bin files, it seems to only contain one element
- This element appears to correspond to the number of sprite/hitbox objects in the file

## Character script
The "script" for the character contains a sequence of opcodes which dictate the timing of animation and frame data.
The structs in the Unity port of GGML seem to provide an interpretation of this data. Further investigation of this data
is pending.

## CLUT Arrays
These contain CLUTs used for the contained image data. Each character seems to contain five of these CLUT chunks, likely for each
palette. Each one is 32 bytes long, containing 16 ushorts, one for each color. The first ushort in the array is always overwritten to 0x0000,
which is used for transparency. It is unclear why this is done, maybe to permit a debug mode which shows the full bounds of character sprites?
After this, the MSB of the remaining 15 entries in the CLUT is set (equivalent to ORing with 0x8000). This ensures that none of the pixels are
transparent.

## Sprite and Hitbox data

- 'collrects' are an eight-byte long structure for defining a rectangle of a hitbox
- Image data is compressed with one of two algorithms, determined by the first byte in the compressed data
- Image dimensions are given in 4bpp units. This needs to be mangled a bit to get converted to equivalent 16bpp units.
```
  if (width % 8) {
    int diff = width % 8;
    width += diff;
    img_len += diff * height * 2;
  }
  width /= 4;
```
Seems to do the job

```
+------------------------------------------------+
| Sprite and Associated Hitbox                   |
|                                                |
| +--------------------------------------------+ |
| | Number of Hitbox Rectangles (4-byte word)  | |
| +--------------------------------------------+ |
| | Array of 8 bytes (Hitbox Data):            | |
| |    struct {                                | |
| |      short pos1, pos2;                     | |
| |      unsigned char x_scale, y_scale;       | |
| |      unsigned short type;                  | |
| |    }                                       | |
| +--------------------------------------------+ |
| | Image Dimensions Struct:                   | |
| |    struct {                                | |
| |      unsigned short x, y, width, height;   | |
| |    }                                       | |
| +--------------------------------------------+ |
| | Length of Compressed Data (2-byte halfword)| |
| +--------------------------------------------+ |
| | Padding (2-byte halfword, usually 0x8000)  | |
| +--------------------------------------------+ |
| | Compressed Data:                           | |
| |    - Compression Algorithm (2-byte ushort) | |
| |    - Loop Control Bytes (2 bytes)          | |
| |    - Decompression Offsets (2 shorts)      | |
| +--------------------------------------------+ |
+------------------------------------------------+
```