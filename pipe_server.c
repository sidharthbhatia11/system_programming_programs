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
 * If named pipes are not present it will create them 
 */
void check_and_create_named_pipes()
{
  if( access( server_fifo, F_OK|R_OK|W_OK ) == -1)
  {
    printf("Creating Server_FIFO file : %s\n", server_fifo);
    mknod(server_fifo, S_IFIFO|0640, 0);
  }
  if( access( client_fifo, F_OK|R_OK|W_OK ) == -1)
  {
    printf("Creating Client_FIFO file : %s\n", client_fifo);
    mknod(client_fifo, S_IFIFO|0640, 0);
  }
}


/*
 * Helper method to connect to server fifo and read the 
 * data being sent. It checks the file name if present in the current dir, 
 * It file already present it add _s postfix to the new file it will create 
 * in which it will write the data. 
 */
void read_on_server_fifo()
{
  int server_fd , file_write_fd ; 
  int read_bytes ; 
  printf("Reading on the Server fifo for client connection \n");
  server_fd = open(server_fifo , O_RDONLY );
  int file_check = 1 ; 
  char file_name[FILE_NAME_SIZE];
  while(1)
  {
    read_bytes = read(server_fd,&data,sizeof(data));
    printf("Read %d bytes on pipe and File Block Size = %d \n", read_bytes , data.bytes_sent );
    if(file_check)
    {
      printf("Checking for file present in pipe_Server dir = %s\n", data.file_name);
      strcpy(file_name,data.file_name) ; 
      if ( access(file_name , F_OK ) != -1 ) 
      {
        printf("File already present in server location renaming it with postfix _s\n");
        const char* postfix =  "_s" ;
        strncat(file_name, "_s", 3);
      }
      file_check = 0 ;
      printf("Creating file = %s\n", file_name);
      file_write_fd = open(file_name, O_WRONLY | O_CREAT , 0642 ) ;
    }

    write(file_write_fd, data.data_size , data.bytes_sent  );

    memset(&data,0,sizeof(data));
    if(!read_bytes )
    {
      break ;
      close(file_write_fd );
    }
  }
  close(server_fd);
}

/*
 * Helper method to connect to client fifo and send message 
 * from the server. Currently it sends a single message only. 
 * It can be modified to send multiple messages. 
 */
void send_done_on_client_fifo()
{
  printf("Sending ALL DONE to client \n") ;
  char* message = "ALL DONE";
  int client_fd = open(client_fifo, O_WRONLY);
  int bytes_written = write(client_fd, message, sizeof(message));
  close(client_fd);

}

int main(int argc, char* argv[])
{
  check_and_create_named_pipes();
  while(1)
  {
    read_on_server_fifo();
    send_done_on_client_fifo();
  }
}
