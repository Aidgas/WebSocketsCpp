/* 
 * File:   main.cpp
 * Author: sk
 *
 * Created on 29 июня 2018 г., 11:54
 */
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/epoll.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <fcntl.h>
#include <memory>
#include <vector>
#include <string.h>

#include "include/WebSocket/base64/base64.h"
#include "include/WebSocket/WebSocket.h"

#define PORT                  8383

#define MAXEVENTS             64
#define DEFAULT_BUFFER_SIZE   4096
#define COUNT_EPOLL_WORKERS   2

using namespace std;

struct EventData
{
    int fd;
    unsigned char* buffer;
    int written;

    EventData()
    {
        fd = -1;
        buffer = NULL;
        written = 0;
    }

    ~EventData()
    {
        if(buffer != NULL)
        {
            free(buffer);
            buffer = NULL;
        }
    }
};

struct IOWorkerEpoll
{
    pthread_t th;
    int epoll_fd;
};

struct ParameterDataProcessingWorkerThread
{
    vector<EventData *> queue;
    pthread_rwlock_t rwlock__queue;

    ParameterDataProcessingWorkerThread()
    {
        pthread_rwlock_init(&rwlock__queue, NULL);
    }

    ~ParameterDataProcessingWorkerThread()
    {
       pthread_rwlock_destroy(&rwlock__queue);
    }
};

void exec(int socket, unsigned char* buffer, int len_buffer);

///-----------------------------------------------------------------------------
void create_socket_data(int socket_id)
{
    
}
///-----------------------------------------------------------------------------
void destroy_socket_data(int socket_id, bool block)
{
    shutdown( socket_id, SHUT_RDWR );
    close(socket_id);
}
///-----------------------------------------------------------------------------
void msleep(unsigned int msec)
{
    struct timespec timeout0;
    struct timespec timeout1;
    struct timespec* tmp;
    struct timespec* t0 = &timeout0;
    struct timespec* t1 = &timeout1;

    t0->tv_sec = msec / 1000;
    t0->tv_nsec = (msec % 1000) * (1000 * 1000);

    while(nanosleep(t0, t1) == -1)
    {
        if(errno == EINTR)
        {
            tmp = t0;
            t0 = t1;
            t1 = tmp;
        }
        else
            return;
    }
    return;
}
///-----------------------------------------------------------------------------
void *thread_epoll_data_processin_worker(void *arg)
{
    pthread_setname_np(pthread_self(), "worker");

    struct ParameterDataProcessingWorkerThread *data_quenue = (struct ParameterDataProcessingWorkerThread *) arg;

    /*PGconn *thread_conn;

    char conninfo[255] = "";
    snprintf(conninfo, 255, "user=%s password=%s dbname=%s hostaddr=%s port=%d", PG_USER, PG_PASS, PG_DB, PG_HOST, PG_PORT);

    thread_conn = PQconnectdb(conninfo);

    if (PQstatus(thread_conn) != CONNECTION_OK)
    {
        fprintf(stderr, "Connection to database failed: %s", PQerrorMessage(thread_conn));
        PQfinish(thread_conn);
        pthread_exit(0);
    }*/

    while(true)
    {
        /*if (PQstatus(thread_conn) != CONNECTION_OK)
        {
            PQfinish(thread_conn);

            thread_conn = PQconnectdb(conninfo);

            if (PQstatus(thread_conn) != CONNECTION_OK)
            {
                fprintf(stderr, "Connection to database failed: %s", PQerrorMessage(thread_conn));
                PQfinish(thread_conn);
                sleep(2);

                continue;
            }
        }*/

        while( data_quenue->queue.size() > 0 )
        {
            EventData *_item = data_quenue->queue[0];

            exec(_item->fd, _item->buffer, _item->written);

            pthread_rwlock_wrlock( &data_quenue->rwlock__queue );

            delete _item;

            data_quenue->queue.erase( data_quenue->queue.begin() );

            pthread_rwlock_unlock( &data_quenue->rwlock__queue );
        }

        msleep(15);
    }

    pthread_exit(0);          /* terminate the thread */
}
///-----------------------------------------------------------------------------
void *thread_epoll_worker(void *arg)
{
    pthread_setname_np(pthread_self(), "epoll_worker");

    int epfd = *((int *) arg);
    free( arg );

    int n, i;

    struct epoll_event event, current_event;
    // Buffer where events are returned.
    struct epoll_event* events = static_cast<epoll_event*>(calloc(MAXEVENTS, sizeof event));

    unsigned char buffer_read[DEFAULT_BUFFER_SIZE];
    bool should_close = false, done = false;

    ParameterDataProcessingWorkerThread * data_quenue = new ParameterDataProcessingWorkerThread();

    //data_quenue = (struct ParameterDataProcessingWorkerThread *) calloc(sizeof(struct ParameterDataProcessingWorkerThread), 1);

    /// +++ run thread
    pthread_attr_t pattr;
    pthread_t child;
    // set thread create attributes
    pthread_attr_init(&pattr);
    pthread_attr_setdetachstate(&pattr, PTHREAD_CREATE_DETACHED);

    pthread_create(&child, &pattr, thread_epoll_data_processin_worker, (void*) (data_quenue) );
    /// --- run_thread

    while (true)
    {
        n = epoll_wait(epfd, events, MAXEVENTS, -1);

        for (i = 0; i < n; i++)
        {
            current_event = events[i];

            if (
                   (current_event.events & EPOLLERR)
                || (current_event.events & EPOLLHUP)
                || ( ! (current_event.events & EPOLLIN))
               )
            {
                // An error has occured on this fd, or the socket is not ready for reading (why were we notified then?).
                //fprintf(stderr, "epoll error\n");
                //close(current_event.data.fd);

                destroy_socket_data(current_event.data.fd, true);
                //--total_clients;
            }
            else if (current_event.events & EPOLLRDHUP)
            {
                // Stream socket peer closed connection, or shut down writing half of connection.
                // We still to handle disconnection when read()/recv() return 0 or -1 just to be sure.
                //printf("Closed connection on descriptor vis EPOLLRDHUP %d\n", current_event.data.fd);
                // Closing the descriptor will make epoll remove it from the set of descriptors which are monitored.
                //close(current_event.data.fd);

                destroy_socket_data(current_event.data.fd, true);
                //--total_clients;
            }
            /*else if (sfd == current_event.data.fd)
            {
                // We have a notification on the listening socket, which means one or more incoming connections.
                while (true)
                {
                    struct sockaddr in_addr;
                    socklen_t in_len;
                    int infd;
                    //char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

                    in_len = sizeof in_addr;
                    // No need to make these sockets non blocking since accept4() takes care of it.
                    infd = accept4(sfd, &in_addr, &in_len, SOCK_NONBLOCK | SOCK_CLOEXEC);
                    if (infd == -1)
                    {
                        if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
                        {
                            break;  // We have processed all incoming connections.
                        }
                        else
                        {
                            perror("accept");
                            break;
                        }
                    }

                    // Register the new FD to be monitored by epoll.
                    event.data.fd = infd;
                    // Register for read events, disconnection events and enable edge triggered behavior for the FD.
                    event.events = EPOLLIN | EPOLLRDHUP | EPOLLET;
                    retval = epoll_ctl(epfd, EPOLL_CTL_ADD, infd, &event);
                    if (retval == -1)
                    {
                        perror("epoll_ctl");
                        abort();
                    }

                    create_socket_data(infd);
                    ++total_clients;
                }
            }*/
            else //if (current_event.events & EPOLLIN)
            {
                // We have data on the fd waiting to be read. Read and  display it.
                // We must read whatever data is available completely, as we are running in edge-triggered mode
                // and won't get a notification again for the same data.
                should_close = false;
                done = false;

                while ( ! done )
                {
                    ssize_t count = read(current_event.data.fd, &buffer_read, DEFAULT_BUFFER_SIZE);

                    if (count == -1)
                    {
                        // EAGAIN or EWOULDBLOCK means we have no more data that can be read.
                        // Everything else is a real error.
                        if ( ! ( errno == EAGAIN || errno == EWOULDBLOCK ) )
                        {
                            perror("read");
                            should_close = true;
                        }
                        done = true;
                    }
                    else if (count == 0)
                    {
                        // Technically we don't need to handle this here, since we wait for EPOLLRDHUP. We handle it just to be sure.
                        // End of file. The remote has closed the connection.
                        should_close = true;
                        done = true;
                    }
                    else
                    {
                        EventData *data = new EventData;// = (struct event_data *) calloc(1, sizeof(struct event_data));

                        data->written = count;
                        data->fd = current_event.data.fd;

                        //data.buffer = (unsigned char *)malloc(count);

                        data->buffer = (unsigned char*) calloc( count, 1 );

                        memcpy(data->buffer, buffer_read, count);

                        //printf("count read: %d\n", count);

                        // в очередь
                        pthread_rwlock_wrlock( &data_quenue->rwlock__queue );

                        data_quenue->queue.push_back( data );

                        pthread_rwlock_unlock( &data_quenue->rwlock__queue );

                    }
                }

                if (should_close)
                {
                    //printf("Closed connection on descriptor %d\n", current_event.data.fd);
                    // Closing the descriptor will make epoll remove it from the set of descriptors which are monitored.
                    //close(current_event.data.fd);

                    destroy_socket_data(current_event.data.fd, true);
                    //--total_clients;
                }
            }
        }
    }

    printf("stop thread_epoll_worker");

    free(events);
    delete data_quenue;

    pthread_exit(0);
}

///-----------------------------------------------------------------------------
int make_socket_non_blocking (int sfd)
{
    int flags, s;

    flags = fcntl(sfd, F_GETFL, 0);
    if (flags == -1)
    {
        perror("fcntl");
        return -1;
    }

    flags |= O_NONBLOCK;
    s = fcntl(sfd, F_SETFL, flags);
    if (s == -1)
    {
        perror("fcntl");
        return -1;
    }

    return 0;
}

int main(int argc, char** argv) 
{
    printf("POST: %d\n", PORT);
    
    int sd, value = 1;
    struct sockaddr_in addr;
    //struct ev_io w_accept;

    signal(SIGPIPE, SIG_IGN);

    //connection_db();

    addr.sin_family       = AF_INET;
    addr.sin_port         = htons(PORT);
    addr.sin_addr.s_addr  = INADDR_ANY;

    // Create server socket
    if( (sd = socket(addr.sin_family, SOCK_STREAM, 0)) < 0 )
    {
        perror("socket error");
        return -1;
    }

    if ( setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value)) < 0 )
	{
	    close(sd);
            return -1;
	}

    if ( setsockopt(sd, SOL_SOCKET, SO_KEEPALIVE, &value, sizeof(value)) < 0 )
	{
	    close(sd);
            return -1;
	}

    /*if( setsockopt(sd, SOL_SOCKET, TCP_NODELAY,  &value, sizeof(value)) < 0)
    {
        //printf("ERROR: TCP_NODELAY\n");
        close(sd);
		return -1;
    }*/

    /*if( setsockopt(sd, SOL_SOCKET, SO_SNDBUF,  &sock_buf_size, sizeof(sock_buf_size)) < 0)
    {
        //printf("ERROR_1: SO_SNDBUF\n");
        close(sd);
		return -1;
    }

    if( setsockopt(sd, SOL_SOCKET, SO_RCVBUF,  &sock_buf_size, sizeof(sock_buf_size)) < 0)
    {
        //printf("ERROR_1: SO_RCVBUF\n");
        close(sd);
		return -1;
    }*/

    // Bind socket to address
    if (bind(sd, (struct sockaddr*) &addr, sizeof(addr)) != 0)
    {
        close(sd);
        perror("bind error");
    }

    // Start listing on the socket
    if (listen(sd, 1024) < 0)
    {
        close(sd);
        perror("listen error");
        return -1;
    }
    
    
    struct IOWorkerEpoll workersIO[ COUNT_EPOLL_WORKERS ];

    for(int f = 0; f < COUNT_EPOLL_WORKERS; f++)
    {
        workersIO[f].epoll_fd = epoll_create1(0);

        if (workersIO[f].epoll_fd == -1)
        {
            perror ("epoll_create");
            abort ();
        }

        pthread_attr_t pattr55;
        // set thread create attributes
        pthread_attr_init(&pattr55);
        pthread_attr_setdetachstate(&pattr55, PTHREAD_CREATE_DETACHED);

        int *arg = (int *) malloc(sizeof(*arg));

        if ( arg == NULL )
        {
            fprintf(stderr, "Couldn't allocate memory for thread arg.\n");
            exit(EXIT_FAILURE);
        }

        *arg = workersIO[f].epoll_fd;

        pthread_create(&workersIO[f].th, &pattr55, thread_epoll_worker, arg);
    }
    
    int addlen;
    struct sockaddr_in remoteaddr; // client address

    int index_worker = 0;

    socklen_t len;
    struct sockaddr_storage addr2;
    char ipstr[INET6_ADDRSTRLEN];

    while(true)
    {
        addlen = sizeof(remoteaddr);
        int socket_client = accept( sd, ( sockaddr* )&remoteaddr, (socklen_t*) &addlen);

        if ( socket_client < 0 )
        {
            perror("error accept");
            close( socket_client );
            close( sd );
            exit( EXIT_FAILURE );
        }

        if (make_socket_non_blocking(socket_client) == -1)
        {
            abort();
	}

        create_socket_data(socket_client);
        //++total_clients;

        struct epoll_event event;
        // Register the new FD to be monitored by epoll.
        event.data.fd = socket_client;
        // Register for read events, disconnection events and enable edge triggered behavior for the FD.
        event.events = EPOLLIN | EPOLLRDHUP | EPOLLET;

        //printf("index_worker %d\n", index_worker);

        if (epoll_ctl( workersIO[ index_worker ].epoll_fd, EPOLL_CTL_ADD, socket_client, &event) == -1)
        {
            perror("epoll_ctl");
            abort();
        }

        index_worker += 1;

        if( index_worker >= COUNT_EPOLL_WORKERS )
        {
            index_worker = 0;
        }
    }

    close( sd );
    return 0;
}
///-----------------------------------------------------------------------------
int send_all(int socket, const void *buffer, size_t length, int flags)
{
    ssize_t n;
    const char *p = (const char *)buffer;
    int count_wait = 0;

    while (length > 0)
    {
        n = send(socket, p, length, flags);
        if (n <= 0)
        {
            count_wait += 1;

            if( count_wait > 10 )
            {
                break;
            }

            msleep(100);
            continue;
        }
        p += n;
        length -= n;
    }
    return (n <= 0) ? -1 : 0;
}
///-----------------------------------------------------------------------------
std::string exec_external_cmd(const char* cmd) 
{
    std::array<char, 2048> buffer;
    std::string result;
    
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    
    while (!feof(pipe.get())) 
    {
        if (fgets(buffer.data(), 2048, pipe.get()) != nullptr)
        {
            result += buffer.data();
        }
    }
    return result;
}
///-----------------------------------------------------------------------------
void exec(int socket, unsigned char* buffer, int len_buffer)
{
    WebSocket ws;
    
    int type = ws.parseHandshake(buffer, len_buffer);
    
    ws.debugPrintFrameType( type );
    
    if( type == OPENING_FRAME )
    {
        string answer = ws.answerHandshake();

        /*printf( "--\n");
        printf( "%s\n", answer.c_str() );
        printf( "--\n");*/

        send_all(socket, answer.c_str(), answer.length(), 0 );
    }
    else if( type == INCOMPLETE_FRAME )
    {
        unsigned char data_read[2048], send_data[2048];
        int real_out = 0;
        
        ws.debugPrintFrameType(  ws.getFrame( buffer, len_buffer, data_read, 2048, &real_out )  );
        
        printf( "=[%d] %s\n", real_out, (char *)data_read );
        
        if(real_out == 3)
        {
            FILE *fp = fopen("t.data", "wb");
            
            fwrite(data_read, 1, real_out, fp);
            
            fclose(fp);
        }
        
        string answer = ws.answerHandshake();
        printf( "--\n");
        printf( "%s\n", answer.c_str() );
        printf( "--\n");
        //send_all(socket, answer.c_str(), answer.length(), 0 );
        
        string param = base64_encode(data_read, real_out);
        char cmd[255];
        snprintf(cmd, 255, "php -f 1.php \"%s\"", param.c_str() );
        
        string ss = exec_external_cmd(cmd);
        
        //printf( "exec: %s\n", ss.c_str() );
        
        //int written = snprintf((char*)send_data, 2048, "OK READ MESSAGE %ld", time(NULL) );
        int written = snprintf((char*)send_data, 2048, "%s", ss.c_str() );
        
        int read_output_buffer = ws.makeFrame(TEXT_FRAME, send_data, written, data_read, 2048 );
        
        //printf("%d %d\n", written, read_output_buffer);
        
        send_all(socket, data_read, read_output_buffer, 0 );
    }
    
}
