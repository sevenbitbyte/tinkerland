#ifndef DEBUG_H
#define DEBUG_H

#include <stdlib.h>
#include <iostream>

//#define BUILD_RELEASE
//#define BUILD_TEST
#define BUILD_DEBUG

using namespace std;

#ifdef BUILD_RELEASE
	#define ABORT_ENABLE true
	#define DEBUG_ENABLE false
	#define INFO_ENABLE true
	#define ERROR_ENABLE true
	#define WARNING_ENABLE true
	#define APPEND_ENABLE true
	#define PRINT_ENABLE true
	#define BUILD_STRING "RELEASE"
#endif

#ifdef BUILD_DEBUG
	#define ABORT_ENABLE true
	#define DEBUG_ENABLE true
	#define INFO_ENABLE true
	#define ERROR_ENABLE true
	#define WARNING_ENABLE true
	#define APPEND_ENABLE true
	#define PRINT_ENABLE true
	#define BUILD_STRING "DEBUG"
#endif

#ifdef BUILD_TEST
	#define DEBUG_ENABLE false
	#define ABORT_ENABLE true
	#define INFO_ENABLE true
	#define ERROR_ENABLE true
	#define WARNING_ENABLE true
	#define APPEND_ENABLE true
	#define PRINT_ENABLE true
	#define BUILD_STRING "TEST"
#endif

#if PRINT_ENABLE
	#define PRINT_MSG(msg) std::cout<<__PRETTY_FUNCTION__<<" - "<<msg
#else
	#define PRINT_MSG(msg) //Disabled
#endif


#if INFO_ENABLE
	#define INFO(msg) cout<<__FUNCTION__<<" - INFO, "<<msg
#else
	#define INFO(msg) //Disabled
#endif


#if DEBUG_ENABLE
	#define DEBUG_MSG(msg) std::cout<<__PRETTY_FUNCTION__<<" - DEBUG, "<<msg
#else
	#define DEBUG_MSG(msg) //Disabled
#endif


#if ERROR_ENABLE
	#define ERROR(msg) cerr<<__PRETTY_FUNCTION__<<" - ERROR, "<<msg
#else
	#define ERROR(msg) //Disabled
#endif


#if WARNING_ENABLE
	#define WARNING(msg) cerr<<__PRETTY_FUNCTION__<<" - WARNING, "<<msg
#else
	#define WARNING(msg) //Disabled
#endif

#if UNSUPPORTED_ENABLE
	#define UNSUPPORTED_MSG(msg) cerr<<__PRETTY_FUINCTION__<<" - UNSUPPORTED "<<msg
#else
	#define UNSUPPORTED_MSG(msg) //Disabled
#endif

#if APPEND_ENABLE
	#define APPEND_MSG(msg) cout<<msg
#else
	#define APPEND_MSG(msg) //Disabled
#endif

#if ABORT_ENABLE
	#define ABORT_NOW() cerr<<__PRETTY_FUNCTION__<<" - ABORTED"<<endl; abort()
#else
	#define ABORT_NOW() abort() //Disabled
#endif

#define VERSION_STRING "Floating Cloud Tiered Internet Architecture 2010 - Rochester Institute of Technology\n"\
			BUILD_STRING  " Build " __DATE__ " " __TIME__"\n\n"

#endif // DEBUG_H
