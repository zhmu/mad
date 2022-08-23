/*
                               INV.CPP

           MAD Inventory Routines - Handling of the inventory.

                   (c) 1998, 1999, 2000 The MAD Crew

  Description:
  This module will take care of the inventory.

  Todo: Nothing.

  Portability notes: Fully portable, no changes required
*/
#include <stdlib.h>
#include <string.h>
#include "archive.h"
#include "bar.h"
#include "dlgman.h"
#include "fontman.h"
#include "inv.h"
#include "project.h"
#include "mad.h"
#include "types.h"

#define MAD_INV_DIALOGTITLE "Inventory"
#define MAD_INV_LOOK        " Look "
#define MAD_INV_USE         " Use "
#define MAD_INV_OK          " OK "

#define MAD_ERROR_800 "Error 800: File '%s' is not a MAD inventory file"
#define MAD_ERROR_801 "Error 801: File '%s' is of a wrong version"
#define MAD_ERROR_802 "Error 802: Item '%s' never declared in inventory datafile"
#define MAD_ERROR_803 "Error 803: Too many inventory items used"

/*
 * INV::INV()
 *
 * This is the constructor of the inventory manager. It will clear most
 * fields.
 *
 */
INV::INV() {
    _UINT i;

    /* nuke out the complete inventory */
    for(i=0;i<MAD_INV_MAX_ITEMS;i++) {
        inv_item[i].inv_id=MAD_INV_NOITEM;
        inv_item[i].quantity=0;
    }

    /* set data to null and say there no inventory items */
    data=NULL;nofitems=0;status=0;
}

/*
 * INV::init(char* fname)
 *
 * This will load file [fname] as an inventory file. It won't do anything if
 * the no inventory flag of the project is set.
 *
 */
void
INV::init(char* fname) {
    char tempstr[MAD_TEMPSTR_SIZE];

    /* is the no inventory flag set? */
    if(project->info.flags&PROJECT_FLAG_NOINV) {
         /* yeah. get outta here! */
         return;
    }

    /* load the inventory file */
    data=(char*)archive_readfile(fname,NULL);

    /* create pointers to the data */
    hdr=(struct INV_HEADER*)data;
    inv=(char*)(data+sizeof(INV_HEADER));

    /* verify the header */
    if(hdr->id!=MAD_INV_ID) {
        /* this is not a inventory file. die (error 800: file is not a mad inventory file) */
        sprintf(tempstr,MAD_ERROR_800,fname);
        die(tempstr);
    }
    /* verify the version number */
    if(hdr->version!=MAD_INV_VERSION) {
        /* this is not a inventory file. die (error 801: file is of a wrong version) */
        sprintf(tempstr,MAD_ERROR_801,fname);
        die(tempstr);
    }
}

/*
 * INV::show_inventory()
 *
 * This will pop up a window which will show the inventory.
 *
 */
