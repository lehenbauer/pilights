#
#
#

set nLEDs 960
set nRows 7680

set gdi [GD create #auto $::nLEDs $::nRows]
set photo [image create photo]
button .b -image photo
pack .b

proc refresh_image {} {
    $::photo configure -data [$gd png_data]
}


