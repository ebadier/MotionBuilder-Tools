#ifndef __BVH_TO_FBX_TOOL_H__
#define __BVH_TO_FBX_TOOL_H__

#include "mbt/mbtGUI.h"

/**
*	\brief Tool to convert bvh files to fbx files without the use of a Character (default Motion Builder batch conversion).
*	So the motion is not altered during a batch conversion process.
*/
class BvhToFbxTool : public FBTool
{
	//--- FiLMBOX Tool declaration.
	FBToolDeclare( BvhToFbxTool, FBTool );

public:
	//--- FiLMBOX constructor & destructor
	virtual bool FBCreate();		//!< FiLMBOX Constructor.
	virtual void FBDestroy();		//!< FiLMBOX Destructor.

private:
	// UI Management
	void	UICreate	();
	void	UIConfigure	();
	void	UIConnect	();
	void	UIDisconnect();

	// UI Callbacks
	void	EventOnInputDirPopupButtonClick		( HISender pSender, HKEvent pEvent );
	void	EventOnOutputDirPopupButtonClick	( HISender pSender, HKEvent pEvent );
	void	EventOnForceFPSButtonClick			( HISender pSender, HKEvent pEvent );
	void	EventOnProcessButtonClick			( HISender pSender, HKEvent pEvent );

	void						GetBVHFiles(FBStringList& pBVHFiles);
	double						GetBVHFrameTime(const FBString& pBvhFilename)const;
	bool						ForceFPS()const;
	bool						SetFPS(const FBString& pBvhFilename)const;
	void						ResampleAnimation(FBModel* pSkelRoot);
	void						KeepBVHJointsNames(FBModel* pSkeletonRoot)const;
	void						RemoveBVHReferenceNode(FBModel* pSkeletonRoot)const;
	HdlFBPlugTemplate<FBModel>	GetBVHRoot()const;
	FBString					GetInputDir()const;
	FBString					GetOutputDir()const;

private:
	mbt::FBGridLayout	mLayout;

	FBLabel				mInputDirLabel;
	FBEdit				mInputDirEdit;
	FBButton			mInputDirPopupButton;

	FBLabel				mOutputDirLabel;
	FBEdit				mOutputDirEdit;
	FBButton			mOutputDirPopupButton;

	FBLabel				mRemoveBVHRefNodeLabel;
	FBButton			mRemoveBVHRefNodeButton;

	FBLabel				mForceFPSLabel;
	FBEditNumber		mForceFPSEditNumber;
	FBButton			mForceFPSButton;
	
	FBButton			mProcessButton;
};

#endif // __BVH_TO_FBX_TOOL_H__
