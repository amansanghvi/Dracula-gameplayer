//By Aman Sanghvi (z5116796) & Melena Jordanova
// COMP1917 Hayden
//Started: 13/04/16 Completed 20/04/16
//Gives the Mandelbrot Set
//Server code and header function written by Richard Buckland
//contains "printf" functions for debugging 


#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include "pixelColor.h"
#include "mandelbrot.h"
#include <math.h>

#define START 5
#define YES 1
#define NO 0
#define BYTES_PER_PIXEL 3
#define BITS_PER_PIXEL (BYTES_PER_PIXEL*8)
#define NUMBER_PLANES 1
#define PIX_PER_METRE 2835
#define MAGIC_NUMBER 0x4d42
#define NO_COMPRESSION 0
#define OFFSET 54
#define DIB_HEADER_SIZE 40
#define NUM_COLORS 0

#define MAX_STEPS 256
#define SIZE 512
#define PIXEL_CENTER 0.5
#define MAX_MODULUS 2

#define SIMPLE_SERVER_VERSION 1.0
#define REQUEST_BUFFER_SIZE 1000
#define DEFAULT_PORT 1558
#define NUMBER_OF_PAGES_TO_SERVE 1000000

typedef unsigned char  bits8;
typedef unsigned short bits16;
typedef unsigned int   bits32;
typedef struct _complex complex;

struct _complex {
    double real;
    double imaginary;
};

typedef struct _color {
     bits8 red;
     bits8 blue;
     bits8 green;
   }color;

static void serveBmp (int serverSocket, double x, double y, int z);
int escapeSteps(double row, double col);
static double mod(complex z);
static int waitForConnection (int serverSocket);
static int makeServerSocket (int portno);
static void writeHeader (int socket);
static double extract(char *request, char value);
static void testExtract(void);

// after serving this many pages the server will halt

int main (int argc, char *argv[]) {
   testExtract();

   printf ("************************************\n");
   printf ("Starting simple server %f\n", SIMPLE_SERVER_VERSION);
   printf ("Serving bmps since 2012\n");
   int serverSocket = makeServerSocket (DEFAULT_PORT);
   printf ("Access this server at http://localhost:%d/\n", DEFAULT_PORT);
   printf ("************************************\n");
   char request[REQUEST_BUFFER_SIZE];

   int numberServed = 0;
   while (numberServed < NUMBER_OF_PAGES_TO_SERVE) {
      printf ("*** So far served %d pages ***\n", numberServed);
      int connectionSocket = waitForConnection (serverSocket);
      int bytesRead;
      bytesRead = read (connectionSocket, request, (sizeof request)-1);
      assert (bytesRead >= 0);
 
      printf ("\n\n *** Received http request ***\n %s\n", request);
      printf ("\n\n *** Sending http response ***\n");
//      printf ("******************************************");
//      printf ("\n\n\n request: \n%s\n\n",request);
//      printf ("********************************");

// Looks at 'request' and responds:
// if a 'tile is requested a bmp is made

   if(request[START] == 't' && request[START+1] == 'i' &&
      request[START+2] == 'l' && request[START+3] == 'e') {

      double x = 0;
      x = extract(request, 'x');
      double y =  0;
      y = extract(request, 'y');
      int z = 0;
      z = extract(request, 'z');
      serveBmp(connectionSocket, x, y, z);

   } else {
//If a specific tile isn't requested, it serves the viewer
      printf("\nServing http\n");
      char* response = "<!DOCTYPE html>\n"
      "<script src=\"http://almondbread.cse.unsw.edu.au/tiles.js\">"
      "</script>";
      write(connectionSocket,response,strlen(response));
   }
      close(connectionSocket);
      numberServed++;
   }

   // close the server connection after we are done- keep aust beautiful
   printf ("** shutting down the server **\n");
   close (serverSocket);

   return EXIT_SUCCESS; 
}

 

