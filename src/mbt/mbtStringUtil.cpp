#include "mbtStringUtil.h"

namespace mbt
{
	//---------------------------------------------------------------------------------------//
	bool FBStringComparerEquals::operator()(const char* p1, const char* p2)const
	{
		return strcmp(p1,p2) == 0;
	}

	//---------------------------------------------------------------------------------------//
	bool FBStringComparerFind::operator()(const char* p1, const char* p2)const
	{
		return strstr(p1, p2) != NULL;
	}

	//---------------------------------------------------------------------------------------//
}