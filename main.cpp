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
	SocketFactory factory( SocketListener( 8080 ) );
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
	waiter.pushSleepy(
			{ factory.getSocketListener( ) ,
					[ portstr , &i , &waiter , &factory , &working ]()
					{
						try
						{
							SocketHandler in( factory.get( ) );
							Msg request;
							in >> request;
							std::cout << request.getString();
							std::string answer = "";
							{
								std::string str = request.getString();
								std::regex re( "GET (.*)" );
								std::smatch match;
								if( std::regex_search( str , match , re ) )
								{
									std::string str = match[0];
									std::string cmd = std::regex_replace( str , std::regex( " |GET|HTTP/1.1|/" ) , "" );
									std::cout << cmd << "\n";
									if( cmd == "help" )
									{
										answer = "there is no help.";
									} else if( cmd == "info" )
									{
										answer = "parser is working.";
									} else
									{
										answer = "error page";
									}
								}
							}
							if( answer != "" )
							{
								std::stringstream ss;
								ss << "HTTP/1.1 200 OK\n";
								//ss << "Content-length: 46\n";
								ss << "Content-Type: text/html\n\n";
								ss //<< "<html><head></head><body><div><h1>'"
										<< answer;// << "'</h1></div></body></html>";
								std::cout << ss.str() << "\n";
								in << Msg( ss.str() );
							} else
							{
								std::stringstream ss;
								ss << "HTTP/1.1 200 OK\n";
								//ss << "Content-length: 46\n";
								ss << "Content-Type: text/html\n\n";
								ss << "<html><head></head><body><div>simple page</div></body></html>";
								in << Msg( ss.str() );
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
