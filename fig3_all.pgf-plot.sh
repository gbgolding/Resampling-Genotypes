awk '/^[ 0-9]*/ {if($1!=x&&length(x)!=0){print x, y/n;x=$1;y=$3;n=1} else{x=$1;y+=$3;n+=1;}} END{print x,y/n}' sex_avg_1000
