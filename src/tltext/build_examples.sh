#!/bin/bash

#builds the examples web page from a path of example files
#each .in file must have a comment at the top which is the title of the example

if [ $# -ne 1 ]; then
  echo 'Usage: build_examples.sh testpath'
fi

EXAMPLE_PATH=$1

function output_example()
{
  echo -n '<h2>'
  head -n 1 $FILENAME | sed -e 's!//\(.*\)$!\1!'
  echo '</h2>'
  echo '<form action="tlweb" method="post">'
  echo '<p>'

  LENGTH=$(tail -n +2 $FILENAME | wc -l)

  echo "<textarea cols=\"80\" rows=\"$(($LENGTH+1))\" name=\"program\">"

  tail -n +2 $FILENAME

  echo '</textarea><br><input type="submit" value="Run"></p></form>'
}

echo '<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"'
echo '   "http://www.w3.org/TR/html4/loose.dtd">'
echo '<html>'
echo '<head>'
echo '<meta http-equiv="Content-type" content="text/html;charset=UTF-8">'
echo '<title>TransLucid code examples</title>'
echo '</head>'
echo '<body>'
echo '<h1>TLWeb Examples</h1>'

for t in `find $EXAMPLE_PATH -name '*.in' | sort`; do
  FILENAME=$t
  output_example
done

echo '</body></html>'
