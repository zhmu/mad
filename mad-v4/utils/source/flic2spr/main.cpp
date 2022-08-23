// FLIC2SPR
// FLIC to MAD sprites util
// Copyright (c) 2000, The MAD crew

// Util version
#define HIVER 1
#define LOVER 0
// and other definitions
#define TRUE 1
#define FALSE 0
// and FLI definitions
// magic numbers...
#define FLI_MAGIC 0xAF11        // Standard FLI files
#define FLC_MAGIC 0xAF12        // Standard FLC files (8 bit depth)
#define FLICNOPAL_MAGIC 0xAF44  // FLIC with no pallete (1, 15, 16 or 24 bit depth)
#define FLICCOMP_MAGIC 0xAF30   // FLIC that use Huffman or BTW
// chunk types...
#define FLI_FRAME 0xF1FA        // Frame chunk
#define FLI_PREFIX 0xF100       // Prefix chunk
#define FLI_COLOR256 4          // Compressed color map (RGB range 0-255)
#define FLI_SS2 7               // delta image, word oriented RLE
#define FLI_COLOR64 11          // Compressed color map (RGB range 0-64)
#define FLI_LC 12               /* Line compressed -- the most common type
                                   of compression for any but the first
                                   frame.  Describes the pixel difference
                                   from the previous frame. */
#define FLI_BLACK 13            /* Set whole screen to color 0 (only occurs
                                   on the first frame). */
#define FLI_BRUN 15             /* Bytewise run-length compression -- first
                                   frame only (usually) */
#define FLI_COPY 16             /* Indicates uncompressed w * h bytes soon
                                   to follow.  For those times when
                                   compression just doesn't work! */
#define FLI_PSTAMP 18           // A chunk with a small view of the img

// Header files
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

// Declarations
void parse_parameters(int argc, char *argv[]);
void convert(void);
void help(void);

// Global variables
char flifile[256];                 // input FLI file name
int verbmode = FALSE;              // Verbose mode?
int fromframe = -1;                // from frame (-1 if it's not indicated)
int toframe = -1;                  // to frame (-1 if it's not indicated)
FILE *log = stdout;                // log file (usually stdout)
signed short int trans_index = -1; // transparency palette index (-1 if it's not indicated)
signed short int trans_r = -1;     // transparency r component (-1 if it's not indicated)
signed short int trans_g = -1;     // transparency g component (-1 if it's not indicated)
signed short int trans_b = -1;     // transparency b component (-1 if it's not indicated)
signed short int trans_rgb = FALSE;// if used -tcr, -tcg or -tgb then it's TRUE

// main
int main(int argc, char *argv[]) {
  fprintf(log, "\nFLI2SPR util v%d.%d - MAD project\n\n", HIVER, LOVER);
  parse_parameters(argc, argv);
  convert();
  if (log != stdout) {
    fprintf(log, "\nLogging finished\n\n\n");
    fclose(log);
  }
  return 0;
}

// Help needed?
void help(void) {
  fprintf(log, "  Syntax:     FLIC2SPR <flicfile.ext> (options)\n\n");
  fprintf(log, "   Where the parameters between '<' and '>' are required\n");
  fprintf(log, "   and the ones between '(' and ')' are optionals\n\n");
  fprintf(log, "  Options:\n");
  fprintf(log, "   -v     verbose mode (a lot of extra info wich isn't really needed)\n");
  fprintf(log, "   -l     log mode (all the info generated is sent to the \"flic2spr.log\" file)\n");
  fprintf(log, "   -tcix  set the transparent color to the x palette index (0-255)\n");
  fprintf(log, "   -tcrx  set the transparent color component RED to the x value (0-255)\n");
  fprintf(log, "   -tcgx  set the transparent color component GREEN to the x value (0-255)\n");
  fprintf(log, "   -tcbx  set the transparent color component BLUE to the x value (0-255)\n");
  fprintf(log, "   -fx    start from frame x\n");
  fprintf(log, "   -tx    to frame x\n\n");
  exit(0);
}

