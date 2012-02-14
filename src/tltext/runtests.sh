#!/bin/sh

if [ $# -ne 2 ]; then
  echo Usage: $0 tltext testpath
  exit 1
fi

TLTEXT=$1
TESTPATH=$2
PASSED=0
FAILED=0

echo Testing 'in' $TESTPATH

for t in `find $TESTPATH -name '*.in' | sort`; do
  EXPECTED_OUT=`echo $t | sed 's/\(.*\).in/\1.out/'`
  echo $TLTEXT -v 0 --input $t
  $TLTEXT -v 0 --input $t | diff $EXPECTED_OUT -
  RESULT=$?
  if [ $RESULT -eq 0 ]; then
    PASSED=$((++PASSED))
    echo Passed
  else
    FAILED=$((++FAILED))
    echo Failed
  fi
done

echo Passed $PASSED out of $((PASSED+FAILED)) tests

exit $FAILED
