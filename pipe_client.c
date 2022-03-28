
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define server_fifo "/tmp/SERVER_FIFO"
#define client_fifo "/tmp/CLIENT_FIFO"
#define DATA_SIZE 1024
#define FILE_NAME_SIZE 100

/*
 * Structure for each packet to send data 
 */
struct file_data
{
  char file_name[FILE_NAME_SIZE];
  int bytes_sent ; 
  char data_size[DATA_SIZE];
}data;

/*
 * Helper Method to check if named pipes are present 
 */
int check_named_pipes()
{
  int result = 1 ; 
  if( access( server_fifo, F_OK|R_OK|W_OK ) == -1)
  {
    printf("Not present Server_FIFO file : %s\n", server_fifo);
    result = -1 ; 
  }
  if( access( client_fifo, F_OK|R_OK|W_OK ) == -1)
  {
    printf("Not present Client_FIFO file : %s\n", client_fifo);
    result = -1 ; 
  }
  return result ; 
}

/*
 * Helper method to connect to server fifo and send the 
 * file to server fifo with a block size of DATA_SIZE 
 */
void send_file_to_server_fifo(char* file_to_be_sent)
{

  int fd = open(file_to_be_sent, O_RDONLY);
  if(fd < 0 )
  {
    printf("Error Opening file \n");
    return ; 
  }
  int bytes_written ; 
  int server_fd = open(server_fifo, O_WRONLY);
  if (server_fd > 0)
  {
    while ( data.bytes_sent = read(fd , &data.data_size, DATA_SIZE )  ) 
    {
        bytes_written = write(server_fd, &data, sizeof(data));
        printf("Wrote %d bytes on pipe and File Block Size = %d \n", bytes_written , data.bytes_sent );
        memset(data.data_size, 0 , sizeof(DATA_SIZE));
        if(data.bytes_sent == 0 )
          break ; 
    }
    close(server_fd);
  }

  close(fd);
}

/*
 * Helper method to connect to client fifo and listen to messages 
 * from the server. Currently it listens to only a single message 
 * can be modified to accept multiple messages. 
 */
void listen_to_message_from_server()
{
  char message_buffer[DATA_SIZE];
  memset(message_buffer, 0 , DATA_SIZE);
  int client_fd = open(client_fifo , O_RDONLY );
  int read_bytes = read(client_fd,&message_buffer , DATA_SIZE);
  if (read_bytes)
    printf("Got the message from server : %s \n", message_buffer );
  printf("OK, Closing now connection with the server \n");
  close(client_fd);
}

int main(int argc, char* argv[])
{
  if (argc < 2 )
  {
    printf("Please enter file name to be transferred \n");
    return -1 ; 
  }
  if( check_named_pipes() == -1 ) 
  {
    printf("Named pipe not present\nKindly start pipe_server first \n");
    return -1 ; 
  }
  if( access( argv[1], F_OK|R_OK|W_OK ) == -1 ) 
  {
    printf("File not present please run again \n");
    return -1 ; 
  }
  //Copy name of the file file_name of the data structure 
  strcpy( data.file_name , argv[1]);
  
  send_file_to_server_fifo(argv[1]);
  listen_to_message_from_server();
   
  return 0 ; 
}
