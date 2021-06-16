# jayshell
Custom built UNIX shell <br />
Future development: Allow both Windows and Linux commands
Made with lots of SYSCALLs in C!

![alt text](https://i.imgur.com/fE6wkMN.jpg)

## Features
1.	Uses ANSI sequence to add color and bold to prompter for visual assistance

2.	Run simple commands like ls, pwd, cat, wc, sort etc. with their options 

3.	Erronous inputs/commands handled

4.	Run any number of commands separated by semi-colon (;)

5.	Pipe 2 commands

6.	Pipe 3 commands

7.	Redirect output if ‘>’ or ‘>[WHITESPACE]’ present. The following examples are processed the same way: 
(i)	sort thisfile.txt >newfile.txt
(ii)	sort thisfile.txt > newfile.txt

8.	Redirect input if ‘<’ or ‘<[WHITESPACE]’ present.

9.	If input file doesn’t exist, prompts error message

10.	Appends output if ‘>>’ or ‘>>[WHITESPACE]’ present.

11.	Exit shell

