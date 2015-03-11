set terminal postscript
set terminal postscript "Times-Roman" 12
set output "diagramm1.ps"

#set multiplot

set lmargin 15
set rmargin 15

set y2tics
set ytics nomirror

set xlabel "Zeit (s)"
set ylabel "Framerate (fps)"
set y2label "Dreiecke pro Frame"

plot [:] [0:] [:] [0:] "benchmark.txt" index 0 using 1:2 title "Framerate" linetype rgb "red" lw 2 with lines\
, "benchmark.txt" index 0 using 1:3 title "Dreiecke pro Frame" linetype rgb "blue" lw 2 with lines axis x1y2

set xlabel "Zeit (s)"
set ylabel "Datentransferrate (MB/s)"
set y2label "Bewegungsgeschwindigkeit (m/s)"

set output "diagramm2.ps"

plot [:] [0:] [:] [0:] "benchmark.txt" index 1 using 1:2 title "Datentransferrate" linetype rgb "magenta" lw 2 with fsteps\
, "benchmark.txt" index 0 using 1:4 title "Bewegungsgeschwindigkeit" linetype rgb "black" lw 2 with lines axis x1y2

#unset multiplot

reset