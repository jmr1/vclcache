vclcache - a c++ compiler cache for Microsoft Visual Studio
-----------------------------------------------------------

vclcache attempts to avoid unnecessary recompilation by reusing previously cached object files if possible. It is meant to be called instead of the original cl.exe executable. The program analyses the command line, source file and its header dependencies to decide whether the source file is to be compiled. If so, a cache will be queried for a previously stored object file.

The program also supports multiple source files in one invocation, can process with /Zi option but caching the external pdb file is not supported yet (compiler will display warnings about missing debug info).


Installation
------------

Build 32 bit version of vclcache (optionally 64 bit version for 64 bit compilation but it is not necessary).

In the below locations rename original cl.exe into cl_real.exe and make a copy of its corresponding cl.exe.config to cl_real.exe.config (your paths may be different):
- 32bit compilation in C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\bin
- 32 to 64 bit cross compilation in C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\bin\x86_amd64 (VS ide on 64 bit Windows also uses that)
- 64 bit compilation in C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\bin\amd64

Copy already built cl.exe into above locations. Optionally copy 64 bit version of cl.exe into 64 bit location but 32 bit cl.exe will work too (64 bit cl.exe version in 32 bit locations won't work). 
Also copy all the dll's the new cl.exe depends on or add the dll location to the PATH.


Environment Variables
---------------------

VCLCACHE_DIR
If set, points to the directory within which all the cached object files should be stored. This defaults to C:\.cache

VCLCACHE_OFF
If set to 'true', the original cl.exe will be invoked, unset or set to false to enable caching

VCLCACHE_MODE
If set to 'hash', the caching program will use hash function to determine whether the source file needs recompilation
If set to 'timestamp', the caching program will use last modified time to determine whether the source file needs recompilation
Unset defaults to 'hash'

VCLCACHE_STRIP_COMMENTS
If set to 'true' all comments from the source code will be stripped before hashing so that it does not cause unnecessary recompilation, set to 'false' leaves file unchanged
Unset defaults to 'true'

How vclcache works
------------------

vclcache intercepts calls to the actual cl.exe. 
If an object file is not present in the cache the call is forwarded to the actual cl.exe. After successful compilation the object file will be cached. 
If the object file is present in the cache it will be copied to designated location without invoking the actual compiler.


Credits
-------

vclcache was heavily inspired by [clcache] and [ccache].

[clcache]:https://github.com/frerich/clcache
[ccache]:http://ccache.samba.org/
