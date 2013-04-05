#
# play_image_files - play a series of image files from the command line
#  to the LEDs
#

set delayUsecs 8000
set nLoops 4
set nLEDs 160
set nRows 5000
set delayBetweenFilesMS 500

package require tclgd
package require pilights

proc load_image {file} {
    switch [file extension $file] {
        ".PNG" -
        ".png" {
	    set command create_from_png
	}

	".GIF" -
	".gif" {
	    set command create_from_gif
	}

	".jpeg" -
	".JPEG" -
	".JPG" -
	".jpg" {
	    set command create_from_jpeg
	}
    }

    set fp [open $file]
    fconfigure $fp -translation binary -encoding binary

    set cmd [GD $command #auto $fp]

    close $fp

    return $cmd
}

#set image [load_image symmetric/400-04672846w.png]


proc setup {} {
    pilight create lights $::nLEDs $::nRows

    lights open /dev/spidev0.0
}

proc copy_image {gdObject piObject} {
    $piObject attach_gd $gdObject

     $piObject copy_from_image 0 [$piObject nRows] 0 0 0 [$piObject nLights]
}

proc play_images {files} {
    puts "play_images called with files: $files"
    foreach file $files {
        puts $file
        set image [load_image $file]
	copy_image $image lights

	for {set i 0} {$i < $::nLoops} {incr i} {
	    lights write 0 [$image height] $::delayUsecs
	}
	$image delete

	lights clear 0
	lights write 0 1
	after $::delayBetweenFilesMS
    }
}

setup

if {!$tcl_interactive} {
    play_images $argv
} else {
    puts "interactive, mess around like by doing copy_image $gd lights or lights write 0 100 5000"
}

