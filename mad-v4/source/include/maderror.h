/* are we already included? */
#ifndef __ERRORS_INCLUDED__
/* no. set flag we are included, and define things */
#define __ERRORS_INCLUDED__

/* General errors */
#define MAD_ERROR_1 "Error 1: unable to load '%s'"
#define MAD_ERROR_2 "Error 2: out of memory (allocation of %lu bytes failed)"
#define MAD_ERROR_3 "Error 3: read error when trying to read file '%s'"
#define MAD_ERROR_4 "Error 4: file '%s' was not found in archive"
#define MAD_ERROR_5 "Error 5: animation manager failed when loading '%s'. it said '%s'"
#define MAD_ERROR_6 "Error 6: emergency exit"
#define MAD_ERROR_7 "Error 7: unable to create a %s object"
#define MAD_ERROR_8 "Error 8: terminated by windows message"
#define MAD_ERROR_9 "Error 9: archive said '%s' when trying to initialize '%s'"
#define MAD_ERROR_10 "Error 10: project manager says %s when trying to load '%s'"
#define MAD_ERROR_11 "Error 11: cannot ignore key, there are too much in buffer"
#define MAD_ERROR_12 "Error 12: graphics driver doesn't contain the '%s' function"
#define MAD_ERROR_13 "Error 13: cannot load graphics driver '%s'"
#define MAD_ERROR_14 "Error 14: '%s' is not a graphics driver"
#define MAD_ERROR_15 "Error 15: Initialization of graphics driver failed, it reported '%s'"
#define MAD_ERROR_16 "Error 16: Graphics driver reported '%s'"
#define MAD_ERROR_17 "Error 17: Sound driver reported '%s'"
#define MAD_ERROR_18 "Error 18: Compression code error (eg. bad checksum)"

#define MAD_ERROR_UNKNOWN "unknown error. help!"

#define MAD_DOS_ERROR_MSG "We're sorry, you have encountered a MAD error. Please report\n" \
                          "the following information to bugs@mad-project.cx so we can\n" \
                          "look at the problem. Thank you.\n\n"

/* Bar errors */
#define MAD_ERROR_200 "Error 200: too much icons in bar, cannot add '%s'"
#define MAD_ERROR_201 "Error 201: cannot retrieve name of icon %u, that icon was never loaded"
#define MAD_ERROR_202 "Error 202: tried to use object $%x as move object, $%x is the maximum"
#define MAD_ERROR_203 "Error 203: tried to use internal object '%s' as move object"

/* Graphics manager errors */
#define MAD_ERROR_300 "Error 300: animation $%x doesn't have a valid source number"
#define MAD_ERROR_301 "Error 301: animation $%x doesn't have a valid animation sequence number"
#define MAD_ERROR_302 "Error 302: animation $%x doesn't have a valid animation frame number"
#define MAD_ERROR_303 "Error 303: animation $%x doesn't have a valid animation sprite number"
#define MAD_ERROR_304 "Error 304: cannot load '%s', there are no more data sprites available"
#define MAD_ERROR_305 "Error 305: cannot load '%s', there are no more animation data objects are available"
#define MAD_ERROR_306 "Error 306: cannot load '%s', it is not a picture"
#define MAD_ERROR_307 "Error 307: cannot load '%s', it has a wrong picture version"
#define MAD_ERROR_308 "Error 308: animation object '%s' doesn't have a valid mask direction"

/* Font manager errors */
#define MAD_ERROR_400 "Error 400: out of font manager entries"
#define MAD_ERROR_401 "Error 401: font file '%s' doesn't contain any data"
#define MAD_ERROR_402 "Error 402: font file '%s' is not a mad font file"

/* Dialog manager errors */
#define MAD_ERROR_500 "Error 500: out of controls"
#define MAD_ERROR_501 "Error 501: out of dialogs"

/* Text manager errors */
#define MAD_ERROR_600 "Error 600: %s is not a MAD text file"
#define MAD_ERROR_601 "Error 601: %s is a MAD text file of a wrong version"
#define MAD_ERROR_602 "Error 602: string number %lu does not exists in the MAD text file"

/* Windows port errors */
#define MAD_ERROR_1000 "Error 1000: Unable to register class"
#define MAD_ERROR_1001 "Error 1001: Unable to create window"
#define MAD_ERROR_1002 "Error 1002: Unable to initialize DirectDraw"
#define MAD_ERROR_1003 "Error 1003: SetCooperativeLevel() failed"
#define MAD_ERROR_1004 "Error 1004: SetDisplayMode() failed"
#define MAD_ERROR_1005 "Error 1005: CreateSurface() failed"
#define MAD_ERROR_1006 "Error 1006: GetAttachedSurface() failed"
#define MAD_ERROR_1007 "Error 1007: CreatePalette() failed"
#define MAD_ERROR_1008 "Error 1008: SetPalette() failed"
#define MAD_ERROR_1009 "Error 1009: Lock() failed"
#define MAD_ERROR_1010 "Error 1010: Restore() failed"
#define MAD_ERROR_1011 "Error 1011: SetEntries() failed"
#define MAD_ERROR_1012 "Error 1012: Tried to use a NULL void as emergency handler"

#define MAD_WIN_ERROR_TITLE "MAD Error"

/* Animation errors */
#define ANM_ERROR_0 "everything is ok"
#define ANM_ERROR_1 "unable to open animation file"
#define ANM_ERROR_2 "read error"
#define ANM_ERROR_3 "write error"
#define ANM_ERROR_4 "file is not a mad animation file"
#define ANM_ERROR_5 "this is a wrong version"
#define ANM_ERROR_6 "out of memory"
#define ANM_ERROR_7 "create error"
#define ANM_ERROR_8 "too many sprites"
#define ANM_ERROR_9 "too many frames"

/* Archive errors */
#define ARCHIVE_ERROR_0 "everything is ok"
#define ARCHIVE_ERROR_1 "unable to open archive file"
#define ARCHIVE_ERROR_2 "read error"
#define ARCHIVE_ERROR_3 "write error"
#define ARCHIVE_ERROR_4 "file is not a mad archive"
#define ARCHIVE_ERROR_5 "this is a wrong version"
#define ARCHIVE_ERROR_6 "out of memory"
#define ARCHIVE_ERROR_7 "there are no files in archive"
#define ARCHIVE_ERROR_8 "file is not in archive"
#define ARCHIVE_ERROR_9 "create error"
#define ARCHIVE_ERROR_10 "compression error"

/* Project errors */
#define PROJECT_ERROR_0 "everything is ok"
#define PROJECT_ERROR_1 "unable to open project file"
#define PROJECT_ERROR_2 "read error"
#define PROJECT_ERROR_3 "write error"
#define PROJECT_ERROR_4 "file is not a mad project"
#define PROJECT_ERROR_5 "this is a wrong version"
#define PROJECT_ERROR_6 "unable to create project file"

#endif
