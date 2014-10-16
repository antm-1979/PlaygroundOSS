struct sockaddr_in sa;
struct hostent     *hp;
int a, s;

char hostname[128];
if ((hp = gethostbyname("192.168.1.7")) == NULL) {
	return(-1);
}

memset(&sa, 0, sizeof(sa));
memcpy((char *)&sa.sin_addr, hp->h_addr, hp->h_length);
sa.sin_family = hp->h_addrtype;
sa.sin_port = htons((u_short)5050);

if ((s = socket(hp->h_addrtype, SOCK_STREAM, 0)) < 0)
	return(-1);
if (connect(s, (sockaddr *)&sa, sizeof(sa)) < 0) {
	closesocket(s);
	return(-1);
}


//by qxy
#define MAXHOSTNAME 64
int establish(unsigned short portnum)
{
	char   myname[MAXHOSTNAME + 1];
	int    s;
	struct sockaddr_in sa;
	struct hostent *hp;
	memset(&sa, 0, sizeof(struct sockaddr_in));
	gethostname(myname, MAXHOSTNAME);
	hp = gethostbyname(myname);
	if (hp == NULL)
		return(-1);
	sa.sin_family = hp->h_addrtype;
	sa.sin_port = htons(portnum);
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return(-1);
	if (bind(s, (sockaddr *)&sa, sizeof(struct sockaddr_in)) < 0) {
		close(s);
		return(-1);
	}
	listen(s, 3);
	int t;
	for (unsigned i = 0; hp->h_addr_list[i]; i++)
	{
		char* ip = inet_ntoa(*(struct in_addr*)hp->h_addr_list[i]);
		char buf[1024];
		sprintf(buf, "%s", ip);
		t = t;
	}
	if ((t = accept(s, NULL, NULL)) < 0)
		return(-1);

	return(s);
}


