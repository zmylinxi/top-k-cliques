#! /bin/bash

# /scratch/wujun/inv_graph/large_testcase
# /scratch/wujun/inv_graph/soc-orkut-inv
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $DIR
make clean
make

RUNFILE="/scratch/wujun/inv_graph/soc-orkut-inv"
OUTPUTFILE="/scratch/wujun/inv_graph/soc-orkut-inv.output"

echo $RUNFILE;
# echo "----"
echo "Statistics:"
echo " "
bin/compdegen < $RUNFILE > /dev/null;
bin/printnm < $RUNFILE > /dev/null;
echo " "
echo "Clique counts and runtimes:"
echo " "
# bin/qc --algorithm=tomita     --input-file=$1 > /dev/null
# bin/qc --algorithm=adjlist    --input-file=$1 > /dev/null
# bin/qc --algorithm=hybrid     --input-file=$1 > /dev/null
# bin/qc --algorithm=degeneracy --input-file=$RUNFILE > $OUTPUTFILE
echo " "
echo " "
