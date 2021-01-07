Name: David Shandor	

Exercise 2 - Client 

===Description ===
This program get input from user and connect to the desired website. 
User can get files with or without agruments from server, 
and post (sent) data to server. The connection create with socket. 
In this program I use a struct name request_HTTP, which contain all the components of the request to the server.

Program files:
client.c - file that contain all the program, and create the desired connection to the server.

funcation:
	main - the main of the program, where the program start.
	build_STR 	  - build the string from the request struct components.
	URl_analyze   - parse the URL into host, path and port (if exist).
	post_request  - change command to POST and store data for send to the server.
	r_request 	  - list the agruments after -r n (n = natural number only) to 1 string.  
	check_ARGV	  - check whatever argument is URL, -p, -r or trash argument.
	arg_check	  - validation argument for r_request funcation. 
	del_request   - free all the allocated memory.
	error 		  - used for system call failed (malloc, write, read etc.). free memory and exit the program.
	Usage_err     - used for USAGE misstype. free memory and exit the program.	
-----------


