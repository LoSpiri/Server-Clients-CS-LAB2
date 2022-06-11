# Project for the exam Programming Laboratory 2 

<div align="center">
<img hight="300" width="700" alt="GIF" align="center" src="https://res.cloudinary.com/practicaldev/image/fetch/s--dWwH4rJ4--/c_limit%2Cf_auto%2Cfl_progressive%2Cq_66%2Cw_880/https://media2.giphy.com/media/fnD9cHHIrYRYk/giphy.gif">
</div>
(Please forgive me professor if the gif is inappropriate.)
</br>
</br>
The project is about creating a C script (farm.c) that reads from the command line the names of multiple binary files and writes the names on a shared buffer. It then launches multiple threads that read from that shared buffer, open the files and compute the numbers written on them.<br />
In the meantime a Python script (collector.py) is used as a server to receive via socket the results of the threads' computations and print those on stdout.
<br />
<br />
Implementation choices:<br />
The C threads and the shared buffer are managed using semaphores and locks, with a classical consumer/producer mechanism.<br />
They send via socket a string and beforehand an integer to communicate the string's length.<br />
The Python script uses the socketserver library to manage the socket connection and ThreadingMixin to create multiple threads that parallel compute the socket requests. Used this setup as it was the easiest in my opinion to do so.<br />
The server shutdown is done by the C script by sending an end message in the same way as with the computations.<br />
The requested SIGINT managing by the C script is done using an handler thread that flags a variable to let the C program know that it should just finish with what's already on the buffer and then close the python server and itself.<br />
