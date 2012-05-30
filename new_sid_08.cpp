#include "stdlib.h"
#include "stdio.h"
#include "windows.h"
#include "tchar.h"
#include "time.h"
#include <string>
#include <iostream>
#include "io.h"

char cdDriver[8];

static char* timestr()
{
	static  char str_t[32];
	static  time_t  t = 0;
	time_t  curtime = time(NULL);
	struct  tm*  tm_t;
	if(curtime != t)
	{
		t = curtime;
		tm_t = localtime(&curtime);
		sprintf(str_t, "%04d%02d%02d-%02d:%02d.%02d", 
				tm_t->tm_year+1900, tm_t->tm_mon+1, tm_t->tm_mday,
				tm_t->tm_hour, tm_t->tm_min, tm_t->tm_sec);
	}
	return str_t;
	
}

void LOG_FILE(char* buf)
{
	FILE* fp;
	static char* filename="C:\\newsid.log";
	if(NULL==(fp=fopen(filename, "at")))
	{
		printf("Open file [%s] failed!\n", filename);
		return ;
	}
	fputs(timestr(), fp);
	fputs(" ", fp);
	fputs(buf, fp);
	fputs("\r\n", fp);
	fclose(fp);
}

char* trimright(char* str)
{
    char* p = NULL;
    char* s = str;
    for(; *s; s++)
    {
        if(!p)
        {
            if(isspace((int)(*s)))
                p = s;
        }
        else
        {   
            if(!isspace((int)(*s)))
                p = NULL;
        }
    }
    if(p) *p = 0;
    return str;
}

char* TrimQuote(char* value)
{
	char buf[256];
	if(value[0]==0x22 || value[0]==0x27)
		strcpy(buf, value+1);
	int len=strlen(buf);
	if(buf[len-1] == 0x22 || buf[len-1] == 0x27)
		buf[len-1]=0x00;

	strcpy(value, buf);
	return value;
}

char* GetFieldValue(char* filename, char* fieldname, char* value)
{
	char buf[256];
	FILE* fp;
	char* dotp;
	if(NULL==(fp=fopen(filename, "rt")))
	{
		return NULL;
	}

	while(fgets(buf, 256, fp))
	{
		if(strstr(buf, fieldname))
		{
			dotp=strstr(buf, "=");
			strcpy(value, dotp+1);
	//		trimleft(value);
			trimright(value);
			TrimQuote(value);
			return value;
		}
		memset(value, 0x00, sizeof(value));
	}
	fclose(fp);

	return NULL;
}

int SetHostname(char* hostname, char* oldname)
{
    char hostnamecmd[128];
	char buf1[128];

	sprintf(hostnamecmd, "netdom renamecomputer %s /newname:%s /force", oldname,hostname);
	LOG_FILE(hostnamecmd);
	system(hostnamecmd);
	system("shutdown -r -t 5");

	sprintf(buf1, "Set Hostname is %s.", hostname);
	LOG_FILE(buf1);
	LOG_FILE("System will reboot.");

	return 0;
}

int OnlyCreateSID()
{
	char runcmd[128];
	char* sid_file="C:\\soft\\sysprep\\sysprep.exe";
	if(access(sid_file, 0))
	{
		LOG_FILE("Warning: Cannot find sysprep.exe");
		return -1;
	}

	sprintf(runcmd, "%s /generalize /reboot /audit /quiet", sid_file);
	LOG_FILE("Create NewSID. DON'T SET HOSTNAME!");
	LOG_FILE("System will reboot.");
	system(runcmd);
	return 0;
}

int SetSidAndHostname()
{
	char cLetter;
	char sDrive[8];
	char logbuf[256];
	FILE *fp;
	char config[32];
	char filename[256];
	int cdrom=0;

	for( cLetter = 'D'; cLetter <= 'Z'; cLetter++ )
	{
		sprintf(sDrive, "%c:", cLetter);
		sprintf(config, "%s\\context.sh", sDrive);
		if((fp=fopen(config, "rt")))
		{
			fclose(fp);
			cdrom=1;
			//LOG_FILE(config);
			break;
		}
		
	}
	if( 0 == cdrom)
	{
		LOG_FILE("NO cdrom!");
		strcpy(config, "C:\\context.sh");
		if((fp=fopen(config, "rt")))
		{
			fclose(fp);
			strcpy(sDrive, "C:");
			LOG_FILE(config);
		}
		else
		{
			return -1;
		}
	}

	sprintf(filename, "%s\\context.sh", sDrive);

	if(NULL==(fp=fopen(filename, "rt")))
	{
		sprintf(logbuf, "Open file [%s] failed!\n", filename);
		LOG_FILE(logbuf);
		return -2;
	}

	char hostname[128];
	if(GetFieldValue(filename, "HOSTNAME", hostname))
	{
		//old hostname
        char oldname[128];
        memset(oldname, 0, 128);
        DWORD i=80;
        GetComputerName(oldname,&i);
       
	    char* newsid_file="C:\\newsid.log";
	    if(0 == access(newsid_file, 0) && strcmp(hostname,oldname)==0 )
		{
			LOG_FILE("========================================");
		    LOG_FILE("Newsid and Hostname Had Set, abort.");
		    return 0;
		}
		else if(0 == access(newsid_file, 0) && strcmp(hostname,oldname)!=0)
		{
			LOG_FILE("========================================");
			if(strlen(hostname)>0)
                SetHostname(hostname,oldname);
			else
                LOG_FILE("No Need to Set Hostname!");
		}
        else
		{
            OnlyCreateSID();
		}
	}

   	return 0;
}

int main(int argc, char* argv[])
{
	Sleep(4000);   //wait for setIP.exe
	SetSidAndHostname();

	return 0;
}

