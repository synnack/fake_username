Uses environment variable FAKE\_USER to specify what username to tell the application you are.

Useful for making applications think your username is different from what it really is.

Use like:
```
FAKE_USER=notme LD_PRELOAD=/path/to/libfakeuser.so ./crappy_application
```
