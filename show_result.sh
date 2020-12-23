for((i=0;i<6;i++))
do
{
    suffix=`expr $i \* 5 + 5` 
    echo $suffix
    echo ""
    tail -n 5 log/as$suffix.log
    echo ""
    tail -n 3 log/dc$suffix.log
    echo ""
    tail -n 4 log/li$suffix.log
} 
done
wait