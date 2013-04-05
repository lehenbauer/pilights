#
#
#

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

set image [load_image symmetric/400-04672846w.png]

set nLEDs 160

pilight create lights $nLEDs 1000

lights open /dev/spidev0.0

proc copy_image {gdObject piObject} {
    $piObject attach_gd $gdObject

     $piObject copy_from_image 0 [$piObject nRows] 0 0 0 160
}

#lights write 10 1 5000

if {!$tcl_interactive} {
    puts "foo"
} else {
    puts "'sinwave' to see something"
}

