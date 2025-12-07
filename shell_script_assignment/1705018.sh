#!/bin/bash

#1. If in ignore funtion:

InIgnore()
{
    extension=$1
    for i in ${!ARRAY[@]};
    do
        if [[ "${ARRAY[$i]}" = "${extension}" ]];
        then
            return 0;
        fi
    done

    return 1;
}

#2. recursively traverse all directory

traverse()
{
    cd "$1"

    for f in *
    do
        if [ -d "$f" ]; then

            traverse "$f" "$2/$f"
        elif [ -f "$f" ]; then
            if [[ "$f" == *"."* ]]; then
                new_folder=`echo "$f" | cut -d '.' -f 2-`
            else
                new_folder="others"
            fi  

            if InIgnore $new_folder; then
                ignore_count=$((ignore_count+1))
            else
                #echo "$new_folder not in ignore"

                new_directory="$output_directory/$new_folder/"
                current_directory=`pwd`
                current_directory="$current_directory/$f"
                #echo "current directory: $current_directory"
                #echo "new directory: $new_directory"
                desc_directory="$new_directory/desc_$new_folder.txt"

                if [ -d $new_directory ]; then
                    existing_directory="$new_directory/$f"
                    if [ ! -f $existing_directory ]; then
                        cp $current_directory $new_directory
                        echo "$2/$f" >> $desc_directory 
                    fi
                else
                    mkdir $new_directory
                    cp $current_directory $new_directory
                    touch "$new_directory/desc_$new_folder.txt"
                    echo "$2/$f" > $desc_directory
                fi
            fi    
        fi
    
    done
}

#3. output csv file function:

outputCsv()
{
    cd "$1"
    cd ..
    touch output.csv
    chmod 666 output.csv
    outCsv=`pwd`
    outCsv="$outCsv/output.csv"
    echo "" > output.csv
    #install -m 666 /dev/null outCsv
    
    outString=""
    
    cd $2
    #lineNo=0
    for f in *
    do
        #lineNo=$((lineNo+1))
        if [ -d $f ]; then
            cd $f
            cnt=-1
            for ff in `ls`
            do
                cnt=$((cnt+1))
            done
            #sed -i "$lineNo c\\$f, cnt" outCsv
            outString="${outString}$f, $cnt
            "
            cd ..
        fi
    done

    #lineNo=$((lineNo+1))
    #sed -i "$lineNo c\\$f, cnt" outCsv

    outString="${outString}ignored, $3
    "

    cd $output_directory
    cd ..
    echo $outString > output.csv
    cat output.csv

}


#main code 

filename="nothing"
directory="nothing"
invalid_filename=1
ignore_count=0

if (($# == 0)); then
    directory=`pwd`
    invalid_filename=0
    echo "File name not found, Enter filename"
elif (($# == 1)); then
    if [ -f $1 ]; then
        filename=$1
        directory=`pwd`
    elif [ -d $1]; then
        directory=`pwd`
        directory_name=$1
        directory="$directory/$1"
        invalid_filename=0
    else
        directory=`pwd`
        echo "File name not found, Enter working directory and filename as input"
        invalid_filename=0
    fi
elif (($# == 2)); then
    if [ -d $1 ]; then
        directory=`pwd`
        directory_name=$1
        directory="$directory/$1"
    else 
        echo "Not a directory"
        directory=`pwd`
    fi
    if [ -f $2 ]; then
        filename=$2
    else 
        echo "Not a valid filename"
        invalid_filename=0
    fi
else 
    echo "Invalid input, Enter working directory and valid filename as input"
    directory=`pwd`
fi

# input filename if not given

while (($invalid_filename == 0))
do
    echo "Enter valid file name: "
    read filename

    if [ -f "$filename" ]; then
        invalid_filename=1
        break
    fi
done

echo $filename
echo $directory

#read ignore file extensions

IFS=$'\r\n'
ARRAY=( `cat $filename` )

#cnt=0
#while read line; do
#    cnt=`expr $cnt+1` 
#    ARRAY[$cnt]=$line
#done < $filename

echo ${ARRAY[*]}

#create output directory:

cd $directory
cd ..
output_directory=`pwd`
output_directory="$output_directory/output_directory"


if [ -d output_directory ]; then
    rm -r $output_directory
    echo "cleared previous output directory and created a new one"
fi

mkdir $output_directory



#call traverse function:
temp_directory_name=$directory_name
cd $directory
traverse $directory $temp_directory_name

#create output.csv file:

outputCsv $directory $output_directory $ignore_count




