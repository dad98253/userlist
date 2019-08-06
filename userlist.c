/*
 * userlist.c
 *
 *  Created on: Mar 13, 2018
 *      Author: dad
 *
 *	This program reads a set of passwd, shadow, and group files plus a special input
 *	file that contains a mapping of the user's passwords and samba share names. The
 *	the files are parsed and a set of scripts are produced that can be used to duplicate
 *	the usernames, passwords, and share locations on a second computer (handy when
 *	swapping in a new server). The root directory of the shares on the new server can
 *	be specified.
 *
 *	The passwords in the input file are required because most linux systems hash passwords
 *	in a way that can't be read in by useradd. So, a new hash is calculated. The new
 *	(probably less secure) hash will remain in the new shadow file until the user changes
 *	to a new password...
 *
 *	The format of each line in the input file is as follows:
 *	"username" "password" "sharename"
 *	.
 *	.
 *	.
 *
 *	Minimal checks are performed on the format of this file.
 *	Note that each string must be enclosed in double quotes and spaces inside the string
 *	will mess you up. Null strings can be specifed by "". If a user name in the passwd
 *	file is not found in the input file, no output for that user will be produced.
 *
 *	Since your password will be saved in plain text both in the input file and the
 *	addsmbusers script file, care needs to be taked to protect these from prying eyes!
 *
 *
 */

#define _BSD_SOURCE
#define _XOPEN_SOURCE       /* See feature_test_macros(7) */
#include <unistd.h>
#include <stdlib.h>

char *crypt(const char *key, const char *salt);

#include <stdio.h>
#include <string.h>
#define LINELEN 10000

char line1[LINELEN];
char line2[LINELEN];
char *substr1[LINELEN];
char *substr2[LINELEN];
int lensub1[LINELEN];
int lensub2[LINELEN];
typedef struct TEACHERS {
	char * username;
	char * password;
	char * sharename;
} TEACHERS;
typedef struct GROUPS {
	char * groupname;
	char * groupid;
	unsigned int nummembers;
	char ** members;
} GROUPS;
/*
struct TEACHERS teachers[] = {
		{"First.Teacher","imreallyhim",""},
		{"Second.Teacher","bgirl","teacher2"},
};
*/
struct TEACHERS *teachers;
struct GROUPS *groups = NULL;
unsigned int numgroups = 0;
int numteachers = 0;

int dumpteachers(FILE *fInputFile6);
int loadteachers(FILE *fInputFile6);
int cleanupteachers();
TEACHERS * getuser(char* username);
GROUPS * getgroup(char* groupname, int searchon);
unsigned int loadgroups(FILE *fInputFile4);

