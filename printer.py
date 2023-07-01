# -*- coding: utf-8 -*-
"""
Created on Wed Sep 30 23:05:51 2020

@author: Florian
"""

import win32print
import win32ui
from PIL import Image, ImageWin
import sys

#
# Constants for GetDeviceCaps
#
#
# HORZRES / VERTRES = printable area
#
HORZRES = 8
VERTRES = 10
#
# PHYSICALWIDTH/HEIGHT = total area
#
PHYSICALWIDTH = 110
PHYSICALHEIGHT = 111
#
# PHYSICALOFFSETX/Y = left / top margin
#
PHYSICALOFFSETX = 112
PHYSICALOFFSETY = 113

SIZE = (6, 4)  # in inch
RESOLUTION = 300  # ppi

WTMRK = r"ressources\logo_blanc_sur_transparent.png"


def printer(photoPath, nbPrint, rotate=False, watermark=False):

    print("photoPath:", photoPath, " - nbPrint:", nbPrint, " - rotate:", rotate, " - watermark:", watermark)
    
    image = Image.open(photoPath)
    width = SIZE[0] * RESOLUTION
    height = SIZE[1] * RESOLUTION
    image.thumbnail((width, height), Image.ANTIALIAS)
    

    printer_name = win32print.GetDefaultPrinter()
    
    hDC = win32ui.CreateDC()
    hDC.CreatePrinterDC(printer_name)
    printable_area = hDC.GetDeviceCaps(HORZRES), hDC.GetDeviceCaps(VERTRES)
    printer_size = hDC.GetDeviceCaps(PHYSICALWIDTH), hDC.GetDeviceCaps(PHYSICALHEIGHT)
    # printer_margins = hDC.GetDeviceCaps (PHYSICALOFFSETX), hDC.GetDeviceCaps (PHYSICALOFFSETY)

    #
    # Open the image, rotate it if it's wider than
    #  it is high, and work out how much to multiply
    #  each pixel by to get it as big as possible on
    #  the page without distorting.
    #
    if rotate:
        image = image.rotate(180)
    
    transparent = Image.new('RGBA', (width, height), (0, 0, 0, 0))    
    if watermark:
        watermark = Image.open(WTMRK)
        transparent.paste(image, (0, 0))
        pos_x = SIZE[0] * RESOLUTION - watermark.size[0] - 20
        pos_y = SIZE[1] * RESOLUTION - watermark.size[1] - 40
        transparent.paste(watermark, (pos_x, pos_y), mask=watermark)
    else:
        transparent.paste(image, (0, 0))

    ratios = [1.0 * printable_area[0] / transparent.size[0], 1.0 * printable_area[1] / transparent.size[1]]
    scale = min(ratios)


    for _ in range(nbPrint):
        # Start the print job, and draw the bitmap to
        #  the printer device at the scaled size.
        #
        hDC.StartDoc(photoPath)
        hDC.StartPage()

        dib = ImageWin.Dib(transparent)
        scaled_width, scaled_height = [int(scale * i) for i in transparent.size]
        x1 = int((printer_size[0] - scaled_width) / 2)
        y1 = int((printer_size[1] - scaled_height) / 2)
        x2 = x1 + scaled_width
        y2 = y1 + scaled_height
        dib.draw(hDC.GetHandleOutput(), (x1, y1, x2, y2))

        hDC.EndPage()
        hDC.EndDoc()
        
    hDC.DeleteDC()


if __name__ == '__main__':
    photoPath = "C:\Photos_PhotoBooth\DSC00113.JPG"
    
    printer(photoPath, 2, True, True)
