#include "fbxsdk\core\base\fbxfolder.h"

#include "mbt/mbtMacros.h"
#include "mbt/mbtUtil.h"

#include "BvhToFbxTool.h"

//--- FiLMBOX implementation and registration
FBToolImplementation(	BvhToFbxTool	);
FBRegisterTool		(	BvhToFbxTool,
						"Bvh to FBX converter",	
						"Convert bvh files to fbx files",
						FB_DEFAULT_SDK_ICON	);	// Icon filename (default=Open Reality icon)

#define LAYOUT_NAME "Layout"
#define UI_DEFAULT_HEIGHT 25
#define FRAMETIME_ERROR -1.0

//---------------------------------------------------------------------------------------//
bool BvhToFbxTool::FBCreate()
{
	// UI Create & Configure
	UICreate	();
	UIConfigure	();
	UIConnect	();

	return true;
}

//---------------------------------------------------------------------------------------//
void BvhToFbxTool::FBDestroy()
{
	UIDisconnect();
}

//---------------------------------------------------------------------------------------//
FBString BvhToFbxTool::GetInputDir()const
{
	return FBString(mInputDirEdit.Text, "/");
}

//---------------------------------------------------------------------------------------//
FBString BvhToFbxTool::GetOutputDir()const
{
	return FBString(mOutputDirEdit.Text, "/");
}

//---------------------------------------------------------------------------------------//
void BvhToFbxTool::GetBVHFiles(FBStringList& pBVHFiles)
{
	//MBT_Out("Selected folder : " << pInputFolder);
	fbxsdk::FbxFolder folder;
	if(folder.Open(GetInputDir()))
	{
		while(folder.IsOpen() && folder.Next())
		{
			//MBT_Out("Entry : " << folder.GetEntryName());
			//MBT_Out("Entry type : " << folder.GetEntryType());
			//MBT_Out("Entry extension : " << folder.GetEntryExtension());

			if( folder.GetEntryType() == fbxsdk::FbxFolder::eRegularEntry )
			{
				if( (strcmp(folder.GetEntryExtension(), "bvh") == 0) || (strcmp(folder.GetEntryExtension(), "BVH") == 0) )
				{
					//MBT_Out("BVH File : " << folder.GetEntryName());
					pBVHFiles.Add(folder.GetEntryName());
				}
			}
		}
		folder.Close();
	}
	else
	{
		MBT_Popup("Input Directory is not valid !");
	}
}

//---------------------------------------------------------------------------------------//
double BvhToFbxTool::GetBVHFrameTime(const FBString& pBvhFilename)const
{
	double frameTime = FRAMETIME_ERROR;

	FILE* file = NULL;
	if ( fopen_s(&file, pBvhFilename, "r") == 0 )
	{
		// Get Frame Time line in the bvh file.
		char* pos = NULL; 
		char buffer[4096];
		do
		{
			fgets(buffer, 4096, file);
			pos = strstr(buffer, "Frame Time");
		}
		while( (!feof(file)) && (pos == NULL) );

		// Get the Frame Time value.
		if(pos != NULL)
		{
			const char delims[4] = " :\t";
			char* context = NULL;
			char* token = strtok_s( buffer, delims, &context);
			char* lastToken = NULL;
			do 
			{
				//MBT_Out(token);
				lastToken = token;
				token = strtok_s( NULL, delims, &context);
			} while (token != NULL);
			
			frameTime = atof( lastToken );
			if(frameTime == 0.0)
			{
				frameTime = FRAMETIME_ERROR;
			}
			//MBT_Out("Frame Time found : " << frameTime);
		}
		fclose (file);
	}
	return frameTime;
}

//---------------------------------------------------------------------------------------//
bool BvhToFbxTool::ForceFPS()const
{
	return mForceFPSEditNumber.Enabled;
}

//---------------------------------------------------------------------------------------//
bool BvhToFbxTool::SetFPS(const FBString& pBvhFilename)const
{
	double fps;
	if(ForceFPS())
	{
		fps = mForceFPSEditNumber.Value;
	}
	else
	{
		double frameTime = GetBVHFrameTime(pBvhFilename);
		if(frameTime == FRAMETIME_ERROR)
		{
			MBT_Popup("Unable to read Frame Time in : " << pBvhFilename << " !");
			return false;
		}
		fps = 1.0 / frameTime;
	}
	// Set the desired FPS here, so the import FPS should be the same.
	FBPlayerControl::TheOne().SetTransportFps(kFBTimeModeCustom, fps);
	return true;
}

