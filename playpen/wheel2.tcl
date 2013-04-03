

package require tclspi
package require pilights

set nLEDs 160

pilight create lights $nLEDs 1000

lights open /dev/spidev0.0

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

    if {![info exists rampH]} {
        set rampH $h
	set rampS $s
	set rampV $v
	return
    }

    set rampHincr [expr {double($h - $rampH) / $steps}]
    set rampSincr [expr {double($s - $rampS) / $steps}]
    set rampVincr [expr {double($v - $rampV) / $steps}]

    for {set i 0} {$i < $steps} {incr i} {
        lassign [hsv_to_rgb $rampH $rampS $rampV] r g b
        lights fillrows 10 1  $r $g $b
	lights write 10 1 5000
        

	set rampH [expr {$rampH + $rampHincr}]
	set rampS [expr {$rampS + $rampSincr}]
	set rampV [expr {$rampV + $rampVincr}]
    }
}

proc rampalong {} {
    while 1 {
	set h [expr {rand()}]
	set s [expr {rand()}]
	set v [expr {rand()}]

	puts "ramping to h $h, s $s, v $v"

	rampto $h $s $v 500
    }
}

proc rampart {} {
    while 1 {
	set h [expr {rand()}]
	set s [expr {rand()}]
	set v 0.1

	puts "ramping to h $h, s $s, v $v"

	rampto $h $s $v 500
    }
}