static void serveBmp (int socket, double x, double y, int z) {

//    printf("\nbefore header\n");
    char* message;

   // first send the http response header

   // (if you write stings one after another like this on separate
   // lines the c compiler kindly joins them togther for you into
   // one long string)
    message = "HTTP/1.0 200 OK\r\n"
                "Content-Type: image/bmp\r\n"
                "\r\n";
    printf ("about to send=> %s\n", message);
//    printf("\nBefore Sending\n");

    write (socket, message, strlen (message));
//    printf("\After Sending\n");

    writeHeader(socket);


   double zoom = pow(2,-z);
   double midPix = zoom/2;
   double offset = SIZE*zoom/2;
//     printf("\nHeader sent\n");

   double row = x-offset + midPix;
   double col = y-offset + midPix;
   double increment = zoom;

   color shade;
   while (col <= y + offset - midPix) {
      row = x - offset + midPix;
      while(row <= x + offset - midPix) {
         int steps = escapeSteps(row, col);
         shade.blue = stepsToBlue(steps);
         shade.green = stepsToGreen(steps);
         shade.red = stepsToRed(steps);
         write(socket, &shade, sizeof(shade));
         row += increment;
      }
   col += increment;
   }
}

 

// start the server listening on the specified port number
static int makeServerSocket (int portNumber) { 
   
   // create socket
   int serverSocket = socket (AF_INET, SOCK_STREAM, 0);
   assert (serverSocket >= 0);   
   // error opening socket
   
   // bind socket to listening port
   struct sockaddr_in serverAddress;
   memset ((char *) &serverAddress, 0,sizeof (serverAddress));
   
   serverAddress.sin_family      = AF_INET;
   serverAddress.sin_addr.s_addr = INADDR_ANY;
   serverAddress.sin_port        = htons (portNumber);
   
   // let the server start immediately after a previous shutdown
   int optionValue = 1;
   setsockopt (
      serverSocket,
      SOL_SOCKET,
      SO_REUSEADDR,
      &optionValue, 
      sizeof(int)
   );

   int bindSuccess = 
      bind (
         serverSocket, 
         (struct sockaddr *) &serverAddress,
         sizeof (serverAddress)
      );
   
   assert (bindSuccess >= 0);
   // if this assert fails wait a short while to let the operating 
   // system clear the port before trying again
   
   return serverSocket;
}


static int waitForConnection (int serverSocket) {
   // listen for a connection
   const int serverMaxBacklog = 10;
   listen (serverSocket, serverMaxBacklog);
   
   // accept the connection
   struct sockaddr_in clientAddress;
   socklen_t clientLen = sizeof (clientAddress);
   int connectionSocket = 
      accept (
         serverSocket, 
         (struct sockaddr *) &clientAddress, 
         &clientLen
      );

   assert (connectionSocket >= 0);
   // error on accept

   return (connectionSocket);
}

 


static void writeHeader (int socket) {
   assert(sizeof (bits8) == 1);
   assert(sizeof (bits16) == 2);
   assert(sizeof (bits32) == 4);

   bits16 magicNumber = MAGIC_NUMBER;
   write (socket, &magicNumber, sizeof magicNumber);

   bits32 fileSize = OFFSET + (SIZE * SIZE * BYTES_PER_PIXEL);
   write (socket, &fileSize, sizeof fileSize);

   bits32 reserved = 0;
   write (socket, &reserved, sizeof reserved);

   bits32 offset = OFFSET;
   write (socket, &offset, sizeof offset);


   bits32 dibHeaderSize = DIB_HEADER_SIZE;
   write (socket, &dibHeaderSize, sizeof dibHeaderSize);

   bits32 width = SIZE;
   write (socket, &width, sizeof width);

   bits32 height = SIZE;
   write (socket, &height, sizeof height);

   bits16 planes = NUMBER_PLANES;
   write (socket, &planes, sizeof planes);

   bits16 bitsPerPixel = BITS_PER_PIXEL;
   write (socket, &bitsPerPixel, sizeof bitsPerPixel);

   bits32 compression = NO_COMPRESSION;
   write (socket, &compression, sizeof compression);

   bits32 imageSize = (SIZE * SIZE * BYTES_PER_PIXEL);
   write (socket, &imageSize, sizeof imageSize);

   bits32 hResolution = PIX_PER_METRE;
   write (socket, &hResolution, sizeof hResolution);

   bits32 vResolution = PIX_PER_METRE;
   write (socket, &vResolution, sizeof vResolution);

   bits32 numColors = NUM_COLORS;
   write (socket, &numColors, sizeof numColors);

   bits32 importantColors = NUM_COLORS;
   write (socket, &importantColors, sizeof importantColors);

}