int main ()
{
	FILE *fInputFile1;
	FILE *fInputFile2;
	FILE *fInputFile3;
	FILE *fInputFile4;
	FILE *fInputFile5;
	FILE *fInputFile6;
	int ntok1,ntok2;
	int i,j;
	int ilen;
	char *tok, *end;
	struct TEACHERS *user;
	char *newhome = "/var/lib/opsi/home";




	if ( (fInputFile1 = fopen("/mnt/DataDisk1/dad/ICAserverFiles/etc/shadow","r")) == NULL ) return(1);
	if ( (fInputFile2 = fopen("/mnt/DataDisk1/dad/ICAserverFiles/etc/passwd","r")) == NULL ) return(1);
	if ( (fInputFile3 = fopen("addusers.sh","w")) == NULL ) return(1);
	if ( (fInputFile4 = fopen("/mnt/DataDisk1/dad/ICAserverFiles/etc/group","r")) == NULL ) return(1);
	if ( (fInputFile5 = fopen("addsmbusers.sh","w")) == NULL ) return(1);
	if ( (fInputFile6 = fopen("teachers.txt","r")) == NULL ) return(1);

//	dumpteachers(fInputFile6);
	teachers = malloc(sizeof(TEACHERS));
	numteachers = loadteachers(fInputFile6);
	// load group data
	numgroups = loadgroups(fInputFile4);

			// reads text until newline

	while (fgets(line1,LINELEN, fInputFile1) != NULL) {
//		    	printf("%s", line1);
		if (fgets(line2,LINELEN, fInputFile2) == NULL) {
			printf ("error 1\n");
			return(1);
		}
//		    	printf("%s", line2);
		substr1[0]=line1;
		substr2[0]=line2;
		ntok1=ntok2=0;
		char *r = strdup(line1);
		tok = end = r;
		ilen = strlen(r);
		if ( *(r+ilen-1) == '\n' ) *(r+ilen-1) = '\000';
		while (tok != NULL) {
			strsep(&end, ":");
			ntok1++;
			substr1[ntok1]=tok;
			lensub1[ntok1]=strlen(tok);
			tok = end;
		}

		char *s = strdup(line2);
		tok = end = s;
		ilen = strlen(s);
		if ( *(s+ilen-1) == '\n' ) *(s+ilen-1) = '\000';

		while (tok != NULL) {
			strsep(&end, ":");
			ntok2++;
			substr2[ntok2]=tok;
			lensub2[ntok2]=strlen(tok);
			tok = end;
		}
/*
		for (i=1;i<ntok1+1;i++) {
			printf("%s[%i:%i];",substr1[i],i,lensub1[i]);
		}
		printf("[[%i]]\n",ntok1);
		for (i=1;i<ntok2+1;i++) {
			printf("%s[%i:%i];",substr2[i],i,lensub2[i]);
		}
		printf("[[%i]]\n",ntok2);		*/
// add the account and set the samba password
		if(strcmp(substr2[1],"root") != 0 && strcmp(substr1[2],"*") != 0 && strcmp(substr1[2],"!") != 0 ) {
			if ( ( user = getuser(substr2[1]) ) != NULL ) {
				fprintf(fInputFile3,"useradd -N -m -g 100 -d \"%s%s\" -p %s -s \"%s\" -u %s %s\n",newhome,strrchr(substr2[6],'/'),crypt(user->password,"aZ"),substr2[7],substr2[3],substr2[1]);
				fprintf(fInputFile5,"(echo %s; echo %s) | smbpasswd -s -a %s\n",user->password,user->password,user->username);
			}
		}
		free(r);
		free(s);
	}

// add users to groups (no need to add to "users" because we did that above when creating the accounts)
	for ( i=0;i<numgroups;i++) {
		if ( groups[i].nummembers > 0 && strcmp( groups[i].groupname, "users" ) != 0 ) {
			for (j=0; j < groups[i].nummembers ;j++) {
				if ( getuser ( *(groups[i].members+j) ) ) {
					fprintf(fInputFile3,"usermod -aG %s %s\n",groups[i].groupname,*(groups[i].members+j));
				}
			}
		}
	}

	printf(" that's all !!\n");
//	printf("%s\n",crypt("abc","ab"));

	cleanupteachers();
	return(0);
}

TEACHERS * getuser(char* username) {
	int j;
//	size_t numteachers = sizeof(teachers)/sizeof(TEACHERS);
	for (j=0;j<numteachers;j++) {
		if (strcmp(username,teachers[j].username) == 0 ) {
			return (&teachers[j]);
		}
	}

	return (NULL);
}

/*
 * typedef struct TEACHERS {
	char * username;
	char * password;
	char * sharename;
} TEACHERS;
 */
int dumpteachers(FILE *fInputFile6) {

	int j;
	size_t numteachers = sizeof(teachers)/sizeof(TEACHERS);
	for (j=0;j<numteachers;j++) {
			fprintf(fInputFile6,"\"%s\" \"%s\" \"%s\"\n",teachers[j].username,teachers[j].password,teachers[j].sharename);
		}

	return (0);
}
int loadteachers(FILE *fInputFile6) {

	char user[LINELEN]="";
	char pass[LINELEN]="";
	char share[LINELEN]="";
	char *puser,*ppass,*pshare;
	int j=0;
//	size_t numteachers = sizeof(teachers)/sizeof(TEACHERS);
	while ( fscanf(fInputFile6,"%s %s %s\n",user,pass,share) == 3 ) {
		user[strlen(user)-1] = '\000';
		pass[strlen(pass)-1] = '\000';
		share[strlen(share)-1] = '\000';
		puser = (char*)malloc(strlen(user));
		ppass = (char*)malloc(strlen(pass));
		pshare = (char*)malloc(strlen(share));
		strcpy(puser,user+1);
		strcpy(ppass,pass+1);
		strcpy(pshare,share+1);
//printf("user=[%s],pass=[%s],share=[%s]\n",puser,ppass,pshare);
		teachers = realloc(teachers,sizeof(TEACHERS)*(j+1));
		teachers[j].username=puser;
		teachers[j].password=ppass;
		teachers[j].sharename=pshare;
		j++;
	}

	return (j);
}
int cleanupteachers() {
	int j;
	size_t numteachers = sizeof(teachers)/sizeof(TEACHERS);
	for (j=0;j<numteachers;j++) {
		if (teachers[j].username != NULL )free(teachers[j].username);
		if (teachers[j].password != NULL )free(teachers[j].password);
		if (teachers[j].sharename != NULL )free(teachers[j].sharename);
	}
	free(teachers);
	return (0);
}

