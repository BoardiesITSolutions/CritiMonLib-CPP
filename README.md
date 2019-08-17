<img src="https://critimon.com/images/logo.png" width="150">

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

# Gotchas
There's one thing to be aware of with the C++ CritiMon library. It will capture unhandled
exceptions at the application level, not at the lower level. E.g. if there's a problem
with your app due to a bad pointer for example, or an issue that will cause your app to
receive SIGSEGV or SIGABRT signal CritiMon can't capture these type of crashes. This is due to 
when this is happens, it is highly likely the stack or heap will be corrupted and there's no way that
the app can perform an additional action after receiving this type of error. 

The CritiMon library however, will automatically send crashes in the event that your application
throws some sort of exception, but the exception wasn't caught by a try/catch block. 

Note, there doesn't appear to be a way to get the exception itself from the termination handler.
Instead, when an uncaught exception is detected, an exception will be created as a `runtime_error`
with the exception message `Unhandled Exception Detected`. You will however, receive a complete
stacktrace that lead up to the crash. 

Have any issues or need some help, please visit our support portal at 
https://support.boardiesitsolutions.com

Sign up for a free account by visiting https://critimon.boardiesitsolutions.com

CritiMon - Copyright &copy; 2019 - Boardies IT Solutions

<img src="https://boardiesitsolutions.com/images/logo.png"> 
