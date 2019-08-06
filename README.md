# userlist


This program reads a set of passwd, shadow, and group files plus a special input
file that contains a mapping of the user's passwords and samba share names. The
the files are parsed and a set of scripts are produced that can be used to duplicate
the usernames, passwords, and share locations on a second computer (handy when
swapping in a new server). The root directory of the shares on the new server can
be specified.

The passwords in the input file are required because most linux systems hash passwords
in a way that can't be read in by useradd. So, a new hash is calculated. The new
(probably less secure) hash will remain in the new shadow file until the user changes
to a new password...

The format of each line in the input file is as follows:
"username" "password" "sharename"
.
.
.

Minimal checks are performed on the format of this file.
Note that each string must be enclosed in double quotes and spaces inside the string
will mess you up. Null strings can be specifed by "". If a user name in the passwd
file is not found in the input file, no output for that user will be produced.

Since your password will be saved in plain text both in the input file and the
addsmbusers script file, care needs to be taked to protect these from prying eyes!