void
INV::show_inventory() {
    _UINT   dlgno,height,width,idlook,iduse,idok,i,iw,baseno,in,ic;
    DIALOG* dlg;
    _UCHAR  icono[MAD_INV_MAX_INV_ITEMS];
    struct  INV_ITEM *item;

    /* is the no inventory flag set? */
    if(project->info.flags&PROJECT_FLAG_NOINV) {
         /* yeah. get outta here! */
         return;
    }

    /* set variables */
    height=100;width=200;

    /* create the dialog */
    dlgno=dlgman->createdialog(DLGMAN_DIALOG_FLAG_MOVEABLE|DLGMAN_DIALOG_FLAG_TITLEBAR,0,0,height,width);
    dlg=dlgman->dialog[dlgno];

    /* set up the dialog */
    dlg->setitle(MAD_INV_DIALOGTITLE);
    dlg->centerdialog();

    /* create the 'look' button */
    idlook=dlg->createcontrol(DLGMAN_CONTROL_TYPE_BUTTON);
    dlg->control[idlook]->move(0,0);
    dlg->control[idlook]->setcaption(MAD_INV_LOOK);
    dlg->control[idlook]->resize(fontman->getfontheight(fontman->getsysfontno()),fontman->getextwidth(fontman->getsysfontno(),MAD_INV_LOOK));

    /* create the 'use' button */
    iduse=dlg->createcontrol(DLGMAN_CONTROL_TYPE_BUTTON);
    dlg->control[iduse]->move(fontman->getextwidth(fontman->getsysfontno(),MAD_INV_LOOK)+2,0);
    dlg->control[iduse]->setcaption(MAD_INV_USE);
    dlg->control[iduse]->resize(fontman->getfontheight(fontman->getsysfontno()),fontman->getextwidth(fontman->getsysfontno(),MAD_INV_USE));

    /* create the 'ok' button */
    idok=dlg->createcontrol(DLGMAN_CONTROL_TYPE_BUTTON);
    dlg->control[idok]->move(width-fontman->getextwidth(fontman->getsysfontno(),MAD_INV_OK)-4,0);
    dlg->control[idok]->setcaption(MAD_INV_OK);
    dlg->control[idok]->resize(fontman->getfontheight(fontman->getsysfontno()),fontman->getextwidth(fontman->getsysfontno(),MAD_INV_OK));

    iw=(width/MAD_INV_ICON_SIZE_X);

    /* create an inventory icon */
    for(i=0;i<MAD_INV_MAX_INV_ITEMS;i++) {
        icono[i]=dlg->createcontrol(DLGMAN_CONTROL_TYPE_SPRITE);
        dlg->control[icono[i]]->move((i%(iw/2))*(MAD_INV_ICON_SIZE_X*2)+(iw),(i/(iw/2))*(MAD_INV_ICON_SIZE_Y+(MAD_INV_ICON_SIZE_Y/2))+(fontman->getfontheight(fontman->getsysfontno())*2));
    }

    /* now fill in the items */
    baseno=0;in=0;
    for(i=0;i<MAD_INV_MAX_INV_ITEMS;i++) {
        /* first do a range check */
        if((i+baseno)<MAD_INV_MAX_ITEMS) {
             /* this worked. figure out whether this item exists */
             if(inv_item[i+baseno].inv_id!=MAD_INV_NOITEM) {
                  /* this worked. load the icon */
                  item=(struct INV_ITEM *)(data+sizeof(INV_HEADER)+(sizeof(INV_ITEM)*inv_item[i+baseno].inv_id));
                  dlg->control[icono[in]]->loadsprite(item->inv_icon);
                  in++;
             }
        }
    }

    /* do the dialog */
    do {
        i=dlgman->dodialog(dlgno);
        if((i>=icono[0])&&(i<=icono[MAD_INV_MAX_INV_ITEMS-1])) {
             /* The user selected an icon! Is it a valid item? */
             ic=(i-icono[0]);
             if(inv_item[ic].inv_id!=MAD_INV_NOITEM) {
                 /* It is valid! Nuke the dialog and get outta here! */
                 /* destroy the dialog */
                 dlgman->destroydialog(dlgno);
                 /* select the new item */
                 bar->set_inv_item(inv_item[ic].inv_id);
                 /* and get outta here */
                 return;
             }
        }
    } while ((i>=idlook)&&(i<=iduse));

    /* destroy the dialog */
    dlgman->destroydialog(dlgno);
}

/*
 * INV::done()
 *
 * This will deinitialize the inventory manager. It will release memory as
 * required.
 *
 */
void
INV::done() {
    /* was there memory allocated? */
    if(data!=NULL) {
        /* yeah. free it */
        free(data);
        /* and make sure we dont do this twice */
        data=NULL;
    }
}

/*
 * INV::getitemdata(_UINT itemno)
 *
 * This will return a pointer to the data of inventory definition [itemno].
 *
 */
INV_ITEM*
INV::getitemdata(_UINT itemno) {
    /* return the data of item number [itemno] */
    return (INV_ITEM*)(data+sizeof(INV_HEADER)+(itemno*sizeof(INV_ITEM)));
}

/*
 * INV::give_item(char* name,_UINT quantity)
 *
 * This will add [quantity] items called [name]. It must exist in the
 * inventory datafile, otherwise an error will be generated. If the user
 * didn't have the item, it will be given to him.
 *
 */
void
INV::give_item(char* name,_UINT quantity) {
    _UINT i,invid;
    char tempstr[MAD_TEMPSTR_SIZE];

    /* grab the item id */
    i=getitemid(name);

    /* did this work? */
    if(i==MAD_INV_NOITEM) {
        /* no. die (error 802: item never declared in inventory datafile) */
        sprintf(tempstr,MAD_ERROR_802,name);
        die(tempstr);
    }

    /* do we already have this item? */
    if ((invid=scan_inventory(i))==MAD_INV_NOITEM) {
        /* no. are there too many inventory items? */
        if (nofitems>=MAD_INV_MAX_ITEMS) {
            /* yup. complain (error 803: too many inventory items used) */
            die(MAD_ERROR_803);
        }
        /* no. add it */
        invid=nofitems;
        inv_item[invid].inv_id=i;
        inv_item[invid].quantity=0;

        nofitems++;
    }

    /* change the quantity */
    inv_item[invid].quantity+=quantity;
}

