/*
 * pilights
 *
 * Copyright (C) 2013 by Karl Lehenbauer, All Rights Reserved
 */

#include "pilights.h"

/* pilights_complain routines -- these get called in a few places after
 * integer conversion failures, etc,
 * help the user understand which function argument contained bad
 * data
 */

static int
pilights_complain(Tcl_Interp *interp, char *element) {
    Tcl_AppendResult (interp, " while converting ", element, " component", NULL);
    return TCL_ERROR;
}

static int
pilights_complainrow(Tcl_Interp *interp) {
    return pilights_complain (interp, "row");
}

static int
pilights_complainfirstRow(Tcl_Interp *interp) {
    return pilights_complain (interp, "firstRow");
}

static int
pilights_complainnRows(Tcl_Interp *interp) {
    return pilights_complain (interp, "nRows");
}

static int
pilights_complainfirstPixel(Tcl_Interp *interp) {
    return pilights_complain (interp, "firstPixel");
}

static int
pilights_complainnPixels(Tcl_Interp *interp) {
    return pilights_complain (interp, "nPixels");
}

static int
pilights_complainnLights(Tcl_Interp *interp) {
    return pilights_complain (interp, "nLights");
}

static int
pilights_complainr(Tcl_Interp *interp) {
    return pilights_complain (interp, "r");
}

static int
pilights_complaing(Tcl_Interp *interp) {
    return pilights_complain (interp, "g");
}

static int
pilights_complainb(Tcl_Interp *interp) {
    return pilights_complain (interp, "b");
}

static int
pilights_complaindelay(Tcl_Interp *interp) {
    return pilights_complain (interp, "delay");
}

static int
pilights_complainx(Tcl_Interp *interp) {
    return pilights_complain (interp, "x");
}

static int
pilights_complainy(Tcl_Interp *interp) {
    return pilights_complain (interp, "y");
}


static void
pilightsFillRows (pilights_clientData *cData, int firstRow, int nRows, int r, int g, int b) {
    int row;
    int pixel;
    unsigned char *rowPtr;
    unsigned char rByte, gByte, bByte;

    rByte = PIXEL_TO_LED(r);
    gByte = PIXEL_TO_LED(g);
    bByte = PIXEL_TO_LED(b);

    for (row = firstRow; row < nRows; row++) {
        rowPtr = cData->rowData[row];
        for (pixel = 0; pixel < cData->nLights; pixel++) {
	    *rowPtr++ = rByte;
	    *rowPtr++ = gByte;
	    *rowPtr++ = bByte;
	}
    }
}

static void
pilightsFillPixels (pilights_clientData *cData, int row, int firstPixel, int nPixels, int r, int g, int b) {
    int pixel, lastPixel;
    unsigned char *rowPtr;
    unsigned char rByte, gByte, bByte;

    rByte = PIXEL_TO_LED(r);
    gByte = PIXEL_TO_LED(g);
    bByte = PIXEL_TO_LED(b);

    rowPtr = cData->rowData[row];

    lastPixel = firstPixel + nPixels;
    if (lastPixel > cData->nLights) {
        lastPixel = cData->nLights;
    }

    for (pixel = firstPixel; pixel < lastPixel; pixel++) {
	*rowPtr++ = rByte;
	*rowPtr++ = gByte;
	*rowPtr++ = bByte;
    }
}

static void
pilightsCopyRows (pilights_clientData *cData, int firstRow, int destRow, int nRows) {
    int row;
    unsigned char *rowPtr;
    unsigned char *destRowPtr;

    for (row = firstRow; row < nRows; row++, destRow++) {
        rowPtr = cData->rowData[row];
	destRowPtr = cData->rowData[destRow];

	memcpy (destRowPtr, rowPtr, cData->nRowBytes);
    }
}



/*
 *----------------------------------------------------------------------
 *
 * pilights_copyGDpixels --
 *
 *    Given a tclgd object name, a row and a starting pixel number,
 *    copy pixels to a ByteArray.
 *
 * Results:
 *    None.
 *
 *----------------------------------------------------------------------
 */
static void 
pilights_copyGDPixels (pilights_clientData *cData, gdImagePtr im, int startY, int startX, int startRow, int startPixel, int nPixels)
{
    int x, y;
    int pixel, pixelColor;
    int row = startRow;
    unsigned char *rowPtr;

    pixel = startPixel;
    rowPtr = cData->rowData[row] + 3 * startPixel;

    for (y = startY; (y < im->sy); y++, startX = 0) {
        for (x = startX; (x < im->sx); x++) {
	    pixelColor = im->trueColor ? gdImageTrueColorPixel (im, x, y) : gdImagePalettePixel (im, x, y);
	    *rowPtr++ = PIXEL_TO_LED(gdImageRed (im, pixelColor));
	    *rowPtr++ = PIXEL_TO_LED(gdImageBlue (im, pixelColor));
	    *rowPtr++ = PIXEL_TO_LED(gdImageGreen (im, pixelColor));

	    if (pixel++ >= cData->nLights) {
	        pixel = 0;
		row++;
		rowPtr = cData->rowData[row];
	    }
	}
    }
}


/*
 *----------------------------------------------------------------------
 *
 * pilights_cmdNameObjToSPI --
 *
 *    Given a Tcl_Obj containing a name for an existing tclspi SPI
 *    interface command object, set a pointer to a pointer to
 *    the tclspi_clientData held by that object.
 *
 *    If the command name can be successfully looked up, return TCL_OK, else
 *    return TCL_ERROR.
 *
 * Results:
 *    None.
 *
 *----------------------------------------------------------------------
 */
static int
pilights_cmdNameObjToSPI (Tcl_Interp *interp, Tcl_Obj *commandNameObj, tclspi_clientData **spiDataPtr)
{
    Tcl_CmdInfo        cmdInfo;

    if (!Tcl_GetCommandInfo ( interp, Tcl_GetString(commandNameObj), &cmdInfo)) {
	return TCL_ERROR;
    }

    if (cmdInfo.objClientData == NULL) {
	Tcl_AppendResult (interp, "Error: '", Tcl_GetString (commandNameObj), "' is not a pilights object", NULL);
	return TCL_ERROR;
    }

    *spiDataPtr = ((tclspi_clientData *)cmdInfo.objClientData);
    return TCL_OK;
}


/*
 *----------------------------------------------------------------------
 *
 * pilights_cmdNameObjToGdImagePtr --
 *
 *    Given a Tcl_Obj containing a name for an existing tclspi GD
 *    interface command object, set a pointer to a 
 *    gdImagePtr held by that object.
 *
 *    If the command name can be successfully looked up, return TCL_OK, else
 *    return TCL_ERROR.
 *
 * Results:
 *    None.
 *
 *----------------------------------------------------------------------
 */
static int
pilights_cmdNameObjToGdImagePtr (Tcl_Interp *interp, Tcl_Obj *commandNameObj, gdImagePtr *imPtr)
{
    Tcl_CmdInfo        cmdInfo;

    if (!Tcl_GetCommandInfo ( interp, Tcl_GetString(commandNameObj), &cmdInfo)) {
	return TCL_ERROR;
    }

    if (cmdInfo.objClientData == NULL) {
	Tcl_AppendResult (interp, "Error: '", Tcl_GetString (commandNameObj), "' is not a GD object", NULL);
	return TCL_ERROR;
    }

    *imPtr = ((tclgd_clientData *)cmdInfo.objClientData)->im;
    return TCL_OK;
}

static int
plights_spi_write (pilights_clientData *pData, tclspi_clientData *spiData, int firstRow, int nRows, int delayUsecs) {
    int row, ret;
    struct spi_ioc_transfer spi;

    spi.delay_usecs = delayUsecs;
    spi.rx_buf = (unsigned long) NULL;
    spi.len = pData->nRowBytes;
    spi.speed_hz = spiData->writeSpeed;
    spi.bits_per_word = 8;

    for (row = firstRow; row < firstRow + nRows; row++) {
	unsigned char *rowPtr = pData->rowData[row];

        spi.tx_buf = (unsigned long) rowPtr;

	ret = ioctl (spiData->fd, SPI_IOC_MESSAGE(1), &spi);
	if (ret < 0) {
	    return ret;
	}
    }
    return 0;
}

void pilights_deleteProc (ClientData clientData) {
    pilights_clientData *cData = (pilights_clientData *)clientData;
    int row;

    for (row = 0; row < cData->nRows; row++) {
        ckfree ((char *)cData->rowData[row]);
    }
    ckfree ((char *)cData->rowData);

    ckfree ((char *)cData);
}


/*
 *----------------------------------------------------------------------
 *
 * pilights_ObjectObjCmd --
 *
 *    dispatches the subcommands of a gd object command
 *
 * Results:
 *    stuff
 *
 *----------------------------------------------------------------------
 */
int
pilights_ObjectObjCmd(ClientData cData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[])
{
    int         optIndex;
    pilights_clientData *pData = (pilights_clientData *)cData;

    static CONST char *options[] = {
	"write",
	"nLights",
	"nRows",
	"fillrows",
	"fillpixels",
	"copyrows",
	"copy_from_image",
	"getrow",
	"setrow",
	(char *)NULL
    };

    enum options {
	OPT_WRITE,
	OPT_NLIGHTS,
	OPT_NROWS,
	OPT_FILLROWS,
	OPT_FILLPIXELS,
	OPT_COPYROWS,
	OPT_COPY_FROM_IMAGE,
	OPT_GETROW,
	OPT_SETROW
    };

    if (objc == 1) {
	Tcl_WrongNumArgs (interp, 1, objv, "option ?args?");
	return TCL_ERROR;
    }

    if (Tcl_GetIndexFromObj(interp, objv[1], options, "option", 
	TCL_EXACT, &optIndex) != TCL_OK) {
	return TCL_ERROR;
    }

    switch ((enum options) optIndex) {
      case OPT_NLIGHTS: {
	if (objc != 2) {
	    Tcl_WrongNumArgs (interp, 2, objv, "");
	    return TCL_ERROR;
	}

	Tcl_SetObjResult (interp, Tcl_NewIntObj (pData->nLights));
	break;
      }

      case OPT_NROWS: {
	if (objc != 2) {
	    Tcl_WrongNumArgs (interp, 2, objv, "");
	    return TCL_ERROR;
	}

	Tcl_SetObjResult (interp, Tcl_NewIntObj (pData->nRows));
	break;
      }

      case OPT_FILLROWS: {
	int firstRow, nRows;
	int r, g, b;

	if (objc != 7) {
	    Tcl_WrongNumArgs (interp, 2, objv, "firstRow nRows r g b");
	    return TCL_ERROR;
	}

       if (Tcl_GetIntFromObj (interp, objv[2], &firstRow) == TCL_ERROR) {
	   return pilights_complainfirstRow (interp);
       }

       if (Tcl_GetIntFromObj (interp, objv[3], &nRows) == TCL_ERROR) {
	   return pilights_complainnRows (interp);
       }

       if (Tcl_GetIntFromObj (interp, objv[3], &r) == TCL_ERROR) {
	   return pilights_complainr (interp);
       }

       if (Tcl_GetIntFromObj (interp, objv[3], &g) == TCL_ERROR) {
	   return pilights_complaing (interp);
       }

       if (Tcl_GetIntFromObj (interp, objv[3], &b) == TCL_ERROR) {
	   return pilights_complainb (interp);
       }

	pilightsFillRows (pData, firstRow, nRows, r, g, b);
	break;
      }

      case OPT_FILLPIXELS: {
	int row, firstPixel, nPixels, r, g, b;

	if (objc != 8) {
	    Tcl_WrongNumArgs (interp, 2, objv, "row firstPixel nPixels r g b");
	    return TCL_ERROR;
	}

       if (Tcl_GetIntFromObj (interp, objv[2], &row) == TCL_ERROR) {
	   return pilights_complainrow (interp);
       }

       if (Tcl_GetIntFromObj (interp, objv[3], &firstPixel) == TCL_ERROR) {
	   return pilights_complainfirstPixel (interp);
       }

       if (Tcl_GetIntFromObj (interp, objv[4], &nPixels) == TCL_ERROR) {
	   return pilights_complainnPixels (interp);
       }

       if (Tcl_GetIntFromObj (interp, objv[5], &r) == TCL_ERROR) {
	   return pilights_complainr (interp);
       }

       if (Tcl_GetIntFromObj (interp, objv[6], &g) == TCL_ERROR) {
	   return pilights_complaing (interp);
       }

       if (Tcl_GetIntFromObj (interp, objv[7], &b) == TCL_ERROR) {
	   return pilights_complainb (interp);
       }

	pilightsFillPixels (pData, row, firstPixel, nPixels, r, g, b);
	break;
      }

      case OPT_WRITE: {
	int row, delay = 0;
	int nRows = 1;
	int firstRow;
	tclspi_clientData *spiClientData;
	int ret;


	if ((objc < 4) || (objc > 6)) {
	    Tcl_WrongNumArgs (interp, 2, objv, "spiObject firstRow ?nRows? ?delay?");
	    return TCL_ERROR;
	}

	if (pilights_cmdNameObjToSPI (interp, objv[2], &spiClientData) == TCL_ERROR) {
	    return TCL_ERROR;
	}

       if (Tcl_GetIntFromObj (interp, objv[3], &firstRow) == TCL_ERROR) {
	   return pilights_complainfirstRow (interp);
       }

       if (objc > 4) {
	   if (Tcl_GetIntFromObj (interp, objv[3], &nRows) == TCL_ERROR) {
	       return pilights_complainnRows (interp);
	   }
       }

       if (objc > 5) {
	   if (Tcl_GetIntFromObj (interp, objv[3], &delay) == TCL_ERROR) {
	       return pilights_complaindelay (interp);
	   }
       }

       for (row = firstRow; row < nRows; row++) {
	    ret = plights_spi_write (pData, spiClientData, firstRow, nRows, delay);
	    if (ret < 0) {
		Tcl_AppendResult (interp, "can't perform spi transfer: ", Tcl_PosixError (interp), NULL);
		return TCL_ERROR;
	  }
	}
      return TCL_OK;
    }

      case OPT_COPYROWS: {
	int firstRow, destRow, nRows;

	if (objc != 5) {
	    Tcl_WrongNumArgs (interp, 2, objv, "firstRow destRow nRows");
	    return TCL_ERROR;
	}

       if (Tcl_GetIntFromObj (interp, objv[2], &firstRow) == TCL_ERROR) {
	   return pilights_complainfirstRow (interp);
       }

       if (Tcl_GetIntFromObj (interp, objv[3], &destRow) == TCL_ERROR) {
	   return pilights_complainr (interp);
       }

       if (Tcl_GetIntFromObj (interp, objv[4], &nRows) == TCL_ERROR) {
	   return pilights_complainnRows (interp);
       }

	pilightsCopyRows ((pilights_clientData *)cData, firstRow, destRow, nRows);
	break;
      }

      case OPT_COPY_FROM_IMAGE: {
	int firstRow;
	int firstPixel, nPixels;
	int x, y;
	gdImagePtr im;

	if (objc != 8) {
	    Tcl_WrongNumArgs (interp, 2, objv, "gdObject x y firstRow firstPixel nPixels");
	    return TCL_ERROR;
	}

	if (pilights_cmdNameObjToGdImagePtr (interp, objv[2], &im) == TCL_ERROR) {
	    return TCL_ERROR;
	}

       if (Tcl_GetIntFromObj (interp, objv[3], &x) == TCL_ERROR) {
	   return pilights_complainx (interp);
       }

       if (Tcl_GetIntFromObj (interp, objv[4], &y) == TCL_ERROR) {
	   return pilights_complainy (interp);
       }

       if (Tcl_GetIntFromObj (interp, objv[5], &firstRow) == TCL_ERROR) {
	   return pilights_complainfirstRow (interp);
       }

       if (Tcl_GetIntFromObj (interp, objv[6], &firstPixel) == TCL_ERROR) {
	   return pilights_complainfirstPixel (interp);
       }

       if (Tcl_GetIntFromObj (interp, objv[7], &nPixels) == TCL_ERROR) {
	   return pilights_complainnPixels (interp);
       }

	pilights_copyGDPixels (cData, im, x, y, firstRow, firstPixel, nPixels);
	break;
      }

      case OPT_GETROW: {
        Tcl_Obj **rowList = (Tcl_Obj **)ckalloc (sizeof(Tcl_Obj *) * pData->nLights * 3);
	Tcl_Obj **rlp;
	unsigned char *r;
	int row;
	int nElements = pData->nLights * 3;
	int i;

	if (objc != 3) {
	    Tcl_WrongNumArgs (interp, 2, objv, "row");
	    return TCL_ERROR;
	}

        if (Tcl_GetIntFromObj (interp, objv[2], &row) == TCL_ERROR) {
	   return pilights_complainrow (interp);
        }

	for (i = 0, r = pData->rowData[row], rlp = rowList; i < nElements; i++) {
	    *rlp++ = Tcl_NewIntObj (LED_TO_PIXEL(*r++));
	}

	Tcl_SetObjResult (interp, Tcl_NewListObj (nElements, rowList));
	ckfree ((char *)rowList);
	break;
      }

      case OPT_SETROW: {
	unsigned char *r;
	int row;
	int nElements = pData->nLights * 3;
	Tcl_Obj **listObjv;
	int listObjc;
	int li;
	int pixel;

	if (objc != 4) {
	    Tcl_WrongNumArgs (interp, 2, objv, "row list");
	    return TCL_ERROR;
	}

        if (Tcl_GetIntFromObj (interp, objv[2], &row) == TCL_ERROR) {
	   return pilights_complainrow (interp);
        }

	if (Tcl_ListObjGetElements (interp, objv[3], &listObjc, &listObjv) == TCL_ERROR) {
	    return TCL_ERROR;
	}

	if (listObjc == 0) {
	    return TCL_ERROR;
	}

	for (pixel = 0, li = 0, r = pData->rowData[row]; pixel < nElements; pixel++, li++) {
	    int value;

	    if (li >= listObjc) {
	        li = 0;
	    }

	    if (Tcl_GetIntFromObj (interp, listObjv[li++], &value) == TCL_ERROR) {
	        return TCL_ERROR;
	    }
	    *r++ = PIXEL_TO_LED(value);
	}
	break;
      }
    }

    return TCL_OK;
}


/*
 *----------------------------------------------------------------------
 *
 * pilights_newObject --
 *
 *	This procedure is invoked to create a new Tcl command
 *      that invokes the pilights_ObjectobjCmd command, that
 *      attaches the specified gd image pointer to the new
 *      command's client data structure.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side effects:
 *	A new Tcl command is created.
 *
 *----------------------------------------------------------------------
 */
int
pilights_newObject (Tcl_Interp *interp, Tcl_Obj *nameObj, int nLights, int nRows)
{
    char              *newName = Tcl_GetString (nameObj);;
    char               autoObjName[64];
    pilights_clientData  *pData;
    int row;

    pData = (pilights_clientData *)ckalloc (sizeof (pilights_clientData));

    pData->nLights = nLights;
    pData->nRows = nRows;

    // allocate the array of pointers to rows
    pData->rowData = (unsigned char **) ckalloc (sizeof(unsigned char *) * nRows);

    // nRowBytes is the number of LEDs times 3 (one each for red, green,
    // and blue) plus some latch bytes on the end.
    pData->nRowBytes = (nLights * 3) + ((nLights + 31) / 32);

    for (row = 0; row < nRows; row++) {
        unsigned char *ptr;

	// allocate and clear the bytes for a row plus the latch bytes
        ptr = (unsigned char *) ckalloc (pData->nRowBytes);
	memset (ptr, 0, pData->nRowBytes);
        pData->rowData[row] = ptr;
    }

    /* if new name is "#auto", generate a unique object name */
    if (strcmp (newName, "#auto") == 0) {
	/* if Tcl_GetAssocData fails it returns NULL, we'll take that as a zero */
        unsigned long nextObjNumber = (unsigned long)Tcl_GetAssocData (interp, "pilights", NULL);

	while (1) {
	    Tcl_CmdInfo dummy;

	    snprintf (autoObjName, sizeof (autoObjName), "plights%lu", nextObjNumber++);
	    /* if autoObjName doesn't exist in the interpreter,
	     * we've got a good name
	     */
	    if (Tcl_GetCommandInfo (interp, autoObjName, &dummy) == 0) {
		newName = autoObjName;
		Tcl_SetAssocData (interp, "pilights", NULL, (ClientData) nextObjNumber);
		break;
	    }
	}
    }

    Tcl_CreateObjCommand (interp, newName, pilights_ObjectObjCmd, pData, pilights_deleteProc);

    Tcl_SetObjResult (interp, Tcl_NewStringObj (newName, -1));
    return TCL_OK;
}



/*
 *----------------------------------------------------------------------
 *
 * pilights_pilightObjCmd --
 *
 *	This procedure is invoked to process the "pilight" command.
 *	See the user documentation for details on what it does.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side effects:
 *	See the user documentation.
 *
 *----------------------------------------------------------------------
 */

    /* ARGSUSED */
int
pilights_pilightObjCmd(clientData, interp, objc, objv)
    ClientData clientData;		/* registered proc hashtable ptr. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int objc;				/* Number of arguments. */
    Tcl_Obj   *CONST objv[];
{
    int          optIndex;

    static CONST char *options[] = {
        "create",
        (char *)NULL
    };

    enum options
    {
        OPT_CREATE
    };

    if (objc < 2) {
	Tcl_WrongNumArgs (interp, 1, objv, "subcommand ?args?");
	return TCL_ERROR;
    }

    if (Tcl_GetIndexFromObj(interp, objv[1], options, "option", TCL_EXACT,
        &optIndex) != TCL_OK)
    {
	    return TCL_ERROR;
    }

    switch ((enum options) optIndex) {
      case OPT_CREATE: {
	int nLights, nRows;

	if (objc != 5) {
	    Tcl_WrongNumArgs (interp, 2, objv, "name nLights nRows");
	    return TCL_ERROR;
	}

       if (Tcl_GetIntFromObj (interp, objv[3], &nLights) == TCL_ERROR) {
	   return pilights_complainnLights (interp);
       }

       if (Tcl_GetIntFromObj (interp, objv[4], &nRows) == TCL_ERROR) {
	   return pilights_complainnRows (interp);
       }

       return pilights_newObject (interp, objv[2], nLights, nRows);
      }
    }

    return TCL_OK;
}

