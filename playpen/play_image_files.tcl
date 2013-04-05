#
# play_image_files - play a series of image files from the command line
#  to the LEDs
#

set delayUsecs 8000
set nLoops 4
set nLEDs 160
set nRows 5000
set delayBetweenFilesMS 500
set fadeSteps 40

set nLoops 2
set delayUsecs 32000

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

     $piObject copy_from_image 0 [$gdObject height] 0 0 0 [$piObject nLights]
}

proc play_images {files} {
    puts "play_images called with files: $files"
    foreach file $files {
        puts $file

        set image [load_image $file]
	set height [$image height]
	lights attach_gd $image

	 lights copy_from_image $::fadeSteps $height 0 0 0 [lights nLights]

	$image delete

	 # create a gradient from black to the first line of data (fade in)
	 lights clear 0
	 lights gradient $::fadeSteps 0 $::fadeSteps

	# create a fade out at the end
	set lastLine [expr {$height + $::fadeSteps - 1}]
	lights fade $lastLine $::fadeSteps

	# play the fade in (gradient from 0 to the first line of pixels)
	lights write 0 $::fadeSteps $::delayUsecs

	# loop playing the image
	for {set i 0} {$i < $::nLoops} {incr i} {
	    lights write $::fadeSteps $height $::delayUsecs
	}

	# play fade out
	lights write $lastLine $::fadeSteps $::delayUsecs

	after $::delayBetweenFilesMS
    }
}

setup

if {!$tcl_interactive} {
    play_images $argv
} else {
    puts "interactive, mess around like by doing play_images file or lights write 0 100 5000"
}

