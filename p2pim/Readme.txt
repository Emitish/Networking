Emily Sison 
997279300

Files:
p2pim.cpp
Makefile

Usage:
Use Makefile to get the executable 'p2pim'.
Just running p2pim will set the defaults.
Options are available by typing 'p2p [options]'.
Press enter to send intial Discovery Message.
Keep Pressing Enter- uses Poll:
Finding type 1, finds a discovery message and then sends a reply.
Finding type 2, receives a reply and gets a list of users
Exit the program with an exit signal.
Program will send a Closing message before exiting with signal. 

Establishing a TCP connection with hostinfo is in the code, but not necessarily works.


Resources:
Smartsite- udpbroadcast.c, gethostinfo.c, tcpclient.c
Piazza - https://piazza.com/class/ht8vwov6xkp65r?cid=33 and various other posts
http://www.phon.ucl.ac.uk/courses/spsci/abc/lesson11.htm
http://www.linuxquestions.org/questions/programming-9/chat-program-in-c-580778/
http://www.cplusplus.com/reference/string/string/c_str/
http://www.cplusplus.com/reference/vector/vector/?kw=vector
http://publib.boulder.ibm.com/infocenter/tivihelp/v8r1/index.jsp?topic=%2Fcom.ibm.networkmanagerip.doc_3.9%2Fitnm%2Fip%2Fwip%2Fpoll%2Ftask%2Fnmip_poll_monitornw.html