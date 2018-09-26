# mav-shell
A shell program that utilizes fork() and execl() tokenize inputs as commands and execute them from a separate process.  
Features include:
* forking of processes
* tokenization of input
* various directory level access of commands
* history command (lists last 15 commands given)
* listpids (lists last 15 process IDs started from this shell
* !n command where 0 <= n <= 15. Executes the corresponding command as listed from history
* Signal handlers for Ctrl-C and Ctrl-Z
* Suspension of processes with Ctrl-Z
* bg command (backgrounds a previously suspended process)
