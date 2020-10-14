#!/bin/bash
cd test
for filename in *.seal; do
    echo "--------Test using" $filename "--------"
    ../lexer $filename > ../my_answer/my_$filename.out
    diff ../my_answer/my_$filename.out ../test-answer/$filename.out > /dev/null
    if [ $? -eq 0 ] ; then
        echo passed
    else
        diff ../my_answer/my_$filename.out ../test-answer/$filename.out
        echo NOT passed
    fi
done
rm -f tempfile
cd ..