/*
 * INV::change_quantity(char* name,_UINT quantity)
 *
 * This will change the quantity of the inventory item called [name]. It must
 * exist in the inventory datafile, otherwise an error will be generated. If
 * the user didn't have the item, the item will be given to him. If
 * [quantity] is zero, the item will be dropped.
 *
 */
void
INV::change_quantity(char* name,_UINT quantity) {
    _UINT i,invid;
    char tempstr[MAD_TEMPSTR_SIZE];

    /* grab the item id */
    i=getitemid(name);

    /* did this work? */
    if(i==MAD_INV_NOITEM) {
        /* no. die (error 802: item never declared in inventory datafile) */
        sprintf(tempstr,MAD_ERROR_802,name);
        die(tempstr);
    }

    /* do we have this item? */
    if ((invid=scan_inventory(i))==MAD_INV_NOITEM) {
        /* no. chain to give_item() */
        give_item(name,quantity);
        return;
    }

    /* change the quantity */
    inv_item[invid].quantity=quantity;
    /* zero quantity? */
    if (!quantity) {
        /* yup. drop the item */
        inv_item[invid].inv_id=MAD_INV_NOITEM;
    }
}

/*
 * INV::drop_item(char* name)
 *
 * This will drop the inventory item called [name]. It must exist in the
 * inventory datafile, otherwise an error will be generated. If the user
 * didn't have the item, nothing will happened.
 *
 */
void
INV::drop_item(char* name) {
    /* change the quantity to zero */
    change_quantity(name,0);
}

/*
 * INV::getitemid(char* name)
 *
 * This will return the ID of the inventory item named [name], or
 * MAD_INV_NOITEM if it doesn't exist.
 *
 */
_UINT
INV::getitemid(char* name) {
    _UINT i;
    INV_ITEM* ii;

    /* scan them all */
    for(i=0;i<hdr->nofitems;i++) {
        /* is this the one? */
        ii=getitemdata (i);
        if(!strcmp(ii->name,name)) {
            /* yup. return this id */
            return i;
        }
        /* no. keep looking */
    }

    /* the item wasn't found */
    return MAD_INV_NOITEM;
}

/*
 * INV::scan_inventory(_UINT id)
 *
 * This will scan the user inventory for a item that has id [id]. It will
 * return the number of it when it's found, otherwise MAD_INV_NOITEM.
 *
 */
_UINT
INV::scan_inventory(_UINT id) {
    _UINT i;

    /* scan them all */
    for(i=0;i<MAD_INV_MAX_ITEMS;i++) {
        /* is this the one? */
        if(inv_item[i].inv_id==id) {
            /* yup. return this id */
            return i;
        }
        /* no. keep looking */
    }

    /* the item wasn't found */
    return MAD_INV_NOITEM;
}

/*
 * INV::get_quantity(char* name)
 *
 * This will return the quantity of the inventory item called [name]. It must
 * exist in the inventory datafile, otherwise an error will be generated. If
 * the user didn't have the item, zero will be returned.
 *
 */
_UINT
INV::get_quantity(char* name) {
    _UINT i,invid;
    char tempstr[MAD_TEMPSTR_SIZE];

    /* grab the item id */
    i=getitemid(name);

    /* did this work? */
    if(i==MAD_INV_NOITEM) {
        /* no. die (error 802: item never declared in inventory datafile) */
        sprintf(tempstr,MAD_ERROR_802,name);
        die(tempstr);
    }

    /* do we have this item? */
    if ((invid=scan_inventory(i))==MAD_INV_NOITEM) {
        /* no. return zero */
        return 0;
    }

    /* return the correct quantity */
    return inv_item[i].quantity;
}

/*
 * INV::setstatus(_UCHAR stat)
 *
 * This will set the status of the inventory window to [stat]. If [stat] is
 * zero, the interprinter will not allow the inventory window to pop up by
 * use of the TAB key, otherwise, it will.
 *
 */
void
INV::setstatus(_UCHAR stat) {
    status=stat;
}

/*
 * INV::getstatus()
 *
 * This will return the status of the inventory window.
 *
 */
_UCHAR
INV::getstatus() {
    return status;
}
