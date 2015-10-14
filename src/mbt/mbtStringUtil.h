#ifndef __MBT_STRING_UTIL_H__
#define __MBT_STRING_UTIL_H__

#include "fbsdk\fbsdk.h"

namespace mbt
{
	class FBStringComparerFunc
	{
	public:
		virtual ~FBStringComparerFunc() {}
		virtual bool operator()(const char* p1, const char* p2)const = 0;
	};

	class FBStringComparerEquals : public FBStringComparerFunc
	{
	public:
		virtual ~FBStringComparerEquals() {}
		virtual bool operator()(const char* p1, const char* p2)const;
	};

	class FBStringComparerFind : public FBStringComparerFunc
	{
	public:
		virtual ~FBStringComparerFind() {}
		virtual bool operator()(const char* p1, const char* p2)const;
	};
}

#endif // __MBT_STRING_UTIL_H__