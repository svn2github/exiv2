#! /bin/sh
# Test driver for write unit tests to build Exif metadata from scratch
results="./tmp/modify-test.out"
good="./data/modify-test.out"
diffargs="--strip-trailing-cr"
tmpfile=tmp/ttt
touch $tmpfile
diff -q $diffargs $tmpfile $tmpfile 2>/dev/null
if [ $? -ne 0 ] ; then
    diffargs=""
fi
(
cp -f ./data/exiv2-empty.jpg ./tmp
cp -f ./data/exiv2-gc.jpg ./tmp
cd ./tmp
$VALGRIND exiv2 -v -m ../data/modifycmd1.txt exiv2-empty.jpg
$VALGRIND exiv2 -v -m ../data/modifycmd2.txt exiv2-gc.jpg
$VALGRIND exiv2 -v -pi exiv2-empty.jpg
$VALGRIND exiv2 -v -pt exiv2-empty.jpg exiv2-gc.jpg

) > $results

diff -q $diffargs $results $good
rc=$?
if [ $rc -eq 0 ] ; then
    echo "All testcases passed."
else
    diff $diffargs $results $good
fi
