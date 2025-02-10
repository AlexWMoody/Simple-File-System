Alex Moody V00962257
CSC360 Assignment 3 

Files included:
diskinfo.c
disklist.c
diskget.c 
diskput.c

My design works as follows:

diskinfo.c 
read the file system superblock data into the superblock struct and fatinfo struct. 
read using specific byte sizes and fread.
Then loop through fat to count allocated, reserved, and free blocks based on their entry value.

disklist.c
builds on code from diskinfo.c.
finds directory it wants to list by tokenizing path string and then using recursion until finding the correct directory. It then calls a non-recursive function.
This then loops through the directory, recording in directory_entry struct what the data is of the files and directories being listed and prints them out in the correct format. 

diskget.c 
builds on code from disklist.c.
uses same tokenization and recurive location of directories as disklist.c. The correct named output file in linux directory is also created. Once the correct file in .img file is located, I use the FAT to locate and copy all the blocks and write them into the output file in the linux directory. if the specefied file is never found, it prints "file not found"

diskput.c
builds on code from diskget.c.
uses same tokenization and recurive location of directories as disklist.c and diskget.c. The file in the linux directory is checked to be real and is opened to be read from.
find the correct directory i the img file or print file not found.
check if the disk has enough space for the file. use stat to get file size and creation date. 
create a new directory entry for the file in the correct directory within the img file. Find the available FAT entries in the FAT table and record them. Update them to point to the correct blocks for the file. Update those blocks with the copied over information from the file in the linux directory. update the FAT blcok info as well. 

Thank you:) Have a nice day/afternoon/evening! 

