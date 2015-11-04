#include <tcp/Socket.h>
#include <tcp/SocketHandler.h>
#include <tcp/HTTPRequestFactory.h>
#include <list>
#include <tcp/SocketFactory.h>
#include <tcp/SocketListener.h>
#include <sys/signal.h>
#include <tcp/Socks4Manager.h>
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
	waiter.pushSleepy( factory.getSocketListener( ) , [ &i , &waiter , &factory , &working ]()
	{
		try
		{
			SocketHandler in( factory.get( ) );

			Msg request;
			in >> request;
			std::cout << request.getString() << "\n";
			std::stringstream ss;
			ss << "<head></head><body><br>hello number " << (i++);
			ss << "</br></body>";
			in << Msg( ss.str() );
		} catch( std::exception const &e )
		{
			std::cerr << "main catcher:" << e.what() << "\n";
			//working = false;
		}
	} );
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
