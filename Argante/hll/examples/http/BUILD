#!/bin/sh

PATH=../../hll:../../compiler:$PATH
PATH=../../../hll:../../../compiler:$PATH

if [ "$OSTYPE" = "linux-gnu" ]; then
  if [ "$1" = "" ]; then
    acc -e httpd.ahl
  else
    acc httpd.ahl
  fi
else
  acc httpd.ahl
fi

echo
echo "Now, you can run Mini-HTTP using \$Examples/Mini-HTTP/httpd.img command."
echo