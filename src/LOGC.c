#include "LOGC.h"

/*
 * iLOG3Lite - log function library written in c
 * author	: calvin
 * email	: calvinwilliams.c@gmail.com
 * LastVersion	: v1.0.9
 *
 * Licensed under the LGPL v2.1, see the file LICENSE in base directory.
 */

/* 代码宏 */
#define OFFSET_BUFPTR(_buffer_,_bufptr_,_len_,_buflen_,_remain_len_) \
	if( _len_ > 0 && _buflen_+_len_ <= sizeof(_buffer_)-1 ) \
	{ \
		_bufptr_ += _len_ ; \
		_buflen_ += _len_ ; \
		_remain_len_ -= _len_ ; \
	} \

/* 日志文件名 */
TLS char		g_log_pathfilename[ MAXLEN_FILENAME + 1 ] = "" ;
TLS int			g_log_level = LOGLEVEL_INFO ;
TLS int			g_file_fd = -1 ;
TLS struct timeval	g_time_tv = { 0 , 0 } ;
TLS char		g_date_and_time[ 10 + 1 + 8 + 2 ] = "" ;
TLS unsigned long	g_pid = 0 ;
TLS unsigned long	g_tid = 0 ;

const char log_level_itoa[][6] = { "DEBUG" , "INFO" , "WARN" , "ERROR" , "FATAL" } ;

/* 打开日志文件 */
static int OpenLogFile()
{
	CloseLogFile();
	
#if ( defined __linux__ ) || ( defined __unix ) || ( defined _AIX )
	g_file_fd = OPEN( g_log_pathfilename , O_CREAT | O_WRONLY | O_APPEND , S_IRWXU | S_IRWXG | S_IRWXO ) ;
#elif ( defined _WIN32 )
	g_file_fd = OPEN( g_log_pathfilename , _O_CREAT | _O_WRONLY | _O_APPEND | _O_BINARY , _S_IREAD | _S_IWRITE ) ;
#endif
	if( g_file_fd == -1 )
		return -1;
	
	return 0;
}

/* 设置日志文件名，并打开文件 */
void SetLogFile( char *format , ... )
{
	va_list		valist ;
	
	if( g_file_fd != -1 )
	{
		CLOSE( g_file_fd );
	}
	
	va_start( valist , format );
	VSNPRINTF( g_log_pathfilename , sizeof(g_log_pathfilename)-1 , format , valist );
	va_end( valist );
	
	return;
}

/* 关闭日志文件 */
void CloseLogFile()
{
	if( g_file_fd != -1 )
	{
		CLOSE( g_file_fd );
		g_file_fd = -1 ;
	}
}

/* 设置日志等级 */
void SetLogLevel( int log_level )
{
	g_log_level = log_level ;
	
	return;
}

/* 输出日志 */
int WriteLogBaseV( int log_level , char *c_filename , long c_fileline , char *format , va_list valist )
{
	char		c_filename_copy[ MAXLEN_FILENAME + 1 ] ;
	char		*p_c_filename = NULL ;
	
	char		log_buffer[ 1024 + 1 ] ;
	char		*log_bufptr = NULL ;
	size_t		log_buflen ;
	size_t		log_buf_remain_len ;
	size_t		len ;
	
	int		nret = 0 ;
	
	/* 处理源代码文件名 */
	memset( c_filename_copy , 0x00 , sizeof(c_filename_copy) );
	strncpy( c_filename_copy , c_filename , sizeof(c_filename_copy)-1 );
	p_c_filename = strrchr( c_filename_copy , '\\' ) ;
	if( p_c_filename )
		p_c_filename++;
	else
		p_c_filename = c_filename_copy ;
	
	/* 填充行日志 */
	/* memset( log_buffer , 0x00 , sizeof(log_buffer) ); */
	log_bufptr = log_buffer ;
	log_buflen = 0 ;
	log_buf_remain_len = sizeof(log_buffer) - 1 ;
	
	/*
	gettimeofday( & g_time_tv , NULL );
	len = SNPRINTF( log_bufptr , log_buf_remain_len , "%s.%06ld | %-5s | %lu:%lu:%s:%ld | " , g_date_and_time , g_time_tv.tv_usec , log_level_itoa[log_level] , g_pid , g_tid , p_c_filename , c_fileline ) ;
	*/
	len = SNPRINTF( log_bufptr , log_buf_remain_len , "%s | %-5s | %lu:%lu:%s:%ld | " , g_date_and_time , log_level_itoa[log_level] , g_pid , g_tid , p_c_filename , c_fileline ) ;
	OFFSET_BUFPTR( log_buffer , log_bufptr , len , log_buflen , log_buf_remain_len );
	len = VSNPRINTF( log_bufptr , log_buf_remain_len , format , valist );
	OFFSET_BUFPTR( log_buffer , log_bufptr , len , log_buflen , log_buf_remain_len );
	len = SNPRINTF( log_bufptr , log_buf_remain_len , NEWLINE ) ;
	OFFSET_BUFPTR( log_buffer , log_bufptr , len , log_buflen , log_buf_remain_len );
	
	/* 输出行日志 */
	if( g_file_fd == -1 )
	{
		nret = OpenLogFile() ;
		if( nret )
			return nret;
		
		WRITE( g_file_fd , log_buffer , log_buflen );
	}
	else
	{
		WRITE( g_file_fd , log_buffer , log_buflen );
	}
	
	return 0;
}

int WriteLogBase( int log_level , char *c_filename , long c_fileline , char *format , ... )
{
	va_list		valist ;
	
	int		nret = 0 ;
	
	va_start( valist , format );
	nret = WriteLogBaseV( log_level , c_filename , c_fileline , format , valist );
	va_end( valist );
	
	return 0;
}

#if ( defined __STDC_VERSION__ ) && ( __STDC_VERSION__ >= 199901 )

#else

int WriteLog( int log_level , char *c_filename , long c_fileline , char *format , ... )
{
	va_list		valist ;
	
	if( log_level < g_log_level )
		return 0;
	
	va_start( valist , format );
	WriteLogBaseV( log_level , c_filename , c_fileline , format , valist );
	va_end( valist );
	
	return 0;
}

int FatalLog( char *c_filename , long c_fileline , char *format , ... )
{
	va_list		valist ;
	
	if( LOGLEVEL_FATAL < g_log_level )
		return 0;
	
	va_start( valist , format );
	WriteLogBaseV( LOGLEVEL_FATAL , c_filename , c_fileline , format , valist );
	va_end( valist );
	
	return 0;
}

int ErrorLog( char *c_filename , long c_fileline , char *format , ... )
{
	va_list		valist ;
	
	if( LOGLEVEL_ERROR < g_log_level )
		return 0;
	
	va_start( valist , format );
	WriteLogBaseV( LOGLEVEL_ERROR , c_filename , c_fileline , format , valist );
	va_end( valist );
	
	return 0;
}

int WarnLog( char *c_filename , long c_fileline , char *format , ... )
{
	va_list		valist ;
	
	if( LOGLEVEL_WARN < g_log_level )
		return 0;
	
	va_start( valist , format );
	WriteLogBaseV( LOGLEVEL_WARN , c_filename , c_fileline , format , valist );
	va_end( valist );
	
	return 0;
}

int InfoLog( char *c_filename , long c_fileline , char *format , ... )
{
	va_list		valist ;
	
	if( LOGLEVEL_INFO < g_log_level )
		return 0;
	
	va_start( valist , format );
	WriteLogBaseV( LOGLEVEL_INFO , c_filename , c_fileline , format , valist );
	va_end( valist );
	
	return 0;
}

int DebugLog( char *c_filename , long c_fileline , char *format , ... )
{
	va_list		valist ;
	
	if( LOGLEVEL_DEBUG < g_log_level )
		return 0;
	
	va_start( valist , format );
	WriteLogBaseV( LOGLEVEL_DEBUG , c_filename , c_fileline , format , valist );
	va_end( valist );
	
	return 0;
}

#endif

int WriteHexLogBaseV( int log_level , char *c_filename , long c_fileline , char *buf , long buflen , char *format , va_list valist )
{
	char		hexlog_buffer[ 4096 * 10 + 1 ] ;
	char		*hexlog_bufptr = NULL ;
	size_t		hexlog_buflen ;
	size_t		hexlog_buf_remain_len ;
	size_t		len ;
	
	int		row_offset , col_offset ;
	
	int		nret = 0 ;
	
	if( buf == NULL && buflen <= 0 )
		return 0;
	if( buflen > sizeof(hexlog_buffer) - 1 )
		return -1;
	
	/* 输出行日志 */
	if( format )
	{
		WriteLogBaseV( log_level , c_filename , c_fileline , format , valist );
	}
	
	/* 填充十六进制块日志 */
	memset( hexlog_buffer , 0x00 , sizeof(hexlog_buffer) );
	hexlog_bufptr = hexlog_buffer ;
	hexlog_buflen = 0 ;
	hexlog_buf_remain_len = sizeof(hexlog_buffer) - 1 ;
	
	len = SNPRINTF( hexlog_bufptr , hexlog_buf_remain_len , "             0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F    0123456789ABCDEF" ) ;
	OFFSET_BUFPTR( hexlog_buffer , hexlog_bufptr , len , hexlog_buflen , hexlog_buf_remain_len );
	len = SNPRINTF( hexlog_bufptr , hexlog_buf_remain_len , NEWLINE ) ;
	OFFSET_BUFPTR( hexlog_buffer , hexlog_bufptr , len , hexlog_buflen , hexlog_buf_remain_len );
	
	row_offset = 0 ;
	col_offset = 0 ;
	while(1)
	{
		len = SNPRINTF( hexlog_bufptr , hexlog_buf_remain_len , "0x%08X   " , row_offset * 16 ) ;
		OFFSET_BUFPTR( hexlog_buffer , hexlog_bufptr , len , hexlog_buflen , hexlog_buf_remain_len );
		for( col_offset = 0 ; col_offset < 16 ; col_offset++ )
		{
			if( row_offset * 16 + col_offset < buflen )
			{
				len = SNPRINTF( hexlog_bufptr , hexlog_buf_remain_len , "%02X " , *((unsigned char *)buf+row_offset*16+col_offset)) ;
				OFFSET_BUFPTR( hexlog_buffer , hexlog_bufptr , len , hexlog_buflen , hexlog_buf_remain_len );
			}
			else
			{
				len = SNPRINTF( hexlog_bufptr , hexlog_buf_remain_len , "   " ) ;
				OFFSET_BUFPTR( hexlog_buffer , hexlog_bufptr , len , hexlog_buflen , hexlog_buf_remain_len );
			}
		}
		len = SNPRINTF( hexlog_bufptr , hexlog_buf_remain_len , "  " ) ;
		OFFSET_BUFPTR( hexlog_buffer , hexlog_bufptr , len , hexlog_buflen , hexlog_buf_remain_len );
		for( col_offset = 0 ; col_offset < 16 ; col_offset++ )
		{
			if( row_offset * 16 + col_offset < buflen )
			{
				if( isprint( (int)*(buf+row_offset*16+col_offset) ) )
				{
					len = SNPRINTF( hexlog_bufptr , hexlog_buf_remain_len , "%c" , *((unsigned char *)buf+row_offset*16+col_offset) ) ;
					OFFSET_BUFPTR( hexlog_buffer , hexlog_bufptr , len , hexlog_buflen , hexlog_buf_remain_len );
				}
				else
				{
					len = SNPRINTF( hexlog_bufptr , hexlog_buf_remain_len , "." ) ;
					OFFSET_BUFPTR( hexlog_buffer , hexlog_bufptr , len , hexlog_buflen , hexlog_buf_remain_len );
				}
			}
			else
			{
				len = SNPRINTF( hexlog_bufptr , hexlog_buf_remain_len , " " ) ;
				OFFSET_BUFPTR( hexlog_buffer , hexlog_bufptr , len , hexlog_buflen , hexlog_buf_remain_len );
			}
		}
		len = SNPRINTF( hexlog_bufptr , hexlog_buf_remain_len , NEWLINE ) ;
		OFFSET_BUFPTR( hexlog_buffer , hexlog_bufptr , len , hexlog_buflen , hexlog_buf_remain_len );
		if( row_offset * 16 + col_offset >= buflen )
			break;
		row_offset++;
	}
	
	/* 输出十六进制块日志 */
	if( g_file_fd == -1 )
	{
		nret = OpenLogFile() ;
		if( nret )
			return nret;
		
		WRITE( g_file_fd , hexlog_buffer , hexlog_buflen );
	}
	else
	{
		WRITE( g_file_fd , hexlog_buffer , hexlog_buflen );
	}
	
	return 0;
}

int WriteHexLogBase( int log_level , char *c_filename , long c_fileline , char *buf , long buflen , char *format , ... )
{
	va_list		valist ;
	
	int		nret = 0 ;
	
	va_start( valist , format );
	nret = WriteHexLogBaseV( log_level , c_filename , c_fileline , buf , buflen , format , valist ) ;
	va_end( valist );
	
	return 0;
}

#if ( defined __STDC_VERSION__ ) && ( __STDC_VERSION__ >= 199901 )

#else

int WriteHexLog( int log_level , char *c_filename , long c_fileline , char *buf , long buflen , char *format , ... )
{
	va_list		valist ;
	
	if( log_level < g_log_level )
		return 0;
	
	va_start( valist , format );
	WriteHexLogBaseV( log_level , c_filename , c_fileline , buf , buflen , format , valist );
	va_end( valist );
	
	return 0;
}

int FatalHexLog( char *c_filename , long c_fileline , char *buf , long buflen , char *format , ... )
{
	va_list		valist ;
	
	if( LOGLEVEL_FATAL < g_log_level )
		return 0;
	
	va_start( valist , format );
	WriteHexLogBaseV( LOGLEVEL_FATAL , c_filename , c_fileline , buf , buflen , format , valist );
	va_end( valist );
	
	return 0;
}

int ErrorHexLog( char *c_filename , long c_fileline , char *buf , long buflen , char *format , ... )
{
	va_list		valist ;
	
	if( LOGLEVEL_ERROR < g_log_level )
		return 0;
	
	va_start( valist , format );
	WriteHexLogBaseV( LOGLEVEL_ERROR , c_filename , c_fileline , buf , buflen , format , valist );
	va_end( valist );
	
	return 0;
}

int WarnHexLog( char *c_filename , long c_fileline , char *buf , long buflen , char *format , ... )
{
	va_list		valist ;
	
	if( LOGLEVEL_WARN < g_log_level )
		return 0;
	
	va_start( valist , format );
	WriteHexLogBaseV( LOGLEVEL_WARN , c_filename , c_fileline , buf , buflen , format , valist );
	va_end( valist );
	
	return 0;
}

int InfoHexLog( char *c_filename , long c_fileline , char *buf , long buflen , char *format , ... )
{
	va_list		valist ;
	
	if( LOGLEVEL_INFO < g_log_level )
		return 0;
	
	va_start( valist , format );
	WriteHexLogBaseV( LOGLEVEL_INFO , c_filename , c_fileline , buf , buflen , format , valist );
	va_end( valist );
	
	return 0;
}

int DebugHexLog( char *c_filename , long c_fileline , char *buf , long buflen , char *format , ... )
{
	va_list		valist ;
	
	if( LOGLEVEL_DEBUG < g_log_level )
		return 0;
	
	va_start( valist , format );
	WriteHexLogBaseV( LOGLEVEL_DEBUG , c_filename , c_fileline , buf , buflen , format , valist );
	va_end( valist );
	
	return 0;
}

#endif
