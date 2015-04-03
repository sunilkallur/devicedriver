# devicedriver
Learning linux device drivers!!! 

The kernelmodule.c file defines the character device. 
Clone the repository and execute make to compile the kernel module.
To insert the module execute : 

insmod kernelerlmodule.ko 

To check the module type : dmesg 
Note the major number and the message to type in the last line something like :

mknod /dev/sunildevice c 250 0 

Note : 250 is the major number and 0 is the minor number

Now inside the application folder, compile the simple user application :

gcc -o app app.c

Then to run the application :

./app 

This might throw error stating the module is locked by other process or has no rights to access ! It is the rights issue now and to provide access execute :

sudo chmod 777 /dev/sunildevice 

Now the application will run and request for command. 

