set terminal png font "Times-New-Roman, 14"
t=0.00
set output sprintf( 'heatmap_%.2f.png' , t )
set title sprintf( 'heatmap when randomization threshold = %.2f' , t )
set view map scale 1
set xrange [-1:1050]
set yrange [-1:1024]

fname = sprintf( 'heatmap_nds_128_tpr_4_thh_%f_tks_128.txt' , t )
splot fname matrix with image notitle
