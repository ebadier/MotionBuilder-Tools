#ifndef __MBT_UTIL_H__
#define __MBT_UTIL_H__

#include "fbsdk\fbsdk.h"

namespace mbt
{
	/**
	*	\brief Do a Parent-Child Connexion.
	*	\return true if successful, false otherwise.
	*/
	bool Parent(FBPlug* pChild, FBPlug* pParent);

	/**
	*	\brief Undo a Parent-Child Connexion.
	*	\return true if successful, false otherwise.
	*/
	bool UnParent(FBPlug* pChild, FBPlug* pParent);

	/**
	*	\brief get the list of Children.
	*	\param pOutList : the list of children.
	*	\param pRoot : Model to search from.
	*	\param pRec : do it recursively.
	*/
	void GetChildren( FBModelList& pOutList, FBModel* pRoot, bool pRec = false );

	/**
	*	\brief get a FBModel named pName.
	*	\param pMatchWholeWord : name search criteria (whole name or partial name).
	*	\return NULL if not found.
	*/
	FBModel* GetModel( const FBString& pName, bool pMatchWholeWord );

	/**
	*	\brief Plot Animation on the given model using the given options.
	*/
	void PlotAnimation( FBPlotOptions* options, FBModel* pRoot, bool pIncludeChildren = true );
}

#endif // __MBT_UTIL_H__