#!/bin/bash 

# bash ./dataGenerator.sh cities.txt types.txt directoryPath 10 10

if ! [ $# -eq 5 ]
  then
    echo "Invalid number of arguments, expected 5, got $#."
fi


# Check cities file
CITIES_FILE=$1

if [[ -r $CITIES_FILE ]]; then
  # do stuff
  echo  "Reading cities file..."
  cities_arr=()
  readarray -t cities_arr < $CITIES_FILE
  # printf '* %s\n' "${cities_arr[@]}"
else
  # file is either not readable or writable or both
  echo "Error reading cities file at path: $CITIES_FILE"
  exit 1
fi


# Check types file
TYPES_FILE=$2

if [[ -r $TYPES_FILE ]]; then
  # do stuff
  echo "Reading types file..."
  types_arr=()
  readarray -t types_arr < $TYPES_FILE
  # printf '* %s\n' "${types_arr[@]}"
else
  # file is either not readable or writable or both
  echo "Error reading types file at path: $2"
  exit 1
fi

#Create working directory for scripts
DIRECTORY_PATH=$3
rm -rf $DIRECTORY_PATH
mkdir -p $DIRECTORY_PATH
if [[ -r $DIRECTORY_PATH && -w $DIRECTORY_PATH ]]; then
  # do stuff
  echo "Working on directy at path: $DIRECTORY_PATH"
else
  # file is either not readable or writable or both
  echo "Error reading/writing directory at path: $DIRECTORY_PATH"
fi

#Create sub-directories for each city

for cityName in ${cities_arr[*]}; do
  mkdir -p $DIRECTORY_PATH/$cityName
done

FILES_PER_DIRECTORY=$4
echo "Files per directory: $FILES_PER_DIRECTORY"

ROWS_PER_FILE=$5
echo "Rows per file: $ROWS_PER_FILE"

transaction_id=0
charsU='ABCDEFGHIJKLMNOPQRSTUVWXYZ'


for cityName in ${cities_arr[*]}; do
  for i in `seq 1 $FILES_PER_DIRECTORY`; do
    dateFlag=1
    while [ $dateFlag -le 2 ]
    do 
      days=$(( $RANDOM % 30 + 1 ))
      months=$(( $RANDOM % 12 + 1 ))
      years=$(( $RANDOM % 200 + 2000 ))
      printf  -v RAND_DATE "%s%s%s" "${days}-${months}-${years}"
      dateFlag=10
      if test -f "$DIRECTORY_PATH/$cityName/$RAND_DATE"; then
      	echo "File exists"
        dateFlag = 1;
      fi
    done

    touch $DIRECTORY_PATH/$cityName/$RAND_DATE

    for k in `seq 1 $ROWS_PER_FILE`; do
      random_type_index=$[$RANDOM % ${#types_arr[@]}]
      random_type="${types_arr[$random_type_index]}"
      transaction_id=$((transaction_id+1))
      firstStr=""
      lastStr=""
      randii=$(( $RANDOM % 10 + 3 ))
      for (( i=1; i<=$randii; i++)) ; do	
          lastStr="${charsU:RANDOM%${#charsU}:1}$lastStr"
      done

      randomStreetName=$firstStr$lastStr

      surface_sqm=$(( $RANDOM % 1000 + 35 ))
      price=$(( $RANDOM % 10000000 + 100000 ))

      fullRow="$transaction_id $random_type $randomStreetName $surface_sqm $price"
      echo $fullRow >> $DIRECTORY_PATH/$cityName/$RAND_DATE
    done
  done   
done

echo "Generated files."
