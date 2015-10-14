#include "mbtStringUtil.h"

#include "mbtUtil.h"

namespace mbt
{
	//---------------------------------------------------------------------------------------//
	bool Parent(FBPlug* pChild, FBPlug* pParent)
	{
		return FBConnect(pChild, pParent);
	}

	//---------------------------------------------------------------------------------------//
	bool UnParent(FBPlug* pChild, FBPlug* pParent)
	{
		return FBDisconnect(pChild, pParent);
	}

	//---------------------------------------------------------------------------------------//
	void GetChildren( FBModelList& pOutList, FBModel* pRoot, bool pRec /*= false*/ )
	{
		if(pRoot)
		{
			for (int i = 0; i < pRoot->Children.GetCount(); ++i)
			{
				FBModel* child = pRoot->Children[i];
				pOutList.Add(child);
				if(pRec)
				{
					GetChildren(pOutList, child, pRec);
				}
			}
		}
	}

	//---------------------------------------------------------------------------------------//
	FBModel* GetModel( const FBString& pName, bool pMatchWholeWord )
	{
		static FBStringComparerEquals sFBStringComparerEquals;
		static FBStringComparerFind sFBStringComparerFind;

		FBModel* ret = NULL;
		FBStringComparerFunc* func;
		if(pMatchWholeWord)
		{
			func = &sFBStringComparerEquals;
		}
		else
		{
			func = &sFBStringComparerFind;
		}

		FBModelList allModels;
		GetChildren(allModels, FBSystem::TheOne().Scene->RootModel, true);
		for (int i = 0; i < allModels.GetCount(); ++i)
		{
			FBModel* child = allModels[i];
			if(func->operator()(child->Name, pName))
			{
				ret = child;
				break;
			}
		}
		return ret;
	}

	//---------------------------------------------------------------------------------------//
	void PlotAnimation(FBPlotOptions* options, FBModel* pRoot, bool pIncludeChildren )
	{
		FBModelList models;
		if(pIncludeChildren)
		{
			GetChildren(models, pRoot, true);
		}
		models.Add(pRoot);

		// Select only the desired models
		FBModelList selectedModels;
		FBGetSelectedModels(selectedModels);
		for (int i=0; i < selectedModels.GetCount(); ++i)
		{
			selectedModels[i]->Selected = false;
		}
		for (int i = 0; i < models.GetCount(); ++i)
		{
			models[i]->Selected = true;
		}

		// Plot Animation on the desired models.
		FBSystem::TheOne().CurrentTake->PlotTakeOnSelected(options);

		// Reset Selection
		for (int i = 0; i < models.GetCount(); ++i)
		{
			models[i]->Selected = false;
		}
		for (int i=0; i < selectedModels.GetCount(); ++i)
		{
			selectedModels[i]->Selected = true;
		}
	}

	//---------------------------------------------------------------------------------------//
}