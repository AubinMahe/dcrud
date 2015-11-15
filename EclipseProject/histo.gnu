clear
reset
set key off
set border 1
set auto
set xtics 50
set title activity
set xlabel "200 ms Cycle"
set ylabel "Elapsed time in µs"
set terminal png enhanced font arial 12 size 1200, 900
set output activity.'.png'
set style histogram clustered gap 1
set style fill solid border -1
binwidth=2
set boxwidth binwidth
bin(x,width)=width*floor(x/width) + binwidth
plot activity.'.dat' using (bin($1,binwidth)) smooth freq with boxes
