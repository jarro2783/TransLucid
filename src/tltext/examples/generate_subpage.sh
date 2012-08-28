#!/bin/bash

mkdir -p html/examples

source $1

SRC_ROOT=`dirname $0`
LOCATION=html/examples/$PAGE_FILE

rm -f $LOCATION

echo "<li><a href=\"examples/${PAGE_FILE}\">${LINK_TEXT}</a></li>" >> $EXAMPLES

echo -e "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\"
   \"http://www.w3.org/TR/html4/loose.dtd\">
<html>
<head>
<meta http-equiv=\"Content-type\" content=\"text/html;charset=UTF-8\">
<title>${PAGE_TITLE}</title>
</head><body>" >> $LOCATION

echo -e ${PAGE_TOP} >> $LOCATION

echo "<p><ul>" >> $LOCATION

OLDIFS=$IFS
IFS=":"
for example in ${INPUT_FILES}; do

  #put the link in the contents page
  FILE_BASE=${example%=*}
  FILE=$SRC_ROOT/$FILE_BASE
  HTML_NAME=$(echo `basename $FILE` | sed 's/\(.*\)\.in/\1.html/')
  HTML_PATH=html/examples/$HTML_NAME
  DESCRIPTION=${example#*=}
  echo "<li><a href=\"${HTML_NAME}\">${DESCRIPTION}</a></li>" >> $LOCATION

  #output the actual file
  rm -rf $HTML_PATH

  echo -e "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\"
  \"http://www.w3.org/TR/html4/loose.dtd\">
<html>
<head>
<meta http-equiv=\"Content-type\" content=\"text/html;charset=UTF-8\">
<title>$DESCRIPTION</title>
</head>
<body>
<h1>$DESCRIPTION</h1>" >> $HTML_PATH

  echo -e "<form action=\"http://translucid.web.cse.unsw.edu.au/tlweb\" 
method=\"post\">
<p>
<textarea cols=\"80\" rows=\"30\" name=\"program\">" >> $HTML_PATH

  cat $FILE >> $HTML_PATH

  echo "</textarea><br><input type=\"submit\" value=\"Run\"></p></form>
</body></html>" >> $HTML_PATH
done

echo -e "</ul>\n</p>\n\n</body>\n</html>" >> $LOCATION