//---------------------------------------------------------------------------------------//
void BvhToFbxTool::ResampleAnimation(FBModel* pSkelRoot)
{
	FBPlayerControl& player = FBPlayerControl::TheOne();

	FBPlotOptions options;
	options.mPlotOnFrame = true;
	options.mPlotAllTakes = false;
	double fps = player.GetTransportFpsValue(player.GetTransportFps());
	options.mPlotPeriod.SetSecondDouble(1.0/fps);
	options.mRotationFilterToApply = kFBRotationFilterUnroll;
	options.mUseConstantKeyReducer = true;
	options.mConstantKeyReducerKeepOneKey = false;

	mbt::PlotAnimation(&options, pSkelRoot, true);
}

//---------------------------------------------------------------------------------------//
void BvhToFbxTool::KeepBVHJointsNames(FBModel* pSkeletonRoot)const
{
	// remove any Motion Builder auto-added prefix before joint names.
	const FBString rootFullName = pSkeletonRoot->LongName;
	int prefixEndPos = rootFullName.ReverseFind(':');
	if(prefixEndPos != -1)
	{
		//MBT_Out("Root full name : " << rootFullName);
		//const FBString prefix = rootFullName.Left(prefixEndPos+1);
		//MBT_Out("Prefix : " << prefix);

		FBModelList joints;
		mbt::GetChildren(joints, pSkeletonRoot, true);
		joints.Add(pSkeletonRoot);
		for (int j = 0; j < joints.GetCount(); ++j)
		{
			FBModel* joint = joints[j];
			const FBString jointName = joint->LongName;
			const FBString newJointName = jointName.Mid(prefixEndPos+1, jointName.GetLen());
			//MBT_Out("Joint " << jointName << ", new name : " << newJointName);
			joint->LongName = newJointName;
		}
	}
}

//---------------------------------------------------------------------------------------//
void BvhToFbxTool::RemoveBVHReferenceNode(FBModel* pSkeletonRoot)const
{
	HdlFBPlugTemplate<FBModel> sceneRoot = FBSystem::TheOne().Scene->RootModel; 
	HdlFBPlugTemplate<FBModel> bvhRefNode = pSkeletonRoot->Parent;
	if( bvhRefNode.Ok() && (bvhRefNode != sceneRoot) )
	{
		//MBT_Out("BVH Reference Node : " << bvhRefNode->Name);
		if(mbt::UnParent(pSkeletonRoot, bvhRefNode))
		{
			mbt::Parent(pSkeletonRoot, sceneRoot);
			bvhRefNode->FBDelete();
		}
	}
}

//---------------------------------------------------------------------------------------//
HdlFBPlugTemplate<FBModel> BvhToFbxTool::GetBVHRoot()const
{
	HdlFBPlugTemplate<FBModel> skelRoot = FBSystem::TheOne().Scene->RootModel->Children[0];
	if( skelRoot.Ok() && (strstr(skelRoot->LongName, "reference") != NULL) )
	{
		// MoBu can add a node called BVH:reference, so the real root should be the first child.
		skelRoot = skelRoot->Children[0];
	}
	return skelRoot;
}

//---------------------------------------------------------------------------------------//
void BvhToFbxTool::UICreate()
{
	StartSize[0] = 320;
	StartSize[1] = 150;
	MinSize[0] = 320;
	MinSize[1] = 150;

	// Regions
	AddRegion( LAYOUT_NAME, LAYOUT_NAME,
		0,		kFBAttachLeft,	"",	1.0,
		0,		kFBAttachTop,	"",	1.0,
		0,		kFBAttachRight,	"",	1.0,
		0,		kFBAttachBottom,"", 1.0 );

	SetControl(LAYOUT_NAME, mLayout);
}

