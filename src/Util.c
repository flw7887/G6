#include "G6.h"

/* 取随机数工具函数 */
int Rand( int min, int max )
{
	return ( rand() % ( max - min + 1 ) ) + min ;
}

/* 计算字符串HASH工具函数 */
unsigned long CalcHash( char *str )
{
	unsigned long	hashval ;
	unsigned char	*puc = NULL ;
	
	hashval = 19791007 ;
	for( puc = (unsigned char *)str ; *puc ; puc++ )
	{
		hashval = hashval * 19830923 + (*puc) ;
	}
	
	return hashval;
}

/* 转换为守护进程 */
int BindDaemonServer( char *pcServerName , int (* ServerMain)( void *pv ) , void *pv , int (* ControlMain)(long lControlStatus) )
{
	int pid;
	
	pid=fork();
	switch( pid )
	{
		case -1:
			return -1;
		case 0:
			break;
		default		:
			exit( 0 );	
			break;
	}

	setsid() ;

	pid=fork();
	switch( pid )
	{
		case -1:
			return -2;
		case 0:
			break ;
		default:
			exit( 0 );
			break;
	}
	
	setuid( getpid() ) ;
	
	umask( 0 ) ;
	
	return ServerMain( pv );
}

/* 判断字符串匹配性 */
int IsMatchString(char *pcMatchString, char *pcObjectString, char cMatchMuchCharacters, char cMatchOneCharacters)
{
	int el=strlen(pcMatchString);
	int sl=strlen(pcObjectString);
	char cs,ce;

	int is,ie;
	int last_xing_pos=-1;

	for(is=0,ie=0;is<sl && ie<el;){
		cs=pcObjectString[is];
		ce=pcMatchString[ie];

		if(cs!=ce){
			if(ce==cMatchMuchCharacters){
				last_xing_pos=ie;
				ie++;
			}else if(ce==cMatchOneCharacters){
				is++;
				ie++;
			}else if(last_xing_pos>=0){
				while(ie>last_xing_pos){
					ce=pcMatchString[ie];
					if(ce==cs)
						break;
					ie--;
				}

				if(ie==last_xing_pos)
					is++;
			}else
				return -1;
		}else{
			is++;
			ie++;
		}
	}

	if(pcObjectString[is]==0 && pcMatchString[ie]==0)
		return 0;

	if(pcMatchString[ie]==0)
		ie--;

	if(ie>=0){
		while(pcMatchString[ie])
			if(pcMatchString[ie++]!=cMatchMuchCharacters)
				return -2;
	} 

	return 0;
}

/* 绑定CPU亲缘性 */
int BindCpuAffinity( int processor_no )
{
	cpu_set_t	cpu_mask ;
	int		nret = 0 ;
	
	CPU_ZERO( & cpu_mask );
	CPU_SET( processor_no , & cpu_mask );
	nret = sched_setaffinity( 0 , sizeof(cpu_mask) , & cpu_mask ) ;
	
	return nret;
}

void UpdateTimeNow( struct timeval *p_time_tv , char *p_date_and_time )
{
	struct tm		stime ;
	
#if ( defined __linux__ ) || ( defined __unix ) || ( defined _AIX )
	p_time_tv->tv_sec = time( NULL ) ;
	localtime_r( &(p_time_tv->tv_sec) , & stime );
	
	snprintf( p_date_and_time , sizeof(g_date_and_time)-1 , "%04d-%02d-%02d %02d:%02d:%02d" , stime.tm_year+1900 , stime.tm_mon+1 , stime.tm_mday , stime.tm_hour , stime.tm_min , stime.tm_sec );
#elif ( defined _WIN32 )
	{
	SYSTEMTIME	stNow ;
	GetLocalTime( & stNow );
	time( & (p_time_tv->tv_sec) );
	}
	
	snprintf( p_date_and_time , sizeof(g_date_and_time)-1 , "%04d-%02d-%02d %02d:%02d:%02d" , stNow.wYear , stNow.wMonth , stNow.wDay , stNow.wHour , stNow.wMinute , stNow.wSecond );
#endif
	
	
	return;
}

