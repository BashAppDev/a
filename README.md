# a
A the proggraming laguage;


## How to use: (linux only sorry :( )
Download the main.c (eazy method)
(Simply do this by clicking on it and then on the download icon)
If you have done that install gcc with:
```
sudo apt install gcc

```

then move the main.c to home and run this command:
```
gcc main.c -o a
```

If you have done that move the ./a file to your persenol bin directory
Now download the main.alang to see if everything works...
(Simply do this by clicking on it and then on the download icon)
```
cd Downloads
a main.alang
```

Hopfully it works

## Tutoriol:
# 1. Write your first .alang program
Open your favorite text editor. Create and save a new file named, for example,
my_first.alang

Paste in this code as a first test:

```
var int : age = 25;

while true {
WriteLine("Hi! Your age is "+age);
until while : 3 { break : while; }
}

if age == 25 {
WriteLine("Congratulations on turning 25!");
} else {
WriteLine("Not 25 yet!");
}
```
# 2. Run the program
```
a my_first.alang
```
You should see:

text
Hi! Your age is 25
Hi! Your age is 25
Hi! Your age is 25
Congratulations on turning 25!

# 3. Experiment! Try more features
Variables
```
var int : x = 10;
var str : name = "Alex";
var float : pi = 3.14;
```
WriteLine and Concatenation
```
WriteLine("Value of x is "+x);
WriteLine("Hello "+name);
```
Conditions (if/else)
text
if x == 10 {
    WriteLine("x is 10");
} else {
    WriteLine("x is not 10");
}
```

Custom Functions
```
fcn hello(var str : name, var int : age) {
    WriteLine("Hello "+name);
    WriteLine("You are "+age);
}

fcn : hello(: "Sam", : 42);
```
While loops with break
```
while true {
WriteLine("Looping...");
until while : 3 { break : while; }
}
```
# 4. Try spacing and indentation
You can use tabs or spaces for better readability:

```
while true {
    WriteLine("Indented is fine!");
    until while : 2 { break : while; }
}
```