//---------------------------------------------------------------------------------------//
void BvhToFbxTool::UIConfigure()
{
	FBSystem& sys = FBSystem::TheOne();
	FBString browseButtonIconPath = sys.MakeFullPath(FBString(sys.PathImages, "/controls/browse_button.png"));
	//MBT_Out(browseButtonIconPath);

	mLayout.AddRange(mInputDirLabel, 0, 0, 0, 0); 
	mInputDirLabel.Caption = "BVH Input Directory";
	mLayout.AddRange(mInputDirEdit, 0, 0, 1, 1); 
	mInputDirEdit.Text = "C:\\";
	mInputDirEdit.Hint = "Bvh Files' Input Directory";
	mLayout.AddRange(mInputDirPopupButton, 0, 0, 2, 2);
	mInputDirPopupButton.Style = kFBPushButton;
	mInputDirPopupButton.SetImageFileNames(browseButtonIconPath);
	mInputDirPopupButton.Hint = "Browse";

	mLayout.AddRange(mOutputDirLabel, 1, 1, 0, 0);
	mOutputDirLabel.Caption = "FBX Output Directory";
	mLayout.AddRange(mOutputDirEdit, 1, 1, 1, 1);
	mOutputDirEdit.Text = "C:\\";
	mOutputDirEdit.Hint = "FBX files' Output Directory";
	mLayout.AddRange(mOutputDirPopupButton, 1, 1, 2, 2);
	mOutputDirPopupButton.Style = kFBPushButton;
	mOutputDirPopupButton.SetImageFileNames(browseButtonIconPath);
	mOutputDirPopupButton.Hint = "Browse";
	mLayout.SetColWidth(2, 30);

	mLayout.AddRange(mRemoveBVHRefNodeLabel, 2, 2, 0, 1);
	mRemoveBVHRefNodeLabel.Caption = "Remove BVH Ref Node";
	mRemoveBVHRefNodeLabel.Hint = "Remove BVH Reference Node if present (Keep Root as the first joint) ?";
	mLayout.AddRange(mRemoveBVHRefNodeButton, 2, 2, 2, 2);
	mRemoveBVHRefNodeButton.Style = kFBCheckbox;
	mRemoveBVHRefNodeButton.State = 1; // Default

	mLayout.AddRange(mForceFPSLabel, 3, 3, 0, 0);
	mForceFPSLabel.Caption = "Force FPS";
	mForceFPSLabel.Hint = "Default use BVH files framerates";
	mLayout.AddRange(mForceFPSEditNumber, 3, 3, 1, 1);
	mForceFPSEditNumber.Value = 100.0;
	mForceFPSEditNumber.Min = 1.0;
	mForceFPSEditNumber.Max = 1000.0;
	mForceFPSEditNumber.Precision = 1.0;
	mForceFPSEditNumber.LargeStep = 10.0;
	mForceFPSEditNumber.SmallStep = 1.0;
	mForceFPSEditNumber.Enabled = false; // Default
	mLayout.AddRange(mForceFPSButton, 3, 3, 2, 2);
	mForceFPSButton.Style = kFBCheckbox;
	mForceFPSButton.State = 0; // Default

	mLayout.AddRange(mProcessButton, 4, 4, 0, 2);
	mProcessButton.Style = kFBPushButton;
	mProcessButton.Caption = "Process";

	for(int i=0; i < mLayout.GetNbRows(); ++i)
	{
		mLayout.SetRowHeight(i, UI_DEFAULT_HEIGHT);
	}
}

//---------------------------------------------------------------------------------------//
void BvhToFbxTool::UIConnect()
{
	mInputDirPopupButton.OnClick.Add(this, (FBCallback) &BvhToFbxTool::EventOnInputDirPopupButtonClick);
	mOutputDirPopupButton.OnClick.Add(this, (FBCallback) &BvhToFbxTool::EventOnOutputDirPopupButtonClick);
	mForceFPSButton.OnClick.Add(this, (FBCallback) &BvhToFbxTool::EventOnForceFPSButtonClick);
	mProcessButton.OnClick.Add(this, (FBCallback) &BvhToFbxTool::EventOnProcessButtonClick);
}

//---------------------------------------------------------------------------------------//
void BvhToFbxTool::UIDisconnect()
{
	mInputDirPopupButton.OnClick.Remove(this, (FBCallback) &BvhToFbxTool::EventOnInputDirPopupButtonClick);
	mOutputDirPopupButton.OnClick.Remove(this, (FBCallback) &BvhToFbxTool::EventOnOutputDirPopupButtonClick);
	mForceFPSButton.OnClick.Remove(this, (FBCallback) &BvhToFbxTool::EventOnForceFPSButtonClick);
	mProcessButton.OnClick.Remove(this, (FBCallback) &BvhToFbxTool::EventOnProcessButtonClick);
}

