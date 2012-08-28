#!/bin/bash

rm -rf html

SOURCE=`dirname $0`

GENSUB=$SOURCE/generate_subpage.sh
GENFRONT=$SOURCE/generate_front.sh

export EXAMPLES=html/examples.html

mkdir -p html
rm -rf $EXAMPLES

#start writing out the examples page
echo "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\"
\"http://www.w3.org/TR/html4/loose.dtd\">
<html>
<head>
<meta http-equiv=\"Content-type\" content=\"text/html;charset=UTF-8\">
<title>TransLucid code examples</title>
</head>
<body>
<h1>TransLucid code examples</h1>
<ul>" >> $EXAMPLES

$GENSUB methodology
$GENSUB lucid

echo "</ul>
<h1>TransLucid preamble</h1>
<ul>
<li><a href="examples/header.tl">The header file with the basic
declarations</a></li>
</ul>" >> $EXAMPLES

$GENFRONT
