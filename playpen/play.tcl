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

proc play_images {files} {
    puts "play_images called with files: $files"
    foreach file $files {
        puts $file
        set image [load_image $file]
	copy_image $image lights

	for {set i 0} {$i < 2} {incr i} {
	    lights write 0 [$image height] 128000
	}
	$image delete
	after 2000
    }
}

#lights write 10 1 5000

if {!$tcl_interactive} {
    play_images $argv
} else {
    puts "interactive, mess around like by doing copy_image $gd0 lights or lights write 0 100 5000"
}