//---------------------------------------------------------------------------------------//
void BvhToFbxTool::EventOnInputDirPopupButtonClick( HISender pSender, HKEvent pEvent )
{
	FBFolderPopup folderPopup;
	folderPopup.Caption	= "Select Directory containing BVH Files";

	mInputDirEdit.Text = "";
	if(folderPopup.Execute())
	{
		mInputDirEdit.Text = folderPopup.Path;
	}
}

//---------------------------------------------------------------------------------------//
void BvhToFbxTool::EventOnOutputDirPopupButtonClick( HISender pSender, HKEvent pEvent )
{
	FBFolderPopup folderPopup;
	folderPopup.Caption	= "Select Directory to export FBX Files";
	
	mOutputDirEdit.Text = "";
	if(folderPopup.Execute())
	{
		fbxsdk::FbxFolder folder;
		if(folder.Open(folderPopup.Path))
		{
			folder.Close();
			mOutputDirEdit.Text = folderPopup.Path;
		}
		else
		{
			MBT_Popup("Directory is not valid !");
		}
	}
}

//---------------------------------------------------------------------------------------//
void BvhToFbxTool::EventOnForceFPSButtonClick( HISender pSender, HKEvent pEvent )
{
	mForceFPSEditNumber.Enabled = (mForceFPSButton.State == 1);
}

//---------------------------------------------------------------------------------------//
void BvhToFbxTool::EventOnProcessButtonClick( HISender pSender, HKEvent pEvent )
{
	MBT_Test_P(GetInputDir() == "", "Input Directory not set !",);
	FBStringList bvhFiles;
	GetBVHFiles(bvhFiles);
	MBT_Test_P(bvhFiles.GetCount() < 1, "No BVH files found in Input Directory !", );
	MBT_Test_P(GetOutputDir() == "", "Ouput Directory not set !",);

	const bool removeBVHRefNode = mRemoveBVHRefNodeButton.State == 1;

	FBApplication& app = FBApplication::TheOne();
	FBProgress progress;
	progress.Caption = "BVH To FBX";
	progress.ProgressBegin();
	const int nbFiles = bvhFiles.GetCount();
	char message[1024];
	for (int i = 0; i < nbFiles; ++i)
	{
		const FBString& bvhFile = bvhFiles[i];
		const FBString bvhFilePath = FBSystem::TheOne().MakeFullPath(FBString(GetInputDir(), bvhFile));
		sprintf_s(message, "Processing file (%d / %d) : %s", i+1, nbFiles, bvhFilePath);
		MBT_Out(message);
		progress.Text = message;
		if(app.FileNew())
		{
			if(SetFPS(bvhFilePath))
			{
				if(app.FileImport(bvhFilePath))
				{
					HdlFBPlugTemplate<FBModel> skelRoot = GetBVHRoot();
					if(skelRoot.Ok())
					{
						// remove BVH reference Node if needed.
						if(removeBVHRefNode)
						{
							RemoveBVHReferenceNode(skelRoot);
						}

						// rename all joints as original Bvh names
						KeepBVHJointsNames(skelRoot);

						// if force FPS, animation should be resampled
						if(ForceFPS())
						{
							ResampleAnimation(skelRoot);
						}

						FBString fbxFile(bvhFile.Left(bvhFile.ReverseFind('.')), ".fbx");
						fbxFile = FBSystem::TheOne().MakeFullPath(FBString(GetOutputDir(), fbxFile));
						//if(!app.FileExport(fbxFile)) // only export at 30 FPS.
						if(!app.FileSave(fbxFile)) // This one export at the right FPS.
						{
							MBT_Popup("Cannot export file : " << fbxFile << " !");
							return;
						}
					}
					else
					{
						MBT_Popup("BVH Root not found !");
						return;
					}
				}
				else
				{
					MBT_Popup("Cannot import file : " << bvhFile << " !");
					return;
				}
			}
		}
		else
		{
			MBT_Popup("FileNew failed !");
		}
		progress.Percent = (int) ((float)(i+1) / float(nbFiles) * 100.0f);
	}
	progress.ProgressDone();
}

//---------------------------------------------------------------------------------------//