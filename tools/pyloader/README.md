## What is this?
Pyloader is a python project loader.It allows user to bundles a Python application and all its dependencies into a single package.And you can run it without installing a python interpreter or any modules.

It's not an easy tool to build project "exe" or "elf",it needs user do some changes.It can build for Windows and Linux. 

## Advantages
* Encrypt your project source files by adding a decrypt function in pythonxx.dll(eg: python34.dll).
* Update your project easily.
* Easily to customise.
* Prevent loader from normal injection.

## How to use
1. build your pyloader from source.
2. package your library.zip by build_library_zip.py
3. prepare your folder like example.
4. run pyloader.exe
