#!/bin/bash

cd test
for filename in *.seal; do
    echo "--------Test using" $filename "--------"
    ../semant $filename > ../my-answer/$filename.out
    diff ../my-answer/$filename.out ../test-answer/$filename.out > /dev/null
    if [ $? -eq 0 ]; then
        echo "Passed"
    else
        diff ../my-answer/$filename.out ../test-answer/$filename.out
        echo NOT passed
    fi
done
rm -f tempfile
cd ..
