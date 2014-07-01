
#
# make_image - given a height and a width, make an image of that size with
#  GD and set height and width variables for future reference
#
proc make_image {myHeight myWidth} {
    variable im
    variable height
    variable width

    set height $myHeight
    set width $myWidth

    set im [GD create #auto $height $width]
}

#
# x_to_pixel - given an x between 0.0 and 1.0, return the corresponding
#  pixel based on the width
#
proc x_to_pixel {x} {
    variable width

    return [expr {round($x * $width)}]
}

#
# y_to_pixel - given a y between 0.0 and 1.0, return the corresponding
#  pixel based on the height
#
proc y_to_pixel {y} {
    variable height

    return [expr {round($y * $height)}]
}

#
# xy_to_pixel - given an x and y between 0.0 and 1.0 and two varaibles,
#  set into the variables the equivalent position in pixels based on
#  the width and height
#
proc xy_to_pixel {x y _pixX _pixY} {
    upvar $_pixX pixX $_pixY pixY
    variable height
    variable width

    set pixX [expr {round($x * $width)}]
    set pixY [expr {round($y * $height)}]

    return
}

#
# symline - draw a line with bilateral symmetry
#
proc symline {x1 y1 x2 y2 color} {
    variable im

    xy_to_pixel $x1 $y1 px1 py1
    xy_to_pixel $x2 $y2 px2 py2

    $im line $px1 $py1 $px2 $py2 $color

    $im line [expr {$width - $px1}] $py1 [expr {$width - $px2}] $py2 $color
}