int escapeSteps (double x, double y) {
   complex z;
   double realZ =0;
   z.real = x;
   z.imaginary = y;
   int counter = 1;

   while(mod(z) <= MAX_MODULUS && counter < MAX_STEPS) {
      realZ = z.real;
      z.real = pow(z.real, 2) - pow(z.imaginary, 2) + x;
      z.imaginary = 2*realZ*z.imaginary + y;

      counter++;
   }
   return counter;
}

static double mod (complex z) {
   double modulusSquare = pow(z.real, 2) + pow(z.imaginary, 2);
   return sqrt(modulusSquare);

}
//extracts values for x, y & z
static double extract(char *request, char value) {

   int counter = 0;
   int magnitude = 1;
   double size = 0;
   while(request[counter] != value) {
      counter++;
   }
//   printf("%d\n",counter);
   int sign = counter + 1;
   int start = counter+1;
   counter = 0;
   int decimal = NO;

//Magnitude is given in powers of 10

   while(request[start] != '_' && request[start] != 'b') {
//      printf("while %lf\n",size);
      if(request[start] == '.') {
         magnitude = pow(10,counter - 1);
         decimal = YES;
//         printf("mag :%d\n",magnitude);
      } else if (request[start] != '-') {
         size += (request[start]-'0')*pow(10,-counter);
         counter++;
      }
      start++;
//      printf("%lf\n",size);
   }
   double result;
   if (request[sign] == '-') {
      result = 0 - size*magnitude;
   } else {
      result = size*magnitude;
   }
   if (decimal == NO) {
      result = result*pow(10,counter-1);
   }

//   printf("%lf\n",result);
   return result;
}


//tests extract

static testmod(void) {

   complex z;

   z.real = 0;
   z..imaginary = 0;
   assert(mod(z) == 0);

   z.real = 1;
   assert(mod(z) == 1);

   z.real = 0;
   z.imaginary = 1;
   assert(mod(z) == 1);

   z.real = 1;
   assert(mod(z) < sqrt(2) + 0.1 && mod(z) > sqrt(2) - 0.1);


}

static void testExtract(void) {
   char *test =  "http://localhost:7191/tile_x-1.0_y0.5_z8.bmp";
   
   assert(extract(test, 'x') > -2 && extract(test, 'x') < 0);
   assert(extract(test, 'y') > 0 && extract(test, 'y') < 1);
   assert(extract(test, 'z') > 7 && extract(test, 'z') < 9);
   
   test = "http://localhost:7191/tile_x0_y-1_z28.bmp";
   
   assert(extract(test, 'x') > -1 && extract(test, 'x') < 1);
   assert(extract(test, 'y') > -2 && extract(test, 'y') < 0);
   assert(extract(test, 'z') > 27 && extract(test, 'z') < 29);

   test = "http://localhost:7191/tile_x-100_y2_z4.bmp";
   
   assert(extract(test, 'x') > -101 && extract(test, 'x') < -99);
   assert(extract(test, 'y') > 1 && extract(test, 'y') < 3);
   assert(extract(test, 'z') > 3 && extract(test, 'z') < 5);

   test = "http://localhost:7191/tile_x-66_y77_z-2.bmp";
   
   assert(extract(test, 'x') > -67 && extract(test, 'x') < -65);
   assert(extract(test, 'y') > 76 && extract(test, 'y') < 78);
   assert(extract(test, 'z') > -3 && extract(test, 'z') < -1);

}