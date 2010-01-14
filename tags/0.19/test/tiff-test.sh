#! /bin/sh
# TIFF parser test driver

# ----------------------------------------------------------------------
# Check if xmpparser-test exists
if [ `which exifprobe`x = x ] ; then
    echo "tiff-test.sh: exifprobe not found. Skipping TIFF tests."
    exit 0
fi

# ----------------------------------------------------------------------
# Setup
results="./tmp/tiff-test.out"
good="./data/tiff-test.out"

# ----------------------------------------------------------------------
# Main routine
(
binpath="$VALGRIND ../../samples"
exiv2="$VALGRIND ../../src/exiv2"
cd ./tmp

# ----------------------------------------------------------------------
# Basic write test
testfile=mini9.tif
cp -f ../data/$testfile .
exifprobe $testfile
$binpath/tiff-test $testfile
exifprobe $testfile

) > $results

# ----------------------------------------------------------------------
# Evaluate results
cat $results | sed 's/\x0d$//' > $results-stripped
diff -q $results-stripped $good
rc=$?
if [ $rc -eq 0 ] ; then
    echo "All testcases passed."
else
    diff $results-stripped $good
fi