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
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <stdlib.h>
#include <stdio.h>

int main() {
    int errors = 0;
    int uid = getuid();
    int euid = geteuid();
    char *env_logname = getenv("LOGNAME");
    char *env_user = getenv("USER");


    struct passwd *uid_ent = getpwuid(uid);
    struct passwd *euid_ent = getpwuid(euid);

    struct passwd uid_r_ent;
    struct passwd euid_r_ent;
    struct passwd *uid_r_ent_ptr;
    struct passwd *euid_r_ent_ptr;
    struct passwd uidnam_r_ent;
    struct passwd euidnam_r_ent;
    struct passwd *uidnam_r_ent_ptr;
    struct passwd *euidnam_r_ent_ptr;

    ssize_t bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (bufsize == -1)          /* Value was indeterminate */
        bufsize = 16384;        /* Should be more than enough */    

    char uid_buf[bufsize];
    int uid_result = getpwuid_r(uid, &uid_r_ent,
                      uid_buf, bufsize, &uid_r_ent_ptr);
    if (uid_result) {
	printf("getpwuid_r() failed\n");
	errors++;
    }
    char euid_buf[bufsize];
    int euid_result = getpwuid_r(uid, &euid_r_ent,
                      euid_buf, 1024, &euid_r_ent_ptr);
    if (euid_result) {
	printf("getpwuid_r() failed\n");
	errors++;
    }
    int ngroups = 1024;
    gid_t groups[1024];

    // getgrouplist ORIG_USER
    int getgrlist_result = getgrouplist(uid_ent->pw_name, uid_ent->pw_gid, groups, &ngroups);
    if (getgrlist_result < 0) {
	printf("getgrouplist() failed, ngroups=%d, result=%d\n", ngroups, getgrlist_result);
	errors++;
    }
    struct passwd *uid_nam = getpwnam(uid_ent->pw_name);
    struct passwd *euid_nam = getpwnam(euid_ent->pw_name);
    char uidnam_buf[bufsize];
    int uidnam_result = getpwnam_r(uid_ent->pw_name, &uidnam_r_ent,
                      uidnam_buf, 1024, &uidnam_r_ent_ptr);
    if (uidnam_result) {
	printf("getpwnam_r(getpwuid_r(uid)) failed\n");
	errors++;
    }
    char euidnam_buf[bufsize];
    int euidnam_result = getpwnam_r(euid_ent->pw_name, &euidnam_r_ent,
                      euidnam_buf, 1024, &euidnam_r_ent_ptr);
    if (euidnam_result) {
	printf("getpwnam_r(getpwuid_r(euid)) failed\n");
	errors++;
    }

    printf("getpwuid(uid):                   %s\n", uid_ent->pw_name);
    printf("getpwuid(euid):                  %s\n", euid_ent->pw_name);
    printf("getpwnam(getpwuid(uid)):         %s\n", uid_nam->pw_name);
    printf("getpwnam(getpwuid(euid)):        %s\n", euid_nam->pw_name);
    printf("getpwuid_r(uid):                 %s\n", uid_r_ent.pw_name);
    printf("getpwuid_r(euid):                %s\n", euid_r_ent.pw_name);
    printf("getpwnam_r(getpwuid_r(uid)):     %s\n", uidnam_r_ent.pw_name);
    printf("getpwnam_r(getpwuid_r(euid)):    %s\n", euidnam_r_ent.pw_name);
    printf("LOGNAME env:                     %s\n", env_logname);
    printf("USER env:                        %s\n", env_user);
    printf("getgrouplist(getpwuid(getuid()):\n");
    for (int i = 0; i < ngroups; i++) {
	printf("Group: %d\n", groups[i]);
	struct group *gr_ent = getgrgid(groups[i]);
	printf("Groupname: %s\n", gr_ent->gr_name);
	struct group gr_r_ent;
	struct group *gr_r_ent_ptr;
	char gr_buf[bufsize];
	int gr_result = getgrgid_r(groups[i], &gr_r_ent,
			  gr_buf, 1024, &gr_r_ent_ptr);
	if (gr_result) {
	    printf("getgrgid_r() failed\n");
	    errors++;
	}
	printf("Groupname(_r): %s\n", gr_r_ent.gr_name);
    }
    return errors;
}
