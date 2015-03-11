set terminal postscript
set terminal postscript "Times-Roman" 12
set output "diagramm3.ps"

#set multiplot

set lmargin 15
set rmargin 15

set y2tics
set ytics nomirror

set xlabel "Zeit (s)"
set ylabel "Bus Transfer Tiles (Tiles/s)"
set y2label "Bus Transfer Triangles (Triangles/s)"

plot [:] [0:] [:] [0:] "benchmark.txt" index 0 using 1:5 title "Bus Transfer Tiles" linetype rgb "red" lw 2 with lines\
, "benchmark.txt" index 0 using 1:6 title "Bus Transfer Triangles" linetype rgb "blue" lw 2 with lines axis x1y2

set xlabel "Zeit (s)"
set ylabel "Bus Transfer Geometry Data (MB/s)"
set y2label "Bus Transfer Texture Data (MB/s)"

set output "diagramm4.ps"

plot [:] [0:] [:] [0:] "benchmark.txt" index 0 using 1:7 title "Bus Transfer Geometry Data" linetype rgb "magenta" lw 2 with lines\
, "benchmark.txt" index 0 using 1:8 title "Bus Transfer Texture Data" linetype rgb "black" lw 2 with lines axis x1y2

#unset multiplot

reset