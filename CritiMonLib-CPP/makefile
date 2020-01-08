SOURCES = APIException.cpp APIHandler.cpp CookieObj.cpp CritiMon.cpp SignalException.cpp

lib_name = libCritiMon-CPP.so.1.0.0.0

#openssl_include = /usr/include/openssl/
#openssl_libs = /usr/lib64/openssl/
boost_inc_path = /usr/include/boost
boost_lib_path = /usr/lib64/boost

libcurl_include =/usr/include/curl
libcurl_lib = /usr/lib64/curl

ifeq ($(HOSTNAME), ci-server)
        rapidjson_inc_path = /usr/include/rapidjson
else
        rapidjson_inc_path = /builds/rapidjson
endif



OBJECTS = $(SOURCES:.cpp=.o)
#CFLAGS = -fpic -c $(SOURCES) -Wall -g -Iincludes -I/usr/include/cppconn -I$(rapidjson_inc_path) -I$(openssl_include) -I$(libcurl_include) -std=c++11 -I$(boost)
CC = g++
#LDFLAGS = -lpthread -pthread -lm -L/usr/lib64/ -lmysqlcppconn -L$(openssl_libs) -lssl -lcrypto -L$(libcurl_lib) -lcurl

CFLAGS =  -fpic -c $(SOURCES) -Wall -g -Iincludes -I$(rapidjson_inc_path) -I$(openssl_include) -I$(libcurl_include) -std=c++11 -I$(boost_inc_path) -DBOOST_STACKTRACE_USE_ADDR2LINE
LDFLAGS = -lpthread -pthread -lm -L/usr/lib64/ -L$(openssl_libs) -lssl -lcrypto -L$(libcurl_lib) -lcurl -L$(boost_lib_path) -lboost_filesystem -ldl

.PHONY: clean

default:
        $(CC) -shared -Wl,-soname,libCritiMon-CPP.so.1 -o $(lib_name) $(OBJECTS) $(LDFLAGS)
        ln -sf $(lib_name) libCritiMon-CPP.so

clean:
        rm -vf $(OBJECTS) depend $(lib_name)

depend: $(SOURCES)
        $(CC) $(CFLAGS) > depend

-include depend

~
~
~
~
~
~
~
~
~
