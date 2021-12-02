##################################################################
##							        ##
##           How to use the Producer/Consumer program           ##
##							        ##
##################################################################

1. File Requirements
	A. This program requires the "buffer.h" header file provided.
	   Ensure to keep inside the same location as where this
	   program is located to prevent the possibility of issues.

2. Compiling the Application
	A. Type the following command in the command-promt:

		gcc -o psynch psynch.c -lpthread -lrt

	B. The -lpthread and -lrt are used to link two special
           libraries to provide multithreading and semaphore support.


3. Executing the Application
	A. Use the following format to execute the program:
		
		./psynch [# producer threads] [# consumer threads] [# items]

	B. Example:

		./psynch 9 3 2


4. Restrictions & Tools
	A. This program requires a total of 4 arguments, including the name
	   of the program. If arguments are missing, then a usage prompt will
	   be displayed instead.

	B. The number of producer and consumer threads have to be greater than 0.
	   The program will exit prematurely if 0 or a negative number 
	   of threads is entered.

	C. Deadlock prevention has been implemented. In the chance that an untested input
	   causes deadlock, using CTRL + C will terminate the program.
