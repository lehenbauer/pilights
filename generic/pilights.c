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
pilightsFillRows (pilights_clientData *pData, int firstRow, int nRows, int r, int g, int b) {
    int row;
    int pixel;
    int i;
    unsigned char *rowPtr;
    unsigned char rByte, gByte, bByte;

    rByte = PIXEL_TO_LED(r);
    gByte = PIXEL_TO_LED(g);
    bByte = PIXEL_TO_LED(b);

    for (row = firstRow, i = 0; i < nRows; i++) {
        rowPtr = pData->rowData[row++];
        for (pixel = 0; pixel < pData->nLights; pixel++) {
	    *rowPtr++ = gByte;
	    *rowPtr++ = rByte;
	    *rowPtr++ = bByte;
	}

	if (row >= pData->nRows) {
	    row = 0;
	}
    }
}

static void
pilightsGradientFillRows (pilights_clientData *pData, int toColorsOfRow, int firstRow, int nRows) {
    int row;
    int i;
    int led;
    unsigned char *rowPtr;

    float *rowFloatsFrom = (float *)ckalloc (sizeof(float) * pData->nLights * 3);
    float *rowFloatsTo = (float *)ckalloc (sizeof(float) * pData->nLights * 3);

    rowPtr = pData->rowData[firstRow];
    for (i = 0; i < pData->nLights * 3; i++) {
        rowFloatsFrom[i] = LED_TO_PIXEL(pData->rowData[firstRow][i]);
        rowFloatsTo[i] = LED_TO_PIXEL(pData->rowData[toColorsOfRow][i]);
    }

    for (row = firstRow + 1, i = 1; i < nRows; i++) {
	float ratio = (float) i / (float) (nRows - 1);

        rowPtr = pData->rowData[row++];
        for (led = 0; led < pData->nLights * 3; led++) {
	    unsigned char c = rowFloatsFrom[led] * (1.0 - ratio) + rowFloatsTo[led] * ratio;
	    *rowPtr++ = PIXEL_TO_LED(c);
	}

	// don't wrap, just bail if they go off the end
	if (row >= pData->nRows) {
	    break;
	}
    }

    ckfree ((char *)rowFloatsFrom);
    ckfree ((char *)rowFloatsTo);
}

static void
pilightsFadeRows (pilights_clientData *pData, int firstRow, int nRows) {
    int row;
    int i;
    int led;
    unsigned char *rowPtr;

    float *rowFloatsFrom = (float *)ckalloc (sizeof(float) * pData->nLights * 3);

    rowPtr = pData->rowData[firstRow];
    for (led = 0; led < pData->nLights * 3; led++) {
        rowFloatsFrom[led] = (float)LED_TO_PIXEL(*rowPtr++);
    }

    for (row = firstRow + 1, i = 1; i < nRows; i++) {
        float ratio = (float) i / (float) (nRows - 1);

	// printf("fade row %d, ratio %f, data: ", row, ratio);

        rowPtr = pData->rowData[row++];
        for (led = 0; led < pData->nLights * 3; led++) {
	    unsigned char c = (unsigned char)(int)(rowFloatsFrom[led] * (1.0 - ratio));
	    // printf("%d ", c);

	    *rowPtr++ = PIXEL_TO_LED(c);
	}
	// printf("\n");

	// don't wrap, just bail if they go off the end
	if (row >= pData->nRows) {
	    break;
	}
    }

    ckfree ((char *)rowFloatsFrom);
}


static void
pilightsFillPixels (pilights_clientData *pData, int row, int firstPixel, int nPixels, int r, int g, int b) {
    int pixel, lastPixel;
    unsigned char *rowPtr;
    unsigned char rByte, gByte, bByte;

    rByte = PIXEL_TO_LED(r);
    gByte = PIXEL_TO_LED(g);
    bByte = PIXEL_TO_LED(b);

    assert ((firstPixel >= 0) && (firstPixel < pData->nLights));

    lastPixel = firstPixel + nPixels;
    if (lastPixel > pData->nLights) {
        lastPixel = pData->nLights;
    }

    rowPtr = pData->rowData[row] + firstPixel * 3;

    for (pixel = firstPixel; pixel < lastPixel; pixel++) {
	*rowPtr++ = gByte;
	*rowPtr++ = rByte;
	*rowPtr++ = bByte;
    }
}

