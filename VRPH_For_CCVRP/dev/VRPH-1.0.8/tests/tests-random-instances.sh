#! /bin/bash

# M.CISSE
# T.SABER

print_help ()
{
    echo "# Usage :"
    echo "# to run tests : sh exec.sh"
    echo "# to clean tests : sh exec.sh clean"

}

## if argc != 1 or argc != 2 print help and exit
if [ $# -ne 0 ]; then
    if [ $# -ne 1 ]; then
	print_help
	exit 1
    fi
fi

## if argc == 1 and argv[1] != 'clean' print help and exit
if [ $# -eq 1 ]; then
    if [ $1 != 'clean' ]; then
	print_help
    else
	rm -rf auto_generated_test/* results/*
    fi
    exit 0
fi

cd ../
make clean && make
cd tests

# compilation of rndmap
cd clustered-vrp-generator
make clean && make 
cd ../



mkdir auto_generated_test
mkdir results
rm -rf results/*

for nbclients in 25 30 50 70 100 150 200 250 500 1000; do
    for nbclusters in 1 2 3 5 7 8 10 12 15 18 20 22; do 
	for i in `seq 1 10`; do
	    name="inst$i-n$nbclients-c$nbclusters"
	    echo "/*****************************************/"
	    echo "/********** $name ****************/"
	    echo "/*****************************************/"	    
	    fichier="./auto_generated_test/$name.ccvrp"
	    clustered-vrp-generator/rndmap "$nbclients" "$nbclusters" 10000 > $fichier
	    ../bin/vrp_rtr -f  $fichier -plot "results/"$name".plot" -pdf "results/"$name".pdf"
	    if [ $? -ne 0 ]; then
		echo "/*****************************************/"
		echo $fichier 
		echo "/*****************************************/"
		cat $fichier
		exit 1
	    fi
	done
    done

done 

