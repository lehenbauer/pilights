#
#
#

proc move_left_right {startRow nRows startPixel nPixels pixelIncrement r g b} {
    for {set row $startRow} {$row < $startRow + $nRows} {incr row} {
        lights setpixels $row $startPixel $nPixels $r $g $b
	incr startPixel $pixelIncrement
    }
}

proc sprinkle {startRow nRows nPixels r g b} {
    puts "sprinkle startRow $startRow nRows $nRows"
    for {set row $startRow} {$row < $startRow + $nRows} {incr row} {
        for {set i 0} {$i < $nPixels} {incr i} {
	    set pixel [expr {int(rand() * [lights nLights])}]
	    lights setpixels $row $pixel 1 $r $g $b
	}
    }
}


proc hsv_to_rgb {hue sat value} {
    set v [format %.0f [expr {255.0*$value}]]
    if {$sat == 0} {
	return "$v $v $v"
    } else {
	set hue [expr {$hue*6.0}]
	if {$hue >= 6.0} {
	    set hue 0.0
	}
	scan $hue. %d i
	set f [expr {$hue-$i}]
	set p [format %.0f [expr {255.0*$value*(1 - $sat)}]]
	set q [format %.0f [expr {255.0*$value*(1 - ($sat*$f))}]]
	set t [format %.0f [expr {255.0*$value*(1 - ($sat*(1 - $f)))}]]
	switch $i {
	    0 {return "$v $t $p"}
	    1 {return "$q $v $p"}
	    2 {return "$p $v $t"}
	    3 {return "$p $q $v"}
	    4 {return "$t $p $v"}
	    5 {return "$v $p $q"}
	    default {error "i value $i is out of range"}
	}
    }
}

proc rampto {h s v steps} {
    variable rampH
    variable rampS
    variable rampV
    variable rampRow

    puts "rampto h $h s $s v $v steps $steps"

    if {![info exists rampH]} {
        set rampH $h
	set rampS $s
	set rampV $v
	return
    }

    if {![info exists rampRow]} {
        set rampRow 0
    }

    set rampHincr [expr {double($h - $rampH) / $steps}]
    set rampSincr [expr {double($s - $rampS) / $steps}]
    set rampVincr [expr {double($v - $rampV) / $steps}]

    for {set i 0} {$i < $steps} {incr i} {
        lassign [hsv_to_rgb $rampH $rampS $rampV] r g b
        lights fillrows $rampRow 1  $r $g $b
	incr rampRow

	set rampH [expr {$rampH + $rampHincr}]
	set rampS [expr {$rampS + $rampSincr}]
	set rampV [expr {$rampV + $rampVincr}]
    }
}

proc rampart {nRamps nSteps} {
    puts "rampart, $nRamps ramps with $nSteps steps each"
    for {set ramp 0} {$ramp < $nRamps} {incr ramp} {
	set h [expr {rand()}]
	set s [expr {rand()}]
	set v 0.05

	rampto $h $s $v $nSteps
    }
}

proc play {} {
    for {set row 0} {$row < [lights nRows]} {incr row} {
        lights write $row 1 5000
    }
}


proc sinwave {row nRows startRadians increment centerPixel pixelWidth r g b} {
    puts "sinwave row $row, nRows $nRows, startRadians $startRadians, increment $increment, centerPixel $centerPixel, pixelWidth $pixelWidth, r $r, g $g, b $b"
    set half [expr {$pixelWidth / 2}]
    set radians $startRadians
    for {set i $row} {$i < $nRows} {incr i} {
        set pixel [expr {$centerPixel + int(sin($radians) * $half)}]
	lights setpixels $row $pixel 1 $r $g $b
	incr row
	set radians [expr $radians + $increment]
    }
}
