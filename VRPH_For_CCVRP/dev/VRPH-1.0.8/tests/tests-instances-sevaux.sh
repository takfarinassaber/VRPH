#!/bin/sh

# M.CISSE
# T.SABER

print_help ()
{
    echo "# Usage :"
    echo "# to run tests : sh tests.sh"
    echo "# to clean tests : sh tests.sh clean"

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
	rm -rf *.pdf
    fi
    exit 0
fi
    

for i in ../instances/ccvrp-instance/*; do
    echo "Instances : " `basename "$i"`
    ../bin/vrp_rtr -f $i -plot `basename "$i" .ccvrp`.plot -pdf `basename "$i" .ccvrp`.pdf -h KITCHEN_SINK
    rm -f `basename "$i" .ccvrp`.plot
    echo "Done"
done