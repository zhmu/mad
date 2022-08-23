/* are we already included? */
#ifndef __INV_INCLUDED__
/* no. set flag we are included, and define things */
#define __INV_INCLUDED__

#include "script.h"
#include "types.h"

#define MAD_INV_ID 0x21564e49             /* inventory magic value */
#define MAD_INV_VERSION 100               /* inventory version */
#define MAD_INV_NAME_LENGTH 16            /* inventory item name length */
#define MAD_INV_ICONAME_LENGTH 16         /* inventory icon name length */

#define MAD_INV_MAX_INV_ITEMS 18          /* maximum number of items in the DIALOG */
#define MAD_INV_ICON_SIZE_X   16          /* horizontal size of an inventory icon */
#define MAD_INV_ICON_SIZE_Y   16          /* vertical size of an inventory icon */

#define MAD_INV_MAX_ITEMS     64          /* maximum number of items in GAME inventory */
#define MAD_INV_NOITEM        0xffff      /* no inventory item */

struct INV_HEADER {
    _ULONG id;                            /* id, should be MAD_INV_ID */
    _UINT  version;                       /* version, should be MAD_INV_VERSION */
    _UINT  nofitems;                      /* number of inventory items in file */
    char   reserved[56];                  /* unused */
};

struct INV_ITEM {
    char   name[MAD_INV_NAME_LENGTH];     /* name */
    _ULONG max_quantity;                  /* maximum quantity */
    char   inv_icon[MAD_INV_ICONAME_LENGTH]; /* inventory icon file */
    _ULONG flags;                         /* flags */
    char   reserved[88];                  /* unused */
};

struct GAMEINV_ITEM {
    _UINT  inv_id;                        /* id of object in inventory file */
    _ULONG quantity;                      /* amount of the object */
};

class INV {
public:
    INV();

    void      init(char*);
    void      done();

    void      show_inventory();

    INV_ITEM* getitemdata(_UINT);

private:
    struct INV_HEADER *hdr;
    char   *inv;

    char   *data;

    struct GAMEINV_ITEM inv_item[MAD_INV_MAX_ITEMS];

    _UINT  nofitems;
};
#endif
