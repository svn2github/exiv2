#! /bin/sh
# Test driver for image file i/o

eraseTest()
{
    src=$1
    test=${src}.etst
    good=$datapath/${src}.egd

    #setup
    cp $datapath/$src $test

    #run tests
    $binpath/metacopy $test $test

    #check results
    diffCheck $test $good
    echo -n "."
}

copyTest()
{
    num=$1
    src=$2
    dst=$3
    test=${dst}.c${num}tst
    good=$datapath/${dst}.c${num}gd

    #setup
    cp $datapath/$dst $test

    #run tests
    $binpath/metacopy -a $datapath/$src $test

    #check results
    diffCheck $test $good
    echo -n "."
}

iptcTest()
{
    num=$1
    src=$2
    dst=$3
    test=${dst}.i${num}tst
    good=$datapath/${dst}.i${num}gd

    #setup
    cp $datapath/$dst $test

    #run tests
    $binpath/metacopy -ip $datapath/$src $test

    #check results
    diffCheck $test $good
    echo -n "."
}


# Make sure to pass the test file first and the known good file second
diffCheck() 
{
    test=$1
    good=$2
    
    #run diff and check results
    diff -q --binary $test $good
    if [ $? -ne 0 ]; then
       errors=`expr $errors + 1`
    else
       rm $test
    fi 
}

# **********************************************************************
# main

LD_LIBRARY_PATH=../../src:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH
binpath="$VALGRIND ../../src"
datapath="../data"

test_files="table.jpg smiley1.jpg smiley2.jpg"

errors=0
cd ./tmp
echo

echo -n "Erase all tests"
for i in $test_files; do eraseTest $i; done
eraseTest "glider.exv" #extra test

echo
echo -n "Copy all tests"
c=0
for src in $test_files; do
    c=`expr $c + 1`
    for dst in $test_files; do copyTest $c $src $dst; done
done

echo
echo -n "Copy iptc tests"
c=0
for src in $test_files; do
    c=`expr $c + 1`
    for dst in $test_files; do iptcTest $c $src $dst; done
done

echo -e "\n---------------------------------------------------------"
if [ $errors -eq 0 ]; then
   echo 'All test cases passed'
else
   echo $errors 'test case(s) failed!'
fi