GROUPS * getgroup(char* groupname, int searchon) {
	int j;
	if ( numgroups == 0 ) return (NULL);
	if ( searchon != 2 ) {
		for (j=0;j<numgroups;j++) {
			if (strcmp(groupname,groups[j].groupname) == 0 ) {
				return (&groups[j]);
			}
		}
	} else {
		for (j=0;j<numgroups;j++) {
			if (strcmp(groupname,groups[j].groupid) == 0 ) {
				return (&groups[j]);
			}
		}
	}

	return (NULL);
}

/*
 * typedef struct GROUPS {
	char * groupname;
	char *  groupid;
	unsigned int nummembers;
	char ** members;
} GROUPS;
 */

unsigned int loadgroups(FILE *fInputFile4) {
	int ntok1;
	int i;
	int ilen;
	char *tok, *end;
//	struct GROUPS *group;
	unsigned int numfound = 0;
	groups = (GROUPS *)malloc(sizeof(GROUPS));

	while (fgets(line1,LINELEN, fInputFile4) != NULL) {
//			    	printf("%s", line1);

			    	substr1[0]=line1;
			    	ntok1=0;
			    	char *r = strdup(line1);
			    	tok = end = r;
			    	ilen = strlen(r);
			    	if ( *(r+ilen-1) == '\n' ) *(r+ilen-1) = '\000';
			    	while (tok != NULL) {
			    	    strsep(&end, ":");
			    	    ntok1++;
			    	    substr1[ntok1]=tok;
			    	    lensub1[ntok1]=strlen(tok);
			    	    tok = end;
			    	}

			    	groups[numfound].groupname = strdup(substr1[1]);
			    	groups[numfound].groupid = strdup(substr1[3]);
			    	groups[numfound].nummembers = 0;
			    	if ( lensub1[4] != 0 ) {
			    		ntok1=0;
			    		char *s = strdup(substr1[4]);
			    		tok = end = s;
			    		ilen = strlen(s);
			    		if ( *(s+ilen-1) == '\n' ) *(s+ilen-1) = '\000';
			    		while (tok != NULL) {
			    			strsep(&end, ",");
			    			ntok1++;
			    			substr1[ntok1]=tok;
			    			lensub1[ntok1]=strlen(tok);
			    			tok = end;
			    		}
			    		groups[numfound].nummembers = ntok1;
			    		groups[numfound].members = (char **)malloc(sizeof(char*)*ntok1);
			    		for (i=0;i<ntok1;i++) {
			    	//		printf("ss (%i) = %s\n",i+1,substr1[i+1]);
			    			*(groups[numfound].members+i) = strdup(substr1[i+1]);
			    		}
			    		free(s);
			    	}
/*			    	printf("%s:%s (%u)\n", groups[numfound].groupname,groups[numfound].groupid,groups[numfound].nummembers);
			    	if ( groups[numfound].nummembers > 0 ) {
						for (i=0;i<groups[numfound].nummembers;i++) {
							printf("%s[%i:%i];",*(groups[numfound].members+i),i+1,(int)strlen(*(groups[numfound].members+i)));
						}
						printf("\n");
			    	}		*/

					numfound++;
			    	free(r);
			    	groups = (GROUPS *)realloc(groups,sizeof(GROUPS)*(numfound+1));

	}

	return (numfound);
}

