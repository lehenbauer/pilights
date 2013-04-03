/*
 * CDATA
 *
 * Copyright (C) 2013 by Karl Lehenbauer, All Rights Reserved
 */

#include <tcl.h>
#include <gd.h>
#include <tclspi.h>
#include <tclgd.h>
#include <string.h>

#define PIXEL_TO_LED(x) ((x >> 1) | 0x80)

extern int
pilights_pilightObjCmd(ClientData dummy, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]);

typedef struct pilights_clientData
{
    int nRows;
    int nLights;
    int nRowBytes;  // nLights * 3 + latch bytes
    unsigned char **rowData;
    int          destroyOnDelete;
} pilights_clientData;