// Are the params ok?
void parse_parameters(int argc, char *argv[]) {
  int logging = FALSE, i;
  if (argc < 2) {
    help();
    return;
  }
  strcpy(flifile, argv[1]);
  for (i = argc; i >= 3; i--) {
    if (strcmp(argv[i - 1], "-v") == 0) verbmode = TRUE;
    else if (strcmp(argv[i - 1], "-l") == 0) logging = TRUE;
    else if (strncmp(argv[i - 1], "-tcr", 4) == 0) {
      unsigned char tmpstr[16];
      char *tmpstr2;
      strcpy((char*)tmpstr, &argv[i - 1][4]);
      trans_r = strtol((char*)tmpstr, &tmpstr2, 10);
      if ((trans_r < 0) || (trans_r > 255)) help();
      if (tmpstr2[0] != 0) help();
      if (strlen((char*)tmpstr) == 0) help();
      trans_rgb = TRUE;
    }
    else if (strncmp(argv[i - 1], "-tcg", 4) == 0) {
      unsigned char tmpstr[16];
      char *tmpstr2;
      strcpy((char*)tmpstr, &argv[i - 1][4]);
      trans_g = strtol((char*)tmpstr, &tmpstr2, 10);
      if ((trans_g < 0) || (trans_g > 255)) help();
      if (tmpstr2[0] != 0) help();
      if (strlen((char*)tmpstr) == 0) help();
      trans_rgb = TRUE;
    }
    else if (strncmp(argv[i - 1], "-tcb", 4) == 0) {
      unsigned char tmpstr[16];
      char *tmpstr2;
      strcpy((char*)tmpstr, &argv[i - 1][4]);
      trans_b = strtol((char*)tmpstr, &tmpstr2, 10);
      if ((trans_b < 0) || (trans_b > 255)) help();
      if (tmpstr2[0] != 0) help();
      if (strlen((char*)tmpstr) == 0) help();
      trans_rgb = TRUE;
    }
    else if (strncmp(argv[i - 1], "-tci", 4) == 0) {
      unsigned char tmpstr[16];
      char *tmpstr2;
      strcpy((char*)tmpstr, &argv[i - 1][4]);
      trans_index = strtol((char*)tmpstr, &tmpstr2, 10);
      if ((trans_index < 0) || (trans_index > 255)) help();
      if (tmpstr2[0] != 0) help();
      if (strlen((char*)tmpstr) == 0) help();
    }
    else if (strncmp(argv[i - 1], "-f", 2) == 0) {
      unsigned char tmpstr[16];
      char *tmpstr2;
      strcpy((char*)tmpstr, &argv[i - 1][2]);
      fromframe = strtol((char*)tmpstr, &tmpstr2, 10);
      if (fromframe < 0) help();
      if (tmpstr2[0] != 0) help();
      if (strlen((char*)tmpstr) == 0) help();
    }
    else if (strncmp(argv[i - 1], "-t", 2) == 0) {
      unsigned char tmpstr[16];
      char *tmpstr2;
      strcpy((char*)tmpstr, &argv[i - 1][2]);
      toframe = strtol((char*)tmpstr, &tmpstr2, 10);
      if ((toframe < 0) || (toframe < fromframe)) help();
      if (tmpstr2[0] != 0) help();
      if (strlen((char*)tmpstr) == 0) help();
    }
    else help();
  }
  if (logging == TRUE) {
    time_t now;
    fprintf(log, "Logging enabled - saving all the info to the \"flic2spr.log\" file\n");
    log = fopen("flic2spr.log", "at");
    time(&now);
    fprintf(log, "FLIC2SPR util v%d.%d - MAD project\n", HIVER, LOVER);
    fprintf(log, "\nLogging started at (local time): %s\n", asctime(localtime(&now)));
    fprintf(log, "Parameters:\n");
    fprintf(log, " FLIC file: %s\n", flifile);
    fprintf(log, " Options:\n");
    fprintf(log, "  -l ");
    if (verbmode) fprintf(log, "-v ");
    if (trans_index != -1) fprintf(log, "-tci%d ", trans_index);
    if (trans_r != -1) fprintf(log, "-tcr%d ", trans_r);
    if (trans_g != -1) fprintf(log, "-tcg%d ", trans_g);
    if (trans_b != -1) fprintf(log, "-tcb%d ", trans_b);
    if (fromframe != -1) fprintf(log, "-f%d ", fromframe);
    if (toframe != -1) fprintf(log, "-t%d ", toframe);
    fprintf(log, "\n");
  }
}

