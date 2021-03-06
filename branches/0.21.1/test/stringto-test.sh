#! /bin/sh
# Test driver for tests of stringToLong/Float/Rational
results="./tmp/stringto-test.out"
good="./data/stringto-test.out"
diffargs="--strip-trailing-cr"
tmpfile=tmp/ttt
touch $tmpfile
diff -q $diffargs $tmpfile $tmpfile 2>/dev/null
if [ $? -ne 0 ] ; then
    diffargs=""
fi
(
binpath="$VALGRIND ../../samples"
cd ./tmp
$binpath/stringto-test
) > $results

diff -q $diffargs $results $good
rc=$?
if [ $rc -eq 0 ] ; then
    echo "All testcases passed."
else
    diff $diffargs $results $good
fi
