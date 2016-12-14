#!/bin/bash

git show :1:$1 > $1.common
git show :2:$1 > $1.ours
git show :3:$1 > $1.theirs

cp $1 $1.orig

meld $1.ours $1 $1.theirs

echo -n "save? (Y/n) "

read item
case "$item" in
    n|N) echo "cancel"
        cp $1.orig $1
        ;;
    *) git add $1
        ;;
esac

rm -rf $1.common $1.ours $1.theirs $1.orig