// convert - The hard stuff
void convert(void) {

  // FLI header
  struct {
    unsigned long int size;       // Lenght of file (4 bytes)
    unsigned short int magic;     // AF11 or AF12
    unsigned short int frames;    // Number of frames (max of 4000)
    unsigned short int width;     // width (usually 320, but not an standard)
    unsigned short int height;    // height (usually 200, but not an standard)
    unsigned short int depth;     // depth (usually 8, but not an standard)
    char nothing[114];            // without use for this util
  } fliheader;
  unsigned char flipalette[256][3];

  // FLI frame chunk (and also FLI prefix chunk)
  struct {
    unsigned long int size;        // Chunk size
    unsigned short int type;       // Chunk type
    unsigned short int chunks;     // Number of chunks for the frame
    unsigned char nothing[8];      // unused
  } fliframechunk;

  // FLI chunk header
  struct {
    unsigned long int size;        // Chunk size
    unsigned short int type;       // Chunk type
  } flichunkheader;
  char *flichunk;

  // SPR header
  struct {
    unsigned long int magic;      // Magic number (SPR!)
    unsigned short int ver;       // File version
    unsigned short int height;    // height
    unsigned short int width;     // width
    unsigned long int flags;      // flags
    unsigned char trans_rgb[3];   // transparency color
    unsigned char nothing[10];
  } sprheader;
  unsigned char *sprrgb;

  FILE *fli;
  char dir[256];
  int i, j;
  if (trans_index != -1) {
    if (trans_rgb == TRUE) fprintf(log, "Warning: Used incompatible parameters, using -tci\n");
  }
  if (trans_r == -1) trans_r = 0;
  if (trans_g == -1) trans_g = 0;
  if (trans_b == -1) trans_b = 0;
  if (verbmode) fprintf(log, "Checking if the FLIC file exists...\n");
  fli = fopen(flifile, "rb");
  if (fli == NULL) {
    fprintf(log, "Error: FLIC file %s not found\n", flifile);
    return;
  }
  if (verbmode) fprintf(log, "FLIC file %s exists\n", flifile);
  if (verbmode) fprintf(log, "Reading the FLIC header...\n");
  if (fread(&fliheader, sizeof(fliheader), 1, fli) == 0) {
    fprintf(log, "Error: Cannot read the FLIC header\n");
    fclose(fli);
    return;
  }
  if (verbmode) fprintf(log, "FLIC header read\n");
  if (verbmode) fprintf(log, "Checking FLIC header...\n");
  if (verbmode) fprintf(log, "FLIC magic number: %X\n", fliheader.magic);
  if (fliheader.magic == FLI_MAGIC) fprintf(log, "The file is a compatible FLI\n");
  else if (fliheader.magic == FLC_MAGIC) fprintf(log, "The file is a compatible FLC\n");
  else {
    fprintf(log, "Error: The file seems to be of an incorrect type\n");
    fclose(fli);
    return;
  }
  if (verbmode) fprintf(log, "FLIC frames: %d\n", fliheader.frames);
  if (fliheader.frames > 4000) {
    fprintf(log, "Error: FLIC animations do not support more than 4000 frames\n");
    fclose(fli);
    return;
  }
  if (verbmode) fprintf(log, "FLIC width: %d\n", fliheader.width);
  if (verbmode) fprintf(log, "FLIC height: %d\n", fliheader.height);
  if (verbmode) fprintf(log, "FLIC depth: %d\n", fliheader.depth);
  if (fliheader.depth != 8) {
    fprintf(log, "Error: FLIC depth is not 8 (as is standard)\n");
    fclose(fli);
    return;
  }
  memset(&fliheader.nothing, 0, sizeof(fliheader.nothing));
  memset(dir, 0, sizeof(dir));
  i = strcspn(flifile, ".");
  strncpy(dir, flifile, i);
  if (verbmode) fprintf(log, "Initializing sprite headers...\n");
  sprheader.magic = 0x21525053;
  sprheader.ver = 300;
  sprheader.height = fliheader.height;
  sprheader.width = fliheader.width;
  memset(&sprheader.flags, 0, sizeof(sprheader.flags));
  memset(sprheader.nothing, 0, sizeof(sprheader.nothing));
  if (verbmode) fprintf(log, "Sprite headers initialized\n");
  if (verbmode) fprintf(log, "Allocating memory for sprites...\n");
  sprrgb = (unsigned char*)malloc(sprheader.height * sprheader.width * 3);
  if (sprrgb == NULL) {
    fprintf(log, "Error: Not enough memory\n");
    fclose(fli);
    return;
  }
  if (verbmode) fprintf(log, "Memory allocated\n");
  memset(flipalette, 0, sizeof(flipalette));
  if (fromframe == -1) fromframe = 0;
  if (toframe == -1) toframe = fliheader.frames - 1;
  for (i = 0; i <= toframe; i++) {
    FILE *spr;
    char sprname[256];
    if (verbmode) fprintf(log, "Proccessing FLIC frame %d of %d...\n", i + 1, fliheader.frames);
    if (verbmode) fprintf(log, "Reading FLIC frame chunk...\n");
    if(fread(&fliframechunk, sizeof(fliframechunk), 1, fli) == 0) {
      fclose(fli);
      fprintf(log, "Error: There seems to be less frames in the FLIC than are supposed to be\n");
      break;
    }
    if (fliframechunk.type == FLI_PREFIX) {
      if (verbmode) fprintf(log, "It's a FLC prefix chunk - skipping...\n");
      if (fseek(fli, fliframechunk.size - 16, SEEK_CUR) != 0) {
        fprintf(log, "Error: Cannot skip the FLC prefix chunk data\n");
        fclose(fli);
        break;
      }
      if (verbmode) fprintf(log, "FLC Prefix chunk skipped\n");
      continue;
    }
    else if (fliframechunk.type != FLI_FRAME) {
      fclose(fli);
      fprintf(log, "Error: Missed FLIC frame chunk in frame %d\n", i);
      break;
    }
    if (verbmode) fprintf(log, "FLIC frame chunk readed\n");
    for (j = 0; j < fliframechunk.chunks; j++) {
      if (verbmode) fprintf(log, "Reading FLIC chunk %d of %d of frame %d\n", j + 1, fliframechunk.chunks, i + 1);
      if (fread(&flichunkheader, 6, 1, fli) == 0) {
        fclose(fli);
        fprintf(log, "Error: Missed FLIC chunk %d in frame %d\n", j + 1, i + 1);
        break;
      }
      flichunk = (char*)malloc(flichunkheader.size);
      if (fread(flichunk, flichunkheader.size - 6, 1, fli) == 0) {
        fprintf(log, "Error: Missed FLIC chunk %d in frame %d\n", j + 1, i + 1);
        fclose(fli);
        break;
      }
      if (flichunkheader.type == FLI_COLOR256) {
        unsigned short int packets, x, y, z, w, read2;
        unsigned char skip, read, r, g, b;
        if (verbmode) fprintf(log, "Found and processing FLI_COLOR256 chunk...\n");
        memcpy(&packets, flichunk, sizeof(packets));
        y = sizeof(packets);
        for (x = 0, z = 0; x < packets; x++) {
          if (verbmode) fprintf(log, "Processing packet %d of %d...\n", x + 1, packets);
          memcpy(&skip, flichunk + y, sizeof(skip)); y += sizeof(skip);
          z += skip;
          memcpy(&read, flichunk + y, sizeof(read)); y += sizeof(read);
          read2 = read;
          if (read2 == 0) read2 = 256;
          for (w = 0; w < read2; w++) {
            memcpy(&flipalette[z][0], flichunk + y, 1); y++;
            memcpy(&flipalette[z][1], flichunk + y, 1); y++;
            memcpy(&flipalette[z][2], flichunk + y, 1); y++;
            z++;
          }
        }
      }
      else if (flichunkheader.type == FLI_COLOR64) {
        unsigned short int packets, x, y, z, w, read2;
        unsigned char skip, read, r, g, b;
        if (verbmode) fprintf(log, "Found and processing FLI_COLOR64 chunk...\n");
        memcpy(&packets, flichunk, sizeof(packets));
        y = sizeof(packets);
        for (x = 0, z = 0; x < packets; x++) {
          if (verbmode) fprintf(log, "Processing packet %d of %d...\n", x + 1, packets);
          memcpy(&skip, flichunk + y, sizeof(skip)); y += sizeof(skip);
          z += skip;
          memcpy(&read, flichunk + y, sizeof(read)); y += sizeof(read);
          read2 = read;
          if (read2 == 0) read2 = 256;
          for (w = 0; w < read2; w++) {
            memcpy(&flipalette[z][0], flichunk + y, 1); y++;
            flipalette[z][0] *= 4;
            memcpy(&flipalette[z][1], flichunk + y, 1); y++;
            flipalette[z][1] *= 4;
            memcpy(&flipalette[z][2], flichunk + y, 1); y++;
            flipalette[z][2] *= 4;
            z++;
          }
        }
      }
      else if (flichunkheader.type == FLI_LC) {
        unsigned short int starty, county, x, y;
        unsigned long int z;
        if (verbmode) fprintf(log, "Found and processing FLI_LC chunk...\n");
        z = 0;
        memcpy(&starty, flichunk + z, 2); z += 2;
        memcpy(&county, flichunk + z, 2); z += 2;
        for (y = starty; y < (county + starty); y++) {
          unsigned int k;
          unsigned char packets;
          memcpy(&packets, flichunk + z, 1); z++;
          x = 0;
          for (k = 0; k < packets; k++) {
            unsigned char xskip, pixel;
            signed char rlecount;
            unsigned long int sprcount;
            memcpy(&xskip, flichunk + z, 1); z++;
            memcpy(&rlecount, flichunk + z, 1); z++;
            x += xskip;
            if (rlecount > 0) {
              for ( ; rlecount != 0; rlecount--) {
                memcpy(&pixel, flichunk + z, 1); z++;
                sprcount = ((y * fliheader.width) + x) * 3;
                sprrgb[sprcount] = flipalette[pixel][2];
                sprrgb[sprcount + 1] = flipalette[pixel][1];
                sprrgb[sprcount + 2] = flipalette[pixel][0];
                x++;
              }
            }
            else if (rlecount < 0) {
              memcpy(&pixel, flichunk + z, 1); z++;
              for ( ; rlecount != 0; rlecount++) {
                sprcount = ((y * fliheader.width) + x) * 3;
                sprrgb[sprcount] = flipalette[pixel][2];
                sprrgb[sprcount + 1] = flipalette[pixel][1];
                sprrgb[sprcount + 2] = flipalette[pixel][0];
                x++;
              }
            }
          }
        }
      }
      else if (flichunkheader.type == FLI_BLACK) {
        unsigned short int x, y;
        unsigned long int z;
        if (verbmode) fprintf(log, "Found and processing FLI_BLACK chunk...\n");
        z = 0;
        for (y = 0; y < sprheader.height; y++ ) {
          for (x = 0; x < sprheader.width; x++) {
            sprrgb[z] = flipalette[0][2];
            sprrgb[z + 1] = flipalette[0][1];
            sprrgb[z + 2] = flipalette[0][0];
            z += 3;
          }
        }
      }
      else if (flichunkheader.type == FLI_BRUN) {
        unsigned short int x, y;
        unsigned long int z, k;
        if (verbmode) fprintf(log, "Found and processing FLI_BRUN chunk...\n");
        z = k = 0;
        for (y = 0; y < fliheader.height; y++ ) {
          k++;
          for (x = 0; x < fliheader.width; ) {
            signed char info;
            unsigned char pixel;
            memcpy(&info, flichunk + k, 1); k++;
            if (info < 0) {
              for ( ; info != 0; info++) {
                memcpy(&pixel, flichunk + k, 1); k++;
                sprrgb[z] = flipalette[pixel][2];
                sprrgb[z + 1] = flipalette[pixel][1];
                sprrgb[z + 2] = flipalette[pixel][0];
                z += 3; x++;
              }
            }
            else if (info > 0) {
              memcpy(&pixel, flichunk + k, 1); k++;
              for ( ; info != 0; info--) {
                sprrgb[z] = flipalette[pixel][2];
                sprrgb[z + 1] = flipalette[pixel][1];
                sprrgb[z + 2] = flipalette[pixel][0];
                z += 3; x++;
              }
            }            
          }
        }
      }
      else if (flichunkheader.type == FLI_COPY) {
        unsigned short int x, y;
        unsigned long int z;
        unsigned char k;
        if (verbmode) fprintf(log, "Found and processing FLI_COPY chunk...\n");
        z = 0;
        for (y = 0; y < sprheader.height; y++ ) {
          for (x = 0; x < sprheader.width; x++) {
            memcpy(&k, flichunk + z / 3, 1);
            sprrgb[z] = flipalette[k][2];
            sprrgb[z + 1] = flipalette[k][1];
            sprrgb[z + 2] = flipalette[k][0];
            z += 3;
          }
        }
      }
      else if (flichunkheader.type == FLI_SS2) {
        unsigned short int countline, x, y, line;
        unsigned long int z;
        if (verbmode) fprintf(log, "Found and processing FLI_SS2 chunk...\n");
        z = 0;
        memcpy(&countline, flichunk + z, 2); z += 2;
        for (line = 0, y = 0; line < countline; line++, y++) {
          unsigned short int k;
          signed short int word;
          unsigned char packets;
          k = FALSE;
          while (k != TRUE) {
            memcpy(&word, flichunk + z, 2); z += 2;
            if ((word & 0xC000) == 0xC000) {              // skip count
              y += abs(word);
            }
            else if ((word & 0xC000) == 0x8000) {         // last byte          }
              unsigned long int sprcount;
              unsigned char pixel;
              sprcount = ((y * fliheader.width) + (fliheader.width - 1)) * 3;
              pixel = (word & 0xFF);
              sprrgb[sprcount] = flipalette[pixel][2];
              sprrgb[sprcount + 1] = flipalette[pixel][1];
              sprrgb[sprcount + 2] = flipalette[pixel][0];
            }
            else if ((word & 0xC000) == 0x0000) {         // packet count
              packets = (word & 0xFF);
              k = TRUE;
            }
            else {
              fprintf(log, "Warning: High order two bits are unkown\n");
            }
          }
          x = 0;
          for (k = 0; k < packets; k++) {
            unsigned char xskip, pixel, pixel2;
            signed char rlecount;
            unsigned long int sprcount;
            memcpy(&xskip, flichunk + z, 1); z++;
            memcpy(&rlecount, flichunk + z, 1); z++;
            x += xskip;
            if (rlecount > 0) {
              for ( ; rlecount != 0; rlecount--) {
                memcpy(&pixel, flichunk + z, 1); z++;
                sprcount = ((y * fliheader.width) + x) * 3;
                sprrgb[sprcount] = flipalette[pixel][2];
                sprrgb[sprcount + 1] = flipalette[pixel][1];
                sprrgb[sprcount + 2] = flipalette[pixel][0];
                x++;
                memcpy(&pixel, flichunk + z, 1); z++;
                sprcount = ((y * fliheader.width) + x) * 3;
                sprrgb[sprcount] = flipalette[pixel][2];
                sprrgb[sprcount + 1] = flipalette[pixel][1];
                sprrgb[sprcount + 2] = flipalette[pixel][0];
                x++;
              }
            }
            else if (rlecount < 0) {
              memcpy(&pixel, flichunk + z, 1); z++;
              memcpy(&pixel2, flichunk + z, 1); z++;
              for ( ; rlecount != 0; rlecount++) {
                sprcount = ((y * fliheader.width) + x) * 3;
                sprrgb[sprcount] = flipalette[pixel][2];
                sprrgb[sprcount + 1] = flipalette[pixel][1];
                sprrgb[sprcount + 2] = flipalette[pixel][0];
                x++;
                sprcount = ((y * fliheader.width) + x) * 3;
                sprrgb[sprcount] = flipalette[pixel2][2];
                sprrgb[sprcount + 1] = flipalette[pixel2][1];
                sprrgb[sprcount + 2] = flipalette[pixel2][0];
                x++;
              }
            }
          }
        }
      }
      else if (flichunkheader.type == FLI_PSTAMP) {
        fprintf(log, "Found and ignored FLI_PSTAMP chunk...\n");
      }
      else
        fprintf(log, "Warning: Found unknown chunk (type %d, chunk %d, frame %d)\n", flichunkheader.type, j + 1, i + 1);
      free(flichunk);
    }
    if (i >= fromframe) {
      if (verbmode) fprintf(log, "Creating SPR file...\n", i);
      sprintf(sprname, "%s.%d", dir, i);
      spr = fopen(sprname, "wb");
      if (spr == NULL) {
        fprintf(log, "Error: Cannot create the SPR file %s (disk full?)\n", sprname);
        break;
      }
      if (verbmode) fprintf(log, "SPR file %s created\n", sprname);
      if (trans_index != -1) {
        sprheader.trans_rgb[0] = flipalette[trans_index][0];
        sprheader.trans_rgb[1] = flipalette[trans_index][1];
        sprheader.trans_rgb[2] = flipalette[trans_index][2];
      }
      else if (trans_rgb == TRUE) {
        sprheader.trans_rgb[0] = trans_r;
        sprheader.trans_rgb[1] = trans_g;
        sprheader.trans_rgb[2] = trans_b;
      }
      else {
        sprheader.trans_rgb[0] = flipalette[0][0];
        sprheader.trans_rgb[1] = flipalette[0][1];
        sprheader.trans_rgb[2] = flipalette[0][2];
      }
      if (verbmode) fprintf(log, "Writing SPR header...\n");
      if (fwrite(&sprheader, sizeof(sprheader), 1, spr) == 0) {
        fprintf(log, "Error: Cannot write the SPR header (disk full?)\n");
        break;
      }
      if (verbmode) fprintf(log, "SPR header written\n");
      if (verbmode) fprintf(log, "Writing SPR image data...\n");
      if (fwrite(sprrgb, sprheader.width * sprheader.height * 3, 1, spr) == 0) {
        fclose(spr);
        fprintf(log, "Error: Cannot write the SPR image data (disk full?)\n", sprname);
        break;
      }
      if (verbmode) fprintf(log, "SPR image data wrote\n", sprname);
      fclose(spr);
    }
  }
  fprintf(log, "Proccessed from frame %d to %d and converted into MAD sprites\n", fromframe + 1, i);
  fclose(fli);
}
