# THE PEBBLE PROGRAMMING LANGUAGE

PEBBL is a minimal hobby programming language

## HOW TO COMPILE

To compile, first create a build directory, like so:
```sh
mkdir -p build && cd build
```
Next, use the cmake command 

```sh
cmake ..
```

(If you haven't installed cmake,  
use  
```sh
sudo apt update
sudo apt install cmake
```
)  

Then run the generated Makefile

```sh
make
```

> [!NOTE]
> If cmake outputs the ninja build system,  
> you need to use
>```sh
> ninja
>```
> Instead of
>```sh
> make
>```
