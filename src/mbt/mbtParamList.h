#ifndef __MBT_PARAMLIST_H__
#define __MBT_PARAMLIST_H__

#include <map>

/**
*	\brief [std::string, T] map.
*/
template<typename T>
class ParamList : public std::map<std::string, T>
{
public:
	bool HasParam(const std::string& pParamName)const
	{
		return count(pParamName) > 0;
	}
};

/**
*	\brief [std::string, int] map.
*/
typedef ParamList<int> ParamListInt;

#endif // __MBT_PARAMLIST_H__