static void
pilightsCopyRows (pilights_clientData *pData, int firstRow, int destRow, int nRows) {
    int row;
    unsigned char *rowPtr;
    unsigned char *destRowPtr;

    for (row = firstRow; row < nRows; row++, destRow++) {
        rowPtr = pData->rowData[row];
	destRowPtr = pData->rowData[destRow];

	memcpy (destRowPtr, rowPtr, pData->nRowBytes);
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
pilights_copyGDPixels (pilights_clientData *pData, int startRow, int nRows, int startY, int startX, int startPixel, int nPixels)
{
    int x, y;
    int pixelColor;
    int row = startRow;
    unsigned char *rowPtr;
    gdImagePtr im = pData->im;
    int i;

    // printf ("\nstartRow %d, nRows %d, startY %d, startX %d, startPixel %d, nPixels %d\n", startRow, nRows, startY, startX, startPixel, nPixels);

    assert (pData->im != NULL);

    if (startPixel > pData->nLights) {
        // printf("\nstartPixel > nData->nLights\n");
        return;
    }

    if (startX > im->sx) {
        // printf("\nstartX > im->sx\n");
        return;
    }

    if (startY > im->sy) {
        // printf("\nstartY > im->sy\n");
	return;
    } 

    if (startRow < 0) {
        startRow = 0;
    }

    if (startX < 0) {
        startX = 0;
    }

    if (startY < 0) {
        startY = 0;
    }

    if (startPixel + nPixels > pData->nLights) {
        nPixels = pData->nLights - startPixel;
    }

    if (im->sx - startX < nPixels) {
        nPixels = im->sx - startX;
    }

    // printf ("\nstartRow %d, nRows %d, startY %d, startX %d, startPixel %d, nPixels %d\n", startRow, nRows, startY, startX, startPixel, nPixels);

    y = startY;
    while (nRows-- > 0) {
	rowPtr = pData->rowData[row] + 3 * startPixel;
        for (i = 0, x = startX; i < nPixels; i++, x++) {
	    pixelColor = im->trueColor ? gdImageTrueColorPixel (im, x, y) : gdImagePalettePixel (im, x, y);
	    *rowPtr++ = PIXEL_TO_LED(gdImageGreen (im, pixelColor));
	    *rowPtr++ = PIXEL_TO_LED(gdImageRed (im, pixelColor));
	    *rowPtr++ = PIXEL_TO_LED(gdImageBlue (im, pixelColor));
	}
	    
        if (row++ >= pData->nRows) {
	    row = 0;
	}

	if (++y >= im->sy) {
	    y = 0;
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

/*
 * pilights_spi_write - given a pilights client data structure, first row,
 *  number of rows and delay in microseconds, write 0 or more rows to
 *  the SPI device
 */
static int
plights_spi_write (pilights_clientData *pData, int firstRow, int nRows, int delayUsecs) {
    int i, row, ret;
    struct spi_ioc_transfer spi;

    spi.delay_usecs = 0;;
    spi.rx_buf = (unsigned long) NULL;
    spi.len = pData->nRowBytes;
    spi.speed_hz = pData->spiData->writeSpeed;
    spi.bits_per_word = 8;

    // printf("plights_spi_write firstRow %d, nRows %d, delay %d\n", firstRow, nRows, delayUsecs);

    for (i = 0, row = firstRow; i < nRows; i++) {
	unsigned char *rowPtr = pData->rowData[row];

        spi.tx_buf = (unsigned long) rowPtr;

	ret = ioctl (pData->spiData->fd, SPI_IOC_MESSAGE(1), &spi);
	if (ret < 0) {
	    return ret;
	}

	ret = usleep(delayUsecs);
	if (ret < 0) {
	    return ret;
	}


	if (row++ >= pData->nRows) {
	    row = 0;
	}
    }
    return 0;
}

/*
 * pilights_close - close the connection to an SPI device if open, of
 *  if we're attached to a tclspi SPI-communicating object, forget
 *  about it.
 *
 */
void
pilights_close (pilights_clientData *pData) {
    if (pData->spiData == NULL) {
        return;
    }

    // if it was an attached tclspi object rather than something i opened,
    // just detach from it
    if (!pData->mySpiData) {
        pData->spiData = NULL;
	return;
    }

    // we opened it and allocated the spiData structure.  close and
    // get rid of it.
    close(pData->spiData->fd);
    ckfree((char *)pData->spiData);
    pData->spiData = NULL;
}

/*
 * pilights_deleteProc - free all the memory and stuff
 *
 */
void pilights_deleteProc (ClientData clientData) {
    pilights_clientData *pData = (pilights_clientData *)clientData;
    int row;

    pilights_close (pData);

    for (row = 0; row < pData->nRows; row++) {
        ckfree ((char *)pData->rowData[row]);
    }
    ckfree ((char *)pData->rowData);

    ckfree ((char *)pData);
}

/*
 * pilights_attach_spi - attach a tclspi object, from which we will get the
 *   fd to talk to the SPI bus through
 */
int
pilights_attach_spi (Tcl_Interp *interp, pilights_clientData *pData, Tcl_Obj *tclspiObj) {
    // close device if open
    pilights_close (pData);

    if (pilights_cmdNameObjToSPI (interp, tclspiObj, &pData->spiData) == TCL_ERROR) {
	return TCL_ERROR;
    }

    // mark that we shouldn't really close the device or free memory on close
    pData->mySpiData = 0;

    return TCL_OK;
}


/*
 * plights_open - open an spi device file (like /dev/spidev0.0) and
 *   set it up to talk to it
 *
 * set read and write for 8-bit, SPI mode 0, 2 MHz.  
 *
 *  If you need different settings, use tclspi to set up an object
 *
 * returns 0 on success, -1 on failure -- you can use Tcl_PosixError
 *  after a failure and you should get something reasonable
 */
int
pilights_open (pilights_clientData *pData, char *fileName) {
    tclspi_clientData *spi;

    // close device if open
    pilights_close (pData);

    pData->spiData = (tclspi_clientData *)ckalloc (sizeof(tclspi_clientData));
    pData->mySpiData = 1;

    spi = pData->spiData;

    spi->fd = open (fileName, O_RDWR);
    if (spi->fd < 0) {
        spi->fd = 0;
        return -1;
    }

    spi->readMode = SPI_MODE_0;
    spi->writeMode = SPI_MODE_0;

    spi->readBits = 8;
    spi->writeBits = 8;

    spi->readSpeed = 2000000;
    spi->writeSpeed = 2000000;

    if (ioctl (spi->fd, SPI_IOC_RD_MODE, &spi->readMode) < 0) {
        return -1;
    }

    if (ioctl (spi->fd, SPI_IOC_WR_MODE, &spi->writeMode) < 0) {
        return -1;
    }

    if (ioctl (spi->fd, SPI_IOC_RD_BITS_PER_WORD, &spi->readBits) < 0) {
        return -1;
    }

    if (ioctl (spi->fd, SPI_IOC_WR_BITS_PER_WORD, &spi->writeBits) < 0) {
        return -1;
    }

    if (ioctl (spi->fd, SPI_IOC_RD_MAX_SPEED_HZ, &spi->readSpeed) < 0) {
        return -1;
    }

    if (ioctl (spi->fd, SPI_IOC_WR_MAX_SPEED_HZ, &spi->writeSpeed) < 0) {
        return -1;
    }

    return 0;
}

/*
 * pilights_attach_gd - attach a tcl.gd object, from which we will get the
 *   GD image to read from
 */
int
pilights_attach_gd (Tcl_Interp *interp, pilights_clientData *pData, Tcl_Obj *tclgdObj) {
    gdImagePtr im;

    if (pilights_cmdNameObjToGdImagePtr (interp, tclgdObj, &im) == TCL_ERROR) {
	return TCL_ERROR;
    }

    pData->im = im;
    return TCL_OK;
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
	"setpixels",
	"clear",
	"copyrows",
	"copy_from_image",
	"gradient",
	"fade",
	"getrow",
	"setrow",
	"open",
	"close",
	"attach_gd",
	"attach_spi",
	(char *)NULL
    };

    enum options {
	OPT_WRITE,
	OPT_NLIGHTS,
	OPT_NROWS,
	OPT_FILLROWS,
	OPT_SETPIXELS,
	OPT_CLEAR,
	OPT_COPYROWS,
	OPT_COPY_FROM_IMAGE,
	OPT_GRADIENT,
	OPT_FADE,
	OPT_GETROW,
	OPT_SETROW,
	OPT_OPEN,
	OPT_CLOSE,
	OPT_ATTACH_GD,
	OPT_ATTACH_SPI
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

      case OPT_OPEN: {
	if (objc != 3) {
	    Tcl_WrongNumArgs (interp, 2, objv, "spiFileName");
	    return TCL_ERROR;
	}

	if (pilights_open (pData, Tcl_GetString (objv[2])) < 0) {
	    Tcl_AppendResult (interp, "error opening or setting up '", Tcl_GetString (objv[2]), "': ", Tcl_PosixError (interp), NULL);
	    return TCL_ERROR;
	}
	break;
      }

      case OPT_CLOSE: {
	if (objc != 2) {
	    Tcl_WrongNumArgs (interp, 2, objv, "");
	    return TCL_ERROR;
	}

	pilights_close (pData);
	break;
      }

      case OPT_ATTACH_GD: {
	if (objc != 3) {
	    Tcl_WrongNumArgs (interp, 2, objv, "tclgdObject");
	    return TCL_ERROR;
	}

	if (pilights_attach_gd (interp, pData, objv[2]) == TCL_ERROR) {
	    return TCL_ERROR;
	}
	break;
      }


      case OPT_ATTACH_SPI: {
	if (objc != 3) {
	    Tcl_WrongNumArgs (interp, 2, objv, "tclspiObject");
	    return TCL_ERROR;
	}

	if (pilights_attach_spi (interp, pData, objv[2]) == TCL_ERROR) {
	    return TCL_ERROR;
	}
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

       if (Tcl_GetIntFromObj (interp, objv[4], &r) == TCL_ERROR) {
	   return pilights_complainr (interp);
       }

       if (Tcl_GetIntFromObj (interp, objv[5], &g) == TCL_ERROR) {
	   return pilights_complaing (interp);
       }

       if (Tcl_GetIntFromObj (interp, objv[6], &b) == TCL_ERROR) {
	   return pilights_complainb (interp);
       }

	pilightsFillRows (pData, firstRow, nRows, r, g, b);
	break;
      }

      case OPT_CLEAR: {
	int firstRow, nRows = 1;

	if ((objc < 3) || (objc > 4)) {
	    Tcl_WrongNumArgs (interp, 2, objv, "firstRow ?nRows?");
	    return TCL_ERROR;
	}

       if (Tcl_GetIntFromObj (interp, objv[2], &firstRow) == TCL_ERROR) {
	   return pilights_complainfirstRow (interp);
       }

       if (objc == 4) {
	   if (Tcl_GetIntFromObj (interp, objv[3], &nRows) == TCL_ERROR) {
	       return pilights_complainnRows (interp);
	   }
	}

	pilightsFillRows (pData, firstRow, nRows, 0, 0, 0);
	break;
      }

      case OPT_SETPIXELS: {
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

       if (firstPixel < 0) {
           firstPixel = 0;
       } else if (firstPixel >= pData->nLights) {
	    Tcl_AppendResult (interp, "Error: first pixel is out of range of defined lights", NULL);
	    return TCL_ERROR;
       }

	pilightsFillPixels (pData, row, firstPixel, nPixels, r, g, b);
	break;
      }

      case OPT_WRITE: {
	int delay = 0;
	int nRows = 1;
	int firstRow;
	int ret;


	if ((objc < 3) || (objc > 5)) {
	    Tcl_WrongNumArgs (interp, 2, objv, "firstRow ?nRows? ?delay?");
	    return TCL_ERROR;
	}

       if (Tcl_GetIntFromObj (interp, objv[2], &firstRow) == TCL_ERROR) {
	   return pilights_complainfirstRow (interp);
       }

       if (objc > 3) {
	   if (Tcl_GetIntFromObj (interp, objv[3], &nRows) == TCL_ERROR) {
	       return pilights_complainnRows (interp);
	   }
       }

       if (objc > 4) {
	   if (Tcl_GetIntFromObj (interp, objv[4], &delay) == TCL_ERROR) {
	       return pilights_complaindelay (interp);
	   }
       }

       if (pData->spiData == NULL) {
	    Tcl_AppendResult (interp, "must open an SPI device or attach a tclspi object using attach_spi before attempting to write", NULL);
	    return TCL_ERROR;
       }

	ret = plights_spi_write (pData, firstRow, nRows, delay);
	if (ret < 0) {
	    Tcl_AppendResult (interp, "can't perform spi transfer: ", Tcl_PosixError (interp), NULL);
	    return TCL_ERROR;
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

	pilightsCopyRows (pData, firstRow, destRow, nRows);
	break;
      }

      case OPT_COPY_FROM_IMAGE: {
	int firstRow;
	int firstPixel, nPixels, nRows;
	int x, y;

	if (objc != 8) {
	    Tcl_WrongNumArgs (interp, 2, objv, "firstRow nRows x y firstPixel nPixels");
	    return TCL_ERROR;
	}

	if (pData->im == NULL) {
	    Tcl_AppendResult (interp, "attach a tclgd image using attach_gd first", NULL);
	    return TCL_ERROR;
	}

       if (Tcl_GetIntFromObj (interp, objv[2], &firstRow) == TCL_ERROR) {
	   return pilights_complainfirstRow (interp);
       }

       if (Tcl_GetIntFromObj (interp, objv[3], &nRows) == TCL_ERROR) {
	   return pilights_complainnRows (interp);
       }

       if (Tcl_GetIntFromObj (interp, objv[4], &x) == TCL_ERROR) {
	   return pilights_complainx (interp);
       }

       if (Tcl_GetIntFromObj (interp, objv[5], &y) == TCL_ERROR) {
	   return pilights_complainy (interp);
       }

       if (Tcl_GetIntFromObj (interp, objv[6], &firstPixel) == TCL_ERROR) {
	   return pilights_complainfirstPixel (interp);
       }

       if (Tcl_GetIntFromObj (interp, objv[7], &nPixels) == TCL_ERROR) {
	   return pilights_complainnPixels (interp);
       }

	pilights_copyGDPixels (pData, firstRow, nRows, y, x, firstPixel, nPixels);
	break;
      }

      case OPT_GRADIENT: {
	int toColorsOfRow;
	int firstRow;
	int nRows;

	if (objc != 5) {
	    Tcl_WrongNumArgs (interp, 2, objv, "toColorsOfRow firstRow nRows");
	    return TCL_ERROR;
	}

	if (pData->im == NULL) {
	    Tcl_AppendResult (interp, "attach a tclgd image using attach_gd first", NULL);
	    return TCL_ERROR;
	}

	if (Tcl_GetIntFromObj (interp, objv[2], &toColorsOfRow) == TCL_ERROR) {
	   return pilights_complainfirstRow (interp);
	}

	if (Tcl_GetIntFromObj (interp, objv[3], &firstRow) == TCL_ERROR) {
	   return pilights_complainfirstRow (interp);
	}

	if (Tcl_GetIntFromObj (interp, objv[4], &nRows) == TCL_ERROR) {
	    return pilights_complainnRows (interp);
	}

	pilightsGradientFillRows (pData, toColorsOfRow, firstRow, nRows);
	break;
      }

      case OPT_FADE: {
	int firstRow;
	int nRows;

	if (objc != 4) {
	    Tcl_WrongNumArgs (interp, 2, objv, "firstRow nRows");
	    return TCL_ERROR;
	}

	if (pData->im == NULL) {
	    Tcl_AppendResult (interp, "attach a tclgd image using attach_gd first", NULL);
	    return TCL_ERROR;
	}

	if (Tcl_GetIntFromObj (interp, objv[2], &firstRow) == TCL_ERROR) {
	   return pilights_complainfirstRow (interp);
	}

	if (Tcl_GetIntFromObj (interp, objv[3], &nRows) == TCL_ERROR) {
	    return pilights_complainnRows (interp);
	}

	pilightsFadeRows (pData, firstRow, nRows);
	break;
      }

      case OPT_GETROW: {
        Tcl_Obj **rowList = (Tcl_Obj **)ckalloc (sizeof(Tcl_Obj *) * pData->nLights * 3);
	Tcl_Obj **rlp;
	unsigned char *rp;
	int row;
	int i;

	if (objc != 3) {
	    Tcl_WrongNumArgs (interp, 2, objv, "row");
	    return TCL_ERROR;
	}

        if (Tcl_GetIntFromObj (interp, objv[2], &row) == TCL_ERROR) {
	   return pilights_complainrow (interp);
        }

	for (i = 0, rp = pData->rowData[row], rlp = rowList; i < pData->nLights; i++) {
	    int r, g, b;

	    g = LED_TO_PIXEL(*rp++);
	    r = LED_TO_PIXEL(*rp++);
	    b = LED_TO_PIXEL(*rp++);

	    *rlp++ = Tcl_NewIntObj (r);
	    *rlp++ = Tcl_NewIntObj (g);
	    *rlp++ = Tcl_NewIntObj (b);
	}

	Tcl_SetObjResult (interp, Tcl_NewListObj (pData->nLights * 3, rowList));
	ckfree ((char *)rowList);
	break;
      }

      case OPT_SETROW: {
	unsigned char *rp;
	int row;
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
	    Tcl_AppendResult (interp, "List must contain at least three elements", NULL);
	    return TCL_ERROR;
	}

	if (listObjc % 3 != 0) {
	    Tcl_AppendResult (interp, "List length must be a multiple of three", NULL);
	    return TCL_ERROR;
	}

	for (pixel = 0, li = 0, rp = pData->rowData[row]; pixel < pData->nLights; pixel++) {
	    int r, g, b;

	    if (Tcl_GetIntFromObj (interp, listObjv[li++], &r) == TCL_ERROR) {
	        return TCL_ERROR;
	    }

	    if (Tcl_GetIntFromObj (interp, listObjv[li++], &g) == TCL_ERROR) {
	        return TCL_ERROR;
	    }

	    if (Tcl_GetIntFromObj (interp, listObjv[li++], &b) == TCL_ERROR) {
	        return TCL_ERROR;
	    }

	    *rp++ = PIXEL_TO_LED(g);
	    *rp++ = PIXEL_TO_LED(r);
	    *rp++ = PIXEL_TO_LED(b);

	    if (li >= listObjc) {
	        li = 0;
	    }
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
    pData->spiData = NULL;
    pData->mySpiData = 0;
    pData->im = NULL;

    // allocate the array of pointers to rows
    pData->rowData = (unsigned char **) ckalloc (sizeof(unsigned char *) * nRows);

    // nRowBytes is the number of LEDs times 3 (one each for red, green,
    // and blue) plus some latch bytes on the end.
    pData->nRowBytes = (nLights * 3) + ((nLights + 31) / 32);

    for (row = 0; row < nRows; row++) {
        unsigned char *ptr;

	// allocate and clear the bytes for a row plus the latch bytes
        ptr = (unsigned char *) ckalloc (pData->nRowBytes);

	// NB we really only have to clear the latch bytes at the end or
	// even make pilightsFillRows do that and let it happen below
	memset (ptr, 0, pData->nRowBytes);
        pData->rowData[row] = ptr;
    }

    // initialize all rows to black
    pilightsFillRows (pData, 0, nRows, 0, 0, 0);

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

