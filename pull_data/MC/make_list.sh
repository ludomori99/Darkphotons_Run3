while IFS= read -r line; do
 ls "$line"
#  echo $line
done < "dataset.txt" 
