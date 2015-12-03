#include <tcp/Socket.h>
#include <tcp/SocketHandler.h>
#include <tcp/HTTPRequestFactory.h>
#include <list>
#include <tcp/SocketFactory.h>
#include <tcp/SocketListener.h>
#include <sys/signal.h>
#include <tcp/Socks4Manager.h>
#include <iostream>
#include <string>
#include <regex>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
template< typename T , typename P >
void operator-=( std::list< T > &out , P i )
{
	out.remove( i );
}
template< typename T >
void operator+=( std::list< T > &out , T const &i )
{
	out.push_back( i );
}
void sigpipeHndl( int sign )
{

}
std::vector< std::string > &split( const std::string &s , char delim , std::vector< std::string > &elems )
{
	std::stringstream ss( s );
	std::string item;
	while( std::getline( ss , item , delim ) )
	{
		if( item.length( ) > 0 )
		{
			elems.push_back( item );
		}
	}
	return elems;
}
std::vector< std::string > split( const std::string &s , char delim )
{
	std::vector< std::string > elems;
	split( s , delim , elems );
	return elems;
}
int main( int argc , char *argv[] )
{
	signal( SIGPIPE , sigpipeHndl ); //atoi( argv[ 1 ] )
	/*std::string str = "GET /help HTTP/1.1";
	 std::cout << str << "\n";
	 std::regex re( "GET (.*)" );
	 std::smatch match;
	 //std::regex_match( str , sm , e );
	 std::regex_search( str , match , re );
	 for( auto i : match )
	 {
	 std::cout << std::regex_replace( str , std::regex( " |GET|HTTP/1.1|/" ) , "" );
	 }
	 exit( 0 );*/
	ushort const port = 8080;
	std::string const portstr = std::to_string( port );
	SocketFactory factory;
	tryagain: try
	{
		std::cout << "trying to bind socket listener...\n";
		factory = std::move( SocketFactory( SocketListener( 8080 ) ) );
	} catch( std::exception const &e )
	{
		sleep( 1 );
		std::cout << "fail\n";
		goto tryagain;
	}
	std::cout << "success\n";
	struct Pipe
	{
		int i;
		SocketHandler in;
		SocketHandler out;
		bool operator==( Pipe const &p ) const
		{
			return p.i == i;
		}
	};
	Waiter waiter;
	int i = 0;
	bool working = true;
	waiter.pushSleepy( { factory.getSocketListener( ) , [ portstr , &i , &waiter , &factory , &working ]()
	{
		try
		{
			SocketHandler in( factory.get( ) );
			Msg request;
			in >> request;
			//std::cout << request.getString();
			Msg answer;
			std::string content_type = "text/html";
			{
				std::string str = request.getString();
				std::regex re( "GET (.*)" );
				std::smatch match;
				if( std::regex_search( str , match , re ) )
				{
					std::string str = match[0];
					std::string cmd = std::regex_replace( str , std::regex( " |GET|HTTP/1.1|/" ) , "" );
					cmd = std::regex_replace( cmd , std::regex( "%20" ) , " " );
					std::vector< std::string > elems;
					std::vector< std::string > smds = split( cmd , '$' );

					/*if( cmd == "help" )
					 {
					 answer = "there is no help.";
					 } else if( cmd == "info" )
					 {
					 answer = "parser is working.";
					 } else
					 {
					 answer = "error page";
					 }*/
					for( std::string cmd : smds )
					{
						std::stringstream ss;
						struct stat sb;
						std::cout << cmd + "\n";
						FILE *fp;
						unsigned char buf[ 0x10000 ];
						fp = fopen( cmd.c_str() , "rb" );
						if( fp != NULL )
						{
							std::smatch match;
							if( std::regex_search( cmd , match , std::regex( "(.*)png" ) ) )
							{
								content_type = "image/png";
							} else if( std::regex_search( cmd , match , std::regex( "(.*)jpg" ) ) )
							{
								content_type = "image/jpg";
							}
							fseek( fp , 0L , SEEK_END );
							unsigned long size = ftell( fp );
							rewind( fp );
							std::cout << "file size: " << size << "\n";
							int integ = 0;
							int l = 0;

							while( ( l = read( fp ->_fileno, buf , 0x10000 ) ) > 0 )
							{
								integ += l;
								answer.add( buf , l );
							}
							std::cout << "file read: " << answer.size() << " " << integ << "\n";
							fclose( fp );
						} else
						{
							fp = popen( cmd.c_str() , "r");
							char buf[ 0x10000 ];
							if( fp != NULL )
							{
								while( fgets( buf , 0x1000 , fp ) != NULL )
								{
									ss << buf;
								}
								pclose(fp);
							}
						}
						answer.add( ( byte*)ss.str().c_str() , ss.str().size() );
					}
				}
			}
			if( answer.size() != 0 )
			{
				//answer = "<html><head></head><body>" + answer + "</body></html>";
				Msg final_answer;
				final_answer << "HTTP/1.1 200 OK\n";
				final_answer << "Content-length: ";
				final_answer << std::string( std::to_string( answer.size() ) );
				final_answer << "\n";
				final_answer << "Content-Type: ";
				final_answer << content_type;
				final_answer << "\n\n";
				final_answer << answer;
				//std::cout << answer << "\n";
				in << final_answer;
			} else
			{
				Msg final_answer;
				final_answer << "HTTP/1.1 200 OK\n";
				//ss << "Content-length: 46\n";
				final_answer << "Content-Type: text/html\n\n";
				final_answer << "<html><head></head><body><div>simple page</div></body></html>";
				in << final_answer;
			}

		} catch( std::exception const &e )
		{
			std::cerr << "main catcher:" << e.what() << "\n";
			//working = false;
		}
	} , Waiter::READ } );
	while( working )
	{
		try
		{
			waiter.wait( 100 );
		} catch( std::exception const &e )
		{
			std::cerr << "waiter catcher:" << e.what( ) << "\n";
			working = false;
		}
	}
	return 0;
}
