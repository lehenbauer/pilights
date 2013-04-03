

package require tclspi
package require pilights

set nLEDs 160

pilight create lights $nLEDs 1000

lights open /dev/spidev0.0

 proc hsv_to_rgb {h s v _r _g _b} {
    upvar $_r r $_g g $_b b

    set Hi [expr { int( double($h) / 60 ) % 6 }]
    set f [expr { double($h) / 60 - $Hi }]
    set s [expr { double($s)/255 }]
    set v [expr { double($v)/255 }]
    set p [expr { double($v) * (1 - $s) }]
    set q [expr { double($v) * (1 - $f * $s) }]
    set t [expr { double($v) * (1 - (1 - $f) * $s) }]
    switch -- $Hi {
        0 {
            set r $v
            set g $t
            set b $p
        }
        1 {
            set r $q
            set g $v
            set b $p
        }
        2 {
            set r $p
            set g $v
            set b $t
        }
        3 {
            set r $p
            set g $q
            set b $v
        }
        4 {
            set r $t
            set g $p
            set b $v
        }
        5 {
            set r $v
            set g $p
            set b $q
        }
        default {
            error "Wrong Hi value in hsvToRgb procedure! This should never happen!"
        }
    }
    set r [expr {round($r*255)}]
    set g [expr {round($g*255)}]
    set b [expr {round($b*255)}]
 }

proc splatter {row centerPixel nRows r g b} {
    set increment 2.0
    set multiplier 0.99
    set width 1.0
    for {set i 0} {$i < $nRows} {incr i} {
        set dWidth [expr {int($width)}]
	lights setpixels $row [expr {$centerPixel - $dWidth / 2}] $dWidth $r $g $b
	set width [expr {$width + $increment}]
	set increment [expr {$increment * $multiplier}]
	lights write $row 1 5000
	incr row
    }
}

proc suckup {row centerPixel width nRows r g b} {
    set increment 1.0
    set multiplier 1.01
    for {set i 0} {$i < $nRows} {incr i} {
        set dWidth [expr {int($width)}]
	lights clear $row
	lights setpixels $row [expr {$centerPixel - $dWidth / 2}] $dWidth $r $g $b
	set width [expr {$width - $increment}]
	if {$width == 0} {
	    break
	}
	set increment [expr {$increment * $multiplier}]
	lights write $row 1 5000
	incr row
    }
}

if {!$tcl_interactive} {
    sinwave
} else {
    puts "'sinwave' to see something"
}
 
