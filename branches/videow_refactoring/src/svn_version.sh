#!/bin/bash

## 
# update svn_version.h when the revision has changed
svn_version=$(svn info .. 2>/dev/null | grep ^Revision | cut -f 2 -d' ')
if [ -z "$svn_version" ]; then svn_version=0 ; fi
if [ -e svn_version.h ]; then
	old=$(cut -f 3 -d' ' svn_version.h)
	if [ "$old" != "$svn_version" ]; then
		rm -rf svn_version.h
	fi
fi
if [ ! -e svn_version.h ]; then
	echo "#define SVN_VERSION $svn_version" > svn_version.h
fi

# That's all Folks!
##
