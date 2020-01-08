<img src="https://critimon.boardiesitsolutions.com/images/logo.png" width="150">

# Introduction
The CritiMon C++ library allows you to send both handled and unhandled crashes to the CritiMon 
Crash Monitoring service at https://critimon.boardiesitsolutions.com. 

# Prerequisites
There are couple of dependencies that are required to be linked to for the C++ library to function
correctly. 
* boost 1.70.0+ (include boost filesystem) - https://www.boost.org/users/download/
* rapidjson - https://github.com/Tencent/rapidjson

# Building CritiMon Library
There's a makefile already available, although may need tweaking to update to the correct paths
to where it can find boost and rapidjson. 

Note that the below is only required for Linux compilation. On Windows there's nothing special
you need to do. Note that the project was created in Visual Studio 2019 so you will need to use
at least this version if opening in Visual Stuido. 

Aprt from including rapdidjson and boost and linking to the boost filesystem you also need to ensure 
that you link to the library dl to ensure crash stacktrace information is shown. You also need
to ensure that the compiler flag BOOST_STACKTRACE_USE_ADDR2LINE is also passed in during the compile. 
Failing to do the above may result in the stacktrace of the crash information not being readable.

# Installing
There's no specific place you need to have the library for installation just as long as it is somewhere
that you main application can reference. For example, on Linux this might be in /usr/lib64/CritiMon. 

In your main applications project, add reference to where the CritiMon header files can be located,
and where the CritiMon library can be found. Once ready, you can then add a #include to to the CritiMon.h
header file. Once included you can now instantiate the CritiMon library.

# Using the library
To use the library create an instance of the CritiMon library. You can create a new instance on each
class you have within your project and initialise each time, but the best, and most efficient way
would be to instantiate CritiMon on the heap and ensure that it is accessible through out your project. 

## Instantiate on the stack - initialise on every class
```
    #include <CritiMon.h>
    CritiMon critimon;
    critimon.initialise("<api_key>", "<app_id>", "<version>");
```

## Instantiate on the heap - usable by everywhere within your project
```
    #include <CritiMon.h>
    CritiMon *critimon = new CritiMon();
    critimon->Initialise("<api_key>", "<app_id>", "<version>");
```

Your API key can be found under the settings on the CritiMon website, next to it you will find
a button to allow you to copy the API key directly to your clipboard. 

If you are only interested in receiving unhandled crashes from your C++ applications, then this is all 
you need to do. 

However, handled exceptions, are just as important as handled ones, therefore the CritiMon library
has the ability for you to send a crash if required within your catch handler. 

You will only be able to do this for exception that have a type e.g. `catch(std::exception)`
generic catch all exception like `catch(...)` cannot be used to send a crash. 

To send a crash you can use the `SendCrash` method within your instance of the critimon object. You need
to send your exception object and a crash severity. The crash severity is available via a the CrashSeverity
enum within the CritiMon class. The following severities are available:

* Low
* Medium
* Major
* Critical

```
try
{
    //Perform some action here so that an exception is thrown
}
catch (std::exception& ex)
{
    critimon.SendCrash(ex, CrashSeverity::Low);
}
```

You can also send a crash with custom properties to help you diagnose why a certain crash happened,
this might be particular app settings, or parameters that passed into the method. This can be done in one
of two ways, either a basic key and value or as a map of key/value pairs as shown below

```
try
{
    //Perform some action here so that an exception is thrown
}
catch (std::exception& ex)
{
    //Send a single key/value pair custom property along with the crash
    critimon.SendCrash(ex, CrashSeverity::Low, "my_key", "my_value");

    //Send an array of key/value pairs
    std::map<std::string, std::string> customProperties;
    customProperties["item 1"] = "value 1";
    customProperties["item 2"] = "value 2";
    customProperties["item 3"] = "value 3";

    critimon.SendCrash(ex, CrashSeverity::Low, customProperties);
}
```

## What Crashes from C++ are Sent to CritiMon?
Any crash that C++ has should be caught from CritiMon. For example, any uncaught exceptions that
are part of, or inherit std::exception will be sent to CritiMon, as either a handled exception (if caught
by a try/catch block), and an unhandled exception if not caught by a try/catch block. 

Crashes that are due to a process receiving a signal are also captured, which are detailed below. 
Internally, CritiMon will register a signal handler for the signals below and when a signal is raised
it will internally raise a SignalException with the details of the signal that was received. It will contain
all of the same information as a normal crash triggered by std::exception. You might find that the class
and line number details may potentially show that it has come from an internal C++ library such as libc as
its usually an internal C++ library that raises the signal (unless you raise your own signal within your own app). 
Another potential issue may be in the event of a serious crash where the stack that CritiMon retrieves may not be 
useful. This is usually due to a really bad bug, which may have caused a stack corruption or memory corruption,
which then may result in the stacktrace not being available or is not valid. 

The following signals are caught by CritiMon
* SIGABRT - Abort signal was received
* SIGFPE - Erreoneous arithmetic operation was done, such as a divide by zero
* SIGILL - A processor command error was attempted - possibily unrecognised processor command
* SIGSEGV - A memory access violation

One thing to be aware of with the signals, if your own app registers any of the above signals after
CritiMon initialises - the crash will not be detected as your own signal handler will override CritiMons. The same
if your register the signals before CritiMon, your own signal handler will be overidden by the CritiMon library. 

We do not register SIGINT signal as this is usually a user initiated graceful shutdown so it is not a crash
and therefore you are fine to register a SIGINT handler within your application without any impact to CritiMon.

Have any issues or need some help, please visit our support portal at 
https://support.boardiesitsolutions.com

Sign up for a free account by visiting https://critimon.com

CritiMon - Copyright &copy; 2019 - Boardies IT Solutions

<img src="https://boardiesitsolutions.com/images/logo.png"> 