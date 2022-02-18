Uses environment variable FAKE\_USER to specify what username to tell the application you are.

Useful for making applications think your username is different from what it really is.

Use like:
```
env FAKE_USER=notme LD_PRELOAD=/path/to/libfakeuser.so ./crappy_application
```

Results with show-getpwuid:
```
Pretending to be user notme while hiding user synnack.
getpwuid(uid):                   notme
getpwuid(euid):                  notme
getpwnam(getpwuid(uid)):         notme
getpwnam(getpwuid(euid)):        notme
getpwuid_r(uid):                 notme
getpwuid_r(euid):                notme
getpwnam_r(getpwuid_r(uid)):     notme
getpwnam_r(getpwuid_r(euid)):    notme
LOGNAME env:                     notme
USER env:                        notme
getgrouplist(getpwuid(getuid()):
Group: 1000
Groupname: synnack
Groupname(_r): synnack
Group: 4
Groupname: adm
Groupname(_r): adm
Group: 20
Groupname: dialout
Groupname(_r): dialout
```
