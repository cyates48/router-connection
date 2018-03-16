# router-connection

The router.c file acts as a router that is on a network using a link-state algorithm. If you were to open this file on four different windows and run them all, they will all output the least cost path from one router (node) to another. It does this because each file accepts four arguments: the node's host ID (0-3), the total number of nodes (4), the costs file (a 4x4 table of numbers), and the machines file (four machines that are locally found). 

The main function first parses the two files, and creates the machine array (each machine is constructed as a struct) and costs table (2D array). After, it creates the UDP socket connection and opens a connection to the 3 other routers. It then creates two pthreads that will run at their various times. I created a mutex that will lock and unlock the access to the costs table because we do not want the pthreads and main function to access it at the same time (like concurrent transactions). Finally, it runs on an infinite loop that accepts two inputs, which are another router aside from itself (0-3) and the new cost to get from the host node to the input router. 

The pthread, receiveinfo(), runs on an infinite loop that updates and prints the costs table to each router after every time the main function receives the two inputs. 

The last pthread, link_state(), also runs on an infinite loop that calculates the new least cost path from one router to the other three routers. It does this calculation every ten seconds and prints an array from which the least cost paths are stored, regardless if the costs table had been updated or not. 
