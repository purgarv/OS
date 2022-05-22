#!/bin/bash

pomoc(){
	echo "Uporaba: $(basename $0) akcija parametri"
}

hehho(){
	count=0
	for i in "${@:2}"
	do
		echo -n "$count: "
		echo $i | sed "s/a/ha/g" | sed "s/e/he/g" | sed "s/i/hi/g" | sed "s/o/ho/g" | sed "s/u/hu/g"
		((count++))
	done
}

status(){
	gcd=1
	i=1
	while [ $i -le $2 ] && [ $i -le $3 ]
	do
		if (( !(($2 % $i)) && !(($3 % $i)) ))
		then
			((gcd=i))
		fi
		((i++))
	done
	echo $gcd
	ret=$(($gcd % 256))
	return $ret
}

leto(){

	for i in "${@:2}"
	do
		(( !(i % 4) && ( i % 100 || !(i % 400) ) )) && echo "Leto $i je prestopno." || echo "Leto $i ni prestopno."
	done
}

stej(){
	grep -v '^#' "${@:2}" | xargs -n1 | cut -d: -f3 | xargs -n1 | sort | uniq -c | sort -r | nl 
}


fib(){

	for((j=$#; j > 1; j--))
	do
		prevprev=0
		prev=0
		current=1
		i=1

		if ((${!j} == 0))
		then
			echo "${!j}: 0"
			continue
		fi

		if ((${!j} == 1))
		then
			echo "${!j}: 1"
			continue
		fi

		while ((i < ${!j}))
		do
			prevprev=$prev
			prev=$current
			current=$(( $prevprev + $prev ))

			i=$(( $i + 1 ))
		done

		echo "${!j}: $current"
	done
}

upori(){
	users=$(cut -d: -f1 /etc/passwd)

	for i in "${@:2}"
	do
		if [[ " ${users[*]} " =~ "$i" ]]
		then
			uid=$(grep -w "^$i" /etc/passwd | cut -d: -f3)
			gid=$(grep -w "^$i" /etc/passwd | cut -d: -f4)
			home=$(grep -w "^$i" /etc/passwd | cut -d: -f6)
			groupCount=$(groups "$i" | wc -w)
			((groupCount=groupCount - 2))

			pc=$(ps -u "$i" | awk 'END {print NR}' )
			((pc=pc-1))

			echo -n "$i: "

			if((uid == gid))
			then
				echo -n "enaka "
			else
				echo -n "razlicna "
			fi

			if [[ -d $home ]]
			then
				echo -n "obstaja "
			else
				echo -n "ne-obstaja "
			fi

			echo -n "$groupCount "
			echo "$pc"

		else
			echo "${!i}: err"


		fi
	done
}

tocke(){
	RANDOM=42
	sestevekS=0
	sestevek=0
	count=0
	while read -r line
	do
        line=$(echo "$line" | grep -v "^#")

        if [ "$line" = "" ]
        then
        	continue
        fi

        arr=($line)
        vpisna=${arr[0]}
        a=${arr[1]}
        b=${arr[2]}
        c=${arr[3]}
        tip=${arr[4]}
        sestevekS=$((a+b+c))

        if [ "$tip" = "p" ] || [ "$tip" = "P" ]
        then
        	sestevekS=$((sestevekS / 2))
        else 
        	if [ "${vpisna:2:2}" = "14" ]
	        then
	        	sestevekS=$((sestevekS + ($RANDOM % 5) + 1))
	        fi
        fi

        if((sestevekS > 50))
        then
        	sestevekS=50
        fi

        echo "$vpisna: $sestevekS"

        ((count = count + 1))
        sestevek=$((sestevek + sestevekS))
        sestevekS=0

    done

    povprecje=$((sestevek / count))
    echo "St. studentov: $count"
    echo "Povprecne tocke: $povprecje"
}


drevo(){
	dir="${2:-"."}"
	globina="${3:-3}"
	local count="${4:-0}"

	if((count == 0))
	then
		count=$((count + 1))
		echo "DIR   $dir"
	fi

	for i in "$dir"/*
	do
		f="$(basename "$i")"
		for((j=0; j < $count; j++))
		do
			echo -n "----"
		done

		if [[ -S $i ]]; then
			echo "SOCK  $f"

		elif [[ -f $i ]]; then
			echo "FILE  $f"

		elif [[ -L $i ]]; then
			echo "LINK  $f"

		elif [[ -b $i ]]; then
			echo "BLOCK $f"

		elif [[ -c $i ]]; then
			echo "CHAR  $f"

		elif [[ -p $i ]]; then
			echo "PIPE  $f"

		elif [[ -d $i ]]; then
			echo "DIR   $f"
			if [ $((count + 1)) -le $globina ] && [ "$(ls -A "$i")" ]
			then
				drevo "" "$i" "$globina" $((count + 1))
			fi
		fi	
	done
}


prostor(){
	size=$(stat -c %s "$2")
	numBlocks=0
	sizeB=0

	calcSize(){
		dir="${1:-"."}"
		globina="${2:-3}"

		for i in "$dir"/*
		do
			size_=$(stat -c %s "$i")
			numBlocks_=$(stat -c %b "$i")
			sizeB=$(stat -c %B "$i")

			(( size += size_))
			(( numBlocks += numBlocks_))

			if [[ -S $i ]]; then
				:

			elif [[ -f $i ]]; then
				:

			elif [[ -L $i ]]; then
				:

			elif [[ -b $i ]]; then
				:

			elif [[ -c $i ]]; then
				:

			elif [[ -p $i ]]; then
				:

			elif [[ -d $i ]]; then
				if [ $(($3 + 1)) -le $globina ] && [ "$(ls -A "$i")" ]
				then
					calcSize "$i" "$globina" $(($3 + 1))
				fi
			fi	
		done
	}

	calcSize $2 $3 1

	echo "Velikost: $size"
	echo "Blokov: $numBlocks"
	echo "Prostor: $((sizeB * numBlocks))"

}


case $1 in

	"pomoc")
    	pomoc "$@"
    ;;

  	"hehho")
    	hehho "$@"
    ;;

  	"status")
    	status "$@"
    ;;

  	"leto")
    	leto "$@"
    ;;

	"stej")
    	stej "$@"
    ;;

	"fib")
    	fib "$@"
    ;;

	"upori")
    	upori "$@"
    ;;

    "tocke")
    	tocke "$@"
    ;;

	"drevo")
    	drevo "$@"
    ;;

    "prostor")
    	prostor "$@"
    ;;

  	*)
		echo "Napacna uporaba skripte!"
		pomoc "$@"
    	exit 42
    ;;
esac
