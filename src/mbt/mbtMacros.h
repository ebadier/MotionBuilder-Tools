#ifndef __MBT_MACROS_H__
#define __MBT_MACROS_H__

#include "fbsdk/fbsdk.h"

#include <sstream>

#ifdef _MSC_VER
	#define NOMINMAX
	#include <exception>
#else
	#include <stdexcept>
#endif

#define MIN(x,y) ((x)<(y)?(x):(y))
#define MAX(x,y) ((x)>(y)?(x):(y))

///\brief Format : "File(line)[Function] -> message
#define MBT_FileLineFunc(X)		__FILE__ << "(" << __LINE__ << ") [" << __FUNCTION__ << "] -> " << X

///\brief print message into Motion Builder console.
#define MBT_Out(X)				{ std::stringstream ss; ss << X << "\n"; FBTrace(ss.str().c_str()); }
#define MBT_Out_Er(X)			MBT_Out(MBT_FileLineFunc(X))
///\brief print message into Motion Builder Popup.
#define MBT_Popup(X)			{ std::stringstream ss; ss << X << "\n"; FBMessageBox("Warning", ss.str().c_str(), "OK"); }
///\brief print message into Motion Builder console + Popup.
#define MBT_OutAndPopup(X)		{ std::stringstream ss; ss << X << "\n"; const std::string& str = ss.str(); \
	FBTrace(str.c_str()); FBMessageBox("Warning", str.c_str(), "OK"); }
#define MBT_Warning(X)			MBT_OutAndPopup(MBT_FileLineFunc(X))

#ifdef _MSC_VER
	///\brief Throw an exception.	
	#define MBT_Exit(X)	{std::stringstream ss; ss << MBT_FileLineFunc("Exception : " << X); \
		throw std::exception(ss.str().c_str()); }
#else
	///\brief Throw an exception.
	#define MBT_Exit(X)	{std::stringstream ss; ss << MBT_FileLineFunc("Exception : " << X); \
		throw std::runtime_error(ss.str().c_str()); }
#endif

///\brief print message into Motion Builder console if XTest is true.
#define MBT_Test_O_Er(XTest,YMessage,RetValue)	if(XTest){MBT_Out_Er(YMessage); return RetValue;}
#define MBT_Test_O(XTest,YMessage,RetValue)		if(XTest){MBT_Out(YMessage); return RetValue;}
///\brief print message into Motion Builder Popup if XTest is true.
#define MBT_Test_P(XTest,YMessage,RetValue)		if(XTest){MBT_Popup(YMessage); return RetValue;}
///\brief print message into Motion Builder console + Popup if XTest is true.
#define MBT_Test_W_MSG(XTest,YMessage,RetValue)	if(XTest){MBT_Warning(YMessage); return RetValue;}
#define MBT_Test_W(XTest,RetValue)				MBT_Test_W_MSG(XTest, #XTest, RetValue)
///\brief Throw an Exception if XTest is true.
#define MBT_Test_E_MSG(XTest,YMessage)			if(XTest){MBT_Exit(YMessage);}
#define MBT_Test_E(XTest)						MBT_Test_E_MSG( XTest, #XTest )

///\brief Debug Only Macros
#ifdef NDEBUG
	#define MBT_DebugOut(X)
	#define MBT_DebugOut_Er(X)
	#define MBT_DebugWarning(X)
	#define MBT_DebugTest_O(XTest,YMessage,RetValue)
	#define MBT_DebugTest_O_Er(XTest,YMessage,RetValue)
	#define MBT_DebugTest_P(XTest,YMessage,RetValue)
	#define MBT_DebugTest_W_MSG(XTest,YMessage,RetValue)
	#define MBT_DebugTest_W(XTest,RetValue)
	#define MBT_DebugTest_E_MSG(XTest,YMessage)
	#define MBT_DebugTest_E(XTest)
#else
	#define MBT_DebugOut		MBT_Out
	#define MBT_DebugOut_Er		MBT_Out_Er
	#define MBT_DebugWarning	MBT_Warning
	#define MBT_DebugTest_O		MBT_Test_O
	#define MBT_DebugTest_O_Er	MBT_Test_O_Er
	#define MBT_DebugTest_P		MBT_Test_P
	#define MBT_DebugTest_W_MSG	MBT_Test_W_MSG
	#define MBT_DebugTest_W		MBT_Test_W
	#define MBT_DebugTest_E_MSG	MBT_Test_E_MSG
	#define MBT_DebugTest_E		MBT_Test_E
#endif

#endif // __MBT_MACROS_H__