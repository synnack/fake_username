/*
BSD 2-Clause License

Copyright (c) 2022, Wilco Baan Hofman <wilco@baanhofman.nl>

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <pwd.h>
#include <grp.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>


static void fakeuser_init(void) __attribute__ ((constructor));
static void fakeuser_uninit(void) __attribute__ ((destructor));

#define ORIGINAL_USER_SIZE 1024
static char original_user[ORIGINAL_USER_SIZE];
static char *fake_user = NULL;

struct passwd *getpwnam (const char *__name) {
	static struct passwd *(*o_getpwnam)(const char *__name) = NULL;
	if (!o_getpwnam) o_getpwnam = dlsym(RTLD_NEXT, "getpwnam");

	// Run getpwnam for original user
	// overwrite the resulting name in the struct
	if (strcmp(__name, fake_user) == 0) {
	    struct passwd *rv = o_getpwnam(original_user);
	    rv->pw_name = fake_user;
	    return rv;
	}
	return o_getpwnam(__name);
}
int getpwnam_r (const char *__restrict __name, struct passwd *__restrict __resultbuf,
		       char *__restrict __buffer, size_t __buflen,
		       struct passwd **__restrict __result) {
	static int (*o_getpwnam_r)(const char *, struct passwd *, char*, size_t, struct passwd**) = NULL;
	if (!o_getpwnam_r) o_getpwnam_r =  dlsym(RTLD_NEXT, "getpwnam_r");


	// Run getpwnam_r for original user,
	// overwrite the resulting name in the struct
	if (strcmp(__name, fake_user) == 0) {
	    int rv = o_getpwnam_r(original_user, __resultbuf, __buffer, __buflen, __result);
	    __resultbuf->pw_name = fake_user;
	    return rv;
	}
	return o_getpwnam_r(__name, __resultbuf, __buffer, __buflen, __result);
}

struct passwd *getpwuid (__uid_t __uid) {
	static struct passwd *(*o_getpwuid)(__uid_t __uid) = NULL;
	if (!o_getpwuid) o_getpwuid = dlsym(RTLD_NEXT, "getpwuid");


	// Run getpwuid like normal
	struct passwd *rv = o_getpwuid(__uid);

	// Overwrite the resulting name in the struct
	if (__uid == getuid()) {
	    rv->pw_name = fake_user;

	}
	return rv;
}

int getpwuid_r (__uid_t __uid, struct passwd *__restrict __resultbuf,
		       char *__restrict __buffer, size_t __buflen,
		       struct passwd **__restrict __result) {
	static int (*o_getpwuid_r)(__uid_t, struct passwd *, char*, size_t, struct passwd**) = NULL;
	if (!o_getpwuid_r) o_getpwuid_r = dlsym(RTLD_NEXT, "getpwuid_r");


	// Run getpwuid_r like normal
	int rv = o_getpwuid_r(__uid, __resultbuf, __buffer, __buflen, __result);

	// Overwrite the resulting name in the struct
	if (__uid == getuid()) {
	    __resultbuf->pw_name = fake_user;
	}
	return rv;
}

int getgrouplist(const char *user, gid_t group, gid_t *groups, int *ngroups) {
    static int (*o_getgrouplist)(const char *user, gid_t group, gid_t *groups, int *ngroups) = NULL;
    if (!o_getgrouplist) o_getgrouplist = dlsym(RTLD_NEXT, "getgrouplist");

    // Run getgrouplist for the original user if we are requesting the fake user
    if (strcmp(user, fake_user) == 0) {
        return o_getgrouplist(original_user, group, groups, ngroups);
    }
    // Otherwise, run getgrouplist like normal
    return o_getgrouplist(user, group, groups, ngroups);
}

// constructor for library (requires gcc)
static void fakeuser_init(void) {
    fake_user = getenv("FAKE_USER");
    if (fake_user == NULL) {
	fprintf(stderr, "Please supply a valid fake user via the FAKE_USER environment variable.\n");
	exit(2);
    }

    // Overwrite environment
    setenv("USER", fake_user, true);
    setenv("LOGNAME", fake_user, true);

    // Find out the origanl username
    struct passwd *(*o_getpwuid)(__uid_t __uid) = dlsym(RTLD_NEXT, "getpwuid");
    struct passwd *pw_ent = o_getpwuid(getuid());
    strncpy(original_user, pw_ent->pw_name, ORIGINAL_USER_SIZE-1);
    original_user[ORIGINAL_USER_SIZE-1] = '\0';

    fprintf(stderr, "Pretending to be user %s while hiding user %s.\n", fake_user, original_user);
}

// destructor for library (requires gcc)
static void fakeuser_uninit(void) {
}
