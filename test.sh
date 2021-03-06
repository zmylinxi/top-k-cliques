#! /bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $DIR

make clean
make

# data/test
# data/biogrid/biogrid-mouse
bin/qc --algorithm=degeneracy --input-file="data/biogrid/biogrid-mouse" $1

for i in  data/*/*
do 
    break
    echo $i;
    # echo "----"
    # echo "Statistics:"
    # echo " "
    # bin/compdegen < $i > /dev/null;
    # bin/printnm < $i > /dev/null;
    # echo " "
    # echo "Clique counts and runtimes:"
    echo " "
    # bin/qc --algorithm=tomita     --input-file=$i > /dev/null
    # bin/qc --algorithm=adjlist    --input-file=$i > /dev/null
    # bin/qc --algorithm=hybrid     --input-file=$i > /dev/null
    bin/qc --algorithm=degeneracy --input-file=$i --K=2
    echo " "
    echo " "
    break
done
