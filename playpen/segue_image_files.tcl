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

    puts "$command $file"
    set cmd [GD $command #auto $fp]

    close $fp

    return $cmd
}

proc fit_image {gd} {
    set height [$gd height]
    set width [$gd width]

    if {$width == 160} {
        return $gd
    }

    set newWidth $::nLEDs
    set newHeight $height

    set newGD [GD create #auto $newHeight $newWidth]
    $newGD copy_resampled $gd 0 0 0 0 $newWidth $newHeight $width $height
    $gd delete

    return $newGD
}

#set image [load_image symmetric/400-04672846w.png]


proc setup {} {
    pilight create lights $::nLEDs $::nRows

    lights open /dev/spidev0.0
}

proc copy_image {gdObject piObject} {
    set newGdObject [fit_image $gdObject]
    $piObject attach_gd $gdObject

     $piObject copy_from_image 0 [$gdObject height] 0 0 0 [$piObject nLights]
}

proc ldraw {_L} {
    upvar 1 $_L L
    set pos [expr {int(rand()*[llength $L])}]
    set res [lindex $L $pos]
    set L [lreplace $L $pos $pos]
    set res
}

proc play_images {files} {
    puts "play_images called with files: $files"
    #foreach file $files
    set firstPass 1
    while {true} {
        set file [ldraw files]
	if {$file == ""} {
	    break
	}
        puts $file

        set image [load_image $file]
	set height [$image height]

	set newImage [fit_image $image]

	lights attach_gd $newImage

	 # copy the image offset by the fade steps
	 lights copy_from_image $::fadeSteps $height 0 0 0 [lights nLights]

	$newImage delete

	 # if it's the first pass
	 # create a gradient from black to the first line of data (fade in)
	if {$firstPass} {
	    lights clear 0
	    set firstPass 0
	} else {
	     # copy the last line from the prior image
	     lights copyrows $priorLast 0 1

	}
	set priorLast [expr {$::fadeSteps + $height}]

	lights gradient $::fadeSteps 0 $::fadeSteps

	# play the fade in (gradient from 0 to the first line of pixels)
	lights write 0 $::fadeSteps $::delayUsecs

	# play the image
	lights write $::fadeSteps $height $::delayUsecs
    }
}

if {!$tcl_interactive} {
    setup
    play_images $argv
} else {
    puts "interactive, mess around like by doing play_images file or lights write 0 100 5000"
}

