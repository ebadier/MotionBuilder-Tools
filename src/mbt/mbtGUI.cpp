#include <sstream>

#include "mbtGUI.h"

namespace mbt
{
	//---------------------------------------------------------------------------------------//
	//---------------------------------------------------------------------------------------//
	//-------------------------class FBBoxLayout::ControlDesc--------------------------------//
	//---------------------------------------------------------------------------------------//
	//---------------------------------------------------------------------------------------//
	FBBoxLayout::ControlDesc::ControlDesc( const FBString& pRegionName, FBVisualComponent& pVisualComponent, int pSize, double pRatio, const ParamListInt& pCustomparams )
	{
		mRegionName = pRegionName;
		mVisualComponent = &pVisualComponent;
		mSize = pSize;
		mRatio = pRatio;
		mCustomparams = pCustomparams;
	}

	//---------------------------------------------------------------------------------------//
	FBBoxLayout::ControlDesc::~ControlDesc()
	{
		mVisualComponent = NULL;
		mCustomparams.clear();
	}

	//---------------------------------------------------------------------------------------//
	int FBBoxLayout::ControlDesc::GetSpace() const
	{
		return mCustomparams.at("space");
	}

	//---------------------------------------------------------------------------------------//
	void FBBoxLayout::ControlDesc::SetSpace( int pSpace )
	{
		mCustomparams["space"] = pSpace;
	}

	//---------------------------------------------------------------------------------------//
	//---------------------------------------------------------------------------------------//
	//----------------------------------class FBBoxLayout------------------------------------//
	//---------------------------------------------------------------------------------------//
	//---------------------------------------------------------------------------------------//
	FBClassImplementation( FBBoxLayout );
	
	//---------------------------------------------------------------------------------------//
	FBBoxLayout::FBBoxLayout( FBAttachType pOri ): FBLayout(NULL), mControls(), mDefaultSpace(4), mTotalRatio(0.0), mAllocatedsize(0), mFloworientation(pOri)
	{
		OnResize.Add(this, (FBCallback) &FBBoxLayout::_Resize);
	}

	//---------------------------------------------------------------------------------------//
	void FBBoxLayout::FBDestroy()
	{
		OnResize.Remove(this, (FBCallback) &FBBoxLayout::_Resize);
		
		for(int i=0; i < mControls.GetCount(); ++i)
		{
			delete mControls[i];
			mControls[i] = NULL;
		}
		mControls.Clear();

		FBLayout::FBDestroy();
	}

	//---------------------------------------------------------------------------------------//
	void FBBoxLayout::Add( FBVisualComponent& pVisualComponent, int pSize, ParamListInt& pCustomparams )
	{
		_Add(pVisualComponent, pSize, 0.0, pCustomparams);
	}

	//---------------------------------------------------------------------------------------//
	void FBBoxLayout::AddRelative( FBVisualComponent& pVisualComponent, double pRatio, ParamListInt& pCustomparams )
	{
		_Add(pVisualComponent, 0, pRatio, pCustomparams);
	}

	//---------------------------------------------------------------------------------------//
	void FBBoxLayout::Remove( const FBVisualComponent& pVisualComponent )
	{
		ControlDesc* desc = _GetDesc(pVisualComponent);
		if(desc)
		{
			// Remove and destroy the control and the region
			mTotalRatio -= desc->mRatio;
			mAllocatedsize -= (desc->GetSpace() + desc->mSize);
			ClearControl(desc->mRegionName);
			RemoveRegion(desc->mRegionName);
			mControls.Remove(desc);
			delete desc;
			desc = NULL;
			_Restructure();
		}
	}

	//---------------------------------------------------------------------------------------//
	void FBBoxLayout::RemoveAll()
	{
		for(int i=0; i < mControls.GetCount(); ++i)
		{
			// Remove and destroy the control and the region
			ClearControl(mControls[i]->mRegionName);
			RemoveRegion(mControls[i]->mRegionName);
			delete mControls[i];
			mControls[i] = NULL;
		}
		mTotalRatio = 0.0;
		mAllocatedsize = 0;
		mControls.Clear();
		_Restructure();
	}

	//---------------------------------------------------------------------------------------//
	void FBBoxLayout::_Resize( HISender pSender, HKEvent pEvent )
	{
		_Restructure();
	}

	//---------------------------------------------------------------------------------------//
	FBBoxLayout::ControlDesc* FBBoxLayout::_GetDesc( const FBVisualComponent& pVisualComponent )
	{
		for (int i=0; i < mControls.GetCount(); ++i)
		{
			if(mControls[i]->mVisualComponent == &pVisualComponent)
			{
				return mControls[i];
			}
		}
		return (ControlDesc*)NULL;
	}

	//---------------------------------------------------------------------------------------//
	void FBBoxLayout::_Add( FBVisualComponent& pVisualComponent, int pSize, double pRatio, ParamListInt& pCustomparams )
	{
		// create region for newly added control
		std::stringstream ss;
		ss << "region" << mControls.GetCount();
		FBString regionName(ss.str().c_str());

		int w, h;
		FBAttachType wAttachType, hAttachType;
		if(pCustomparams.HasParam("width"))
		{
			w = pCustomparams["width"];
			wAttachType = kFBAttachNone;
		}
		else
		{
			w = 0;
			wAttachType = kFBAttachRight;
		}

		if(pCustomparams.HasParam("height"))
		{
			h = pCustomparams["height"];
			hAttachType = kFBAttachNone;
		}
		else
		{
			h = 0;
			hAttachType = kFBAttachBottom;
		}

		AddRegion(regionName, regionName,
			0, kFBAttachLeft,"", 1.0,
			0, kFBAttachTop,"", 1.0,
			w, wAttachType, "", 1.0,
			h, hAttachType, "", 1.0);

		pVisualComponent.Name = regionName;
		SetControl(regionName, pVisualComponent);
		if(! pCustomparams.HasParam("space"))
		{
			pCustomparams["space"] = mDefaultSpace;
		}
		mTotalRatio += pRatio;
		mAllocatedsize += (pCustomparams["space"] + pSize);
		mControls.Add( new ControlDesc( regionName, pVisualComponent, pSize, pRatio, pCustomparams ) );
		_Restructure();
	}

	//---------------------------------------------------------------------------------------//
	void FBBoxLayout::_Restructure()
	{
		SetAutoRestructure(false);
		_ComputeRegion();
		// replace all our layout
		SetAutoRestructure(true);
		Restructure(false);
	}

	//---------------------------------------------------------------------------------------//
	//---------------------------------------------------------------------------------------//
	//----------------------------------class FBHBoxLayout-----------------------------------//
	//---------------------------------------------------------------------------------------//
	//---------------------------------------------------------------------------------------//
	FBClassImplementation( FBHBoxLayout );

	//---------------------------------------------------------------------------------------//
	FBHBoxLayout::FBHBoxLayout( FBAttachType pFloworientation /*= kFBAttachLeft*/ ): FBBoxLayout(pFloworientation)
	{

	}

	//---------------------------------------------------------------------------------------//
	void FBHBoxLayout::_ComputeRegion()
	{
		int spaceAvailable = Region.Position.X[1] - Region.Position.X[0] - mAllocatedsize;
		FBAttachType relativeAttachType = (mFloworientation == kFBAttachLeft) ? kFBAttachRight : kFBAttachLeft;
		FBAttachType attachType = mFloworientation;
		FBVisualComponent* anchor = NULL;

		for (int i=0; i < mControls.GetCount(); ++i)
		{
			ControlDesc* desc = mControls[i];

			int size = (desc->mSize > 0) ? desc->mSize : int( (desc->mRatio / mTotalRatio) * spaceAvailable);
			desc->mVisualComponent->Region.Width = size;
			desc->mVisualComponent->Region.AttachType.Width = kFBAttachNone;

			int offset = (mFloworientation == kFBAttachLeft) ? desc->GetSpace() : (-size - desc->GetSpace());
			desc->mVisualComponent->Region.X = offset;
			desc->mVisualComponent->Region.AttachType.X = attachType;

			if(anchor)
			{
				desc->mVisualComponent->Region.AttachTo.X = anchor;
			}

			attachType = relativeAttachType;
			anchor = desc->mVisualComponent;
		}
	}

	//---------------------------------------------------------------------------------------//
	//---------------------------------------------------------------------------------------//
	//----------------------------------class FBVBoxLayout-----------------------------------//
	//---------------------------------------------------------------------------------------//
	//---------------------------------------------------------------------------------------//
	FBClassImplementation( FBVBoxLayout );

	//---------------------------------------------------------------------------------------//
	FBVBoxLayout::FBVBoxLayout( FBAttachType pFloworientation /*= kFBAttachTop*/ ): FBBoxLayout(pFloworientation)
	{

	}

	//---------------------------------------------------------------------------------------//
	void FBVBoxLayout::_ComputeRegion()
	{
		int spaceAvailable = Region.Position.Y[1] - Region.Position.Y[0] - mAllocatedsize;
		FBAttachType relativeAttachType = (mFloworientation == kFBAttachTop) ? kFBAttachBottom : kFBAttachTop;
		FBAttachType attachType = mFloworientation;
		FBVisualComponent* anchor = NULL;

		for (int i=0; i < mControls.GetCount(); ++i)
		{
			ControlDesc* desc = mControls[i];

			int size = (desc->mSize > 0) ? desc->mSize : int( (desc->mRatio / mTotalRatio) * spaceAvailable);
			desc->mVisualComponent->Region.Height = size;
			desc->mVisualComponent->Region.AttachType.Height = kFBAttachNone;

			int offset = (mFloworientation == kFBAttachTop) ? desc->GetSpace() : (-size - desc->GetSpace());
			desc->mVisualComponent->Region.Y = offset;
			desc->mVisualComponent->Region.AttachType.Y = attachType;

			if(anchor)
			{
				desc->mVisualComponent->Region.AttachTo.Y = anchor;
			}

			attachType = relativeAttachType;
			anchor = desc->mVisualComponent;
		}
	}

	//---------------------------------------------------------------------------------------//
	//---------------------------------------------------------------------------------------//
	//----------------------------class FBGridLayout::ColDesc--------------------------------//
	//---------------------------------------------------------------------------------------//
	//---------------------------------------------------------------------------------------//
	FBGridLayout::ColDesc::ColDesc(): mWidth(0), mRatio(1.0), mSpacing(0), mX(0), mW(0)
	{

	}

	//---------------------------------------------------------------------------------------//
	int FBGridLayout::ColDesc::Right() const
	{
		return mX + mW;
	}

	//---------------------------------------------------------------------------------------//
	//---------------------------------------------------------------------------------------//
	//----------------------------class FBGridLayout::RowDesc--------------------------------//
	//---------------------------------------------------------------------------------------//
	//---------------------------------------------------------------------------------------//
	FBGridLayout::RowDesc::RowDesc(): mHeight(0), mRatio(1.0), mSpacing(0), mY(0), mH(0)
	{

	}

	//---------------------------------------------------------------------------------------//
	int FBGridLayout::RowDesc::Bottom() const
	{
		return mY + mH;
	}

	//---------------------------------------------------------------------------------------//
	//---------------------------------------------------------------------------------------//
	//------------------------class FBGridLayout::ControlDesc--------------------------------//
	//---------------------------------------------------------------------------------------//
	//---------------------------------------------------------------------------------------//
	FBGridLayout::ControlDesc::ControlDesc( FBVisualComponent& pVisualComponent, const FBString& pRegionName, int pR1, int pR2, int pC1, int pC2, FBAttachType pAttachX, FBAttachType pAttachY, int pW, int pH ):
	mVisualComponent(&pVisualComponent), mRegionName(pRegionName), mR1(pR1), mR2(pR2), mC1(pC1), mC2(pC2), mAttachX(pAttachX), mAttachY(pAttachY), mW(pW), mH(pH)
	{
		
	}

	//---------------------------------------------------------------------------------------//
	//---------------------------------------------------------------------------------------//
	//-------------------------------class FBGridLayout--------------------------------------//
	//---------------------------------------------------------------------------------------//
	//---------------------------------------------------------------------------------------//
	FBClassImplementation(FBGridLayout);

	//---------------------------------------------------------------------------------------//
	FBGridLayout::FBGridLayout( int pSpacing /*= 4*/ ): FBLayout(NULL), mDefaultspacing(pSpacing), mControls(), mRows(), mCols()
	{
		OnResize.Add( this, (FBCallback) &FBGridLayout::_Resize);
	}

	//---------------------------------------------------------------------------------------//
	void FBGridLayout::FBDestroy()
	{
		OnResize.Remove(this, (FBCallback) &FBGridLayout::_Resize);

		for(int i=0; i < mControls.GetCount(); ++i)
		{
			delete mControls[i];
			mControls[i] = NULL;
		}
		mControls.Clear();
		mRows.Clear();
		mCols.Clear();

		FBLayout::FBDestroy();
	}

	//---------------------------------------------------------------------------------------//
	int FBGridLayout::GetNbRows() const
	{
		return mRows.GetCount();
	}

	//---------------------------------------------------------------------------------------//
	int FBGridLayout::GetNbCols() const
	{
		return mCols.GetCount();
	}

	//---------------------------------------------------------------------------------------//
	void FBGridLayout::Add( FBVisualComponent& pVisualComponent, int pR, int pC, FBAttachType pAttachX /*= kFBAttachLeft*/, FBAttachType pAttachY /*= kFBAttachTop*/, int pWidth /*= 0*/, int pHeight /*= 0*/ )
	{
		_Add(pVisualComponent, pR, pR, pC, pC, pAttachX, pAttachY, pWidth, pHeight);
	}

	//---------------------------------------------------------------------------------------//
	void FBGridLayout::AddRange( FBVisualComponent& pVisualComponent, int pR1, int pR2, int pC1, int pC2, FBAttachType pAttachX /*= kFBAttachLeft*/, FBAttachType pAttachY /*= kFBAttachTop*/ )
	{
		_Add(pVisualComponent, pR1, pR2, pC1, pC2, pAttachX, pAttachY, 0, 0);
	}

	//---------------------------------------------------------------------------------------//
	void FBGridLayout::Remove( const FBVisualComponent& pVisualComponent )
	{
		ControlDesc* desc = _GetDesc(pVisualComponent);
		if(desc)
		{
			// Remove and destroy the control and the region
			ClearControl(desc->mRegionName);
			RemoveRegion(desc->mRegionName);
			mControls.Remove(desc);
			delete desc;
			desc = NULL;
			_Restructure();
		}
	}

	//---------------------------------------------------------------------------------------//
	void FBGridLayout::RemoveAll()
	{
		for(int i=0; i < mControls.GetCount(); ++i)
		{
			// Remove and destroy the control and the region
			ClearControl(mControls[i]->mRegionName);
			RemoveRegion(mControls[i]->mRegionName);
			delete mControls[i];
			mControls[i] = NULL;
		}
		mControls.Clear();
		mRows.Clear();
		mCols.Clear();
		_Restructure();
	}

	//---------------------------------------------------------------------------------------//
	void FBGridLayout::SetRowHeight( int pR, int pH )
	{
		_Updaterows(pR);
		mRows[pR].mHeight = pH;
		_Restructure();
	}

	//---------------------------------------------------------------------------------------//
	void FBGridLayout::SetRowRatio( int pR, double pRatio )
	{
		_Updaterows(pR);
		mRows[pR].mRatio = pRatio;
		_Restructure();
	}

	//---------------------------------------------------------------------------------------//
	void FBGridLayout::SetRowSpacing( int pR, int pSpacing )
	{
		_Updaterows(pR);
		mRows[pR].mSpacing = pSpacing;
		_Restructure();
	}

	//---------------------------------------------------------------------------------------//
	void FBGridLayout::SetColWidth( int pC, int pW )
	{
		_Updatecols(pC);
		mCols[pC].mWidth = pW;
		_Restructure();
	}

	//---------------------------------------------------------------------------------------//
	void FBGridLayout::SetColRatio( int pC, double pRatio )
	{
		_Updatecols(pC);
		mCols[pC].mRatio = pRatio;
		_Restructure();
	}

	//---------------------------------------------------------------------------------------//
	void FBGridLayout::SetColSpacing( int pC, int pSpacing )
	{
		_Updatecols(pC);
		mCols[pC].mSpacing = pSpacing;
		_Restructure();
	}

	//---------------------------------------------------------------------------------------//
	FBGridLayout::ControlDesc* FBGridLayout::_GetDesc( const FBVisualComponent& pVisualComponent )
	{
		for(int i=0; i < mControls.GetCount(); ++i)
		{
			ControlDesc* desc = mControls[i];
			if(desc->mVisualComponent == &pVisualComponent)
			{
				return desc;
			}
		}			
		return (ControlDesc*)NULL;
	}

	//---------------------------------------------------------------------------------------//
	void FBGridLayout::_Resize( HISender pSender, HKEvent pEvent )
	{
		_Restructure();
	}

	//---------------------------------------------------------------------------------------//
	void FBGridLayout::_Add( FBVisualComponent& pVisualComponent, int pR1, int pR2, int pC1, int pC2, FBAttachType pAttachX, FBAttachType pAttachY, int pWidth, int pHeight )
	{
		_Updaterows(pR2);
		_Updatecols(pC2);

		// create region for newly added control        
		std::stringstream ss;
		ss << "region" << pR1 << pR2 << pC1 << pC2;
		FBString regionName(ss.str().c_str());
		pVisualComponent.Name = regionName;

		AddRegion(regionName, regionName,
			0, pAttachX, "", 1.0,
			0, pAttachY, "", 1.0,
			0, kFBAttachRight, "", 1.0,
			0, kFBAttachBottom, "", 1.0);

		SetControl(regionName, pVisualComponent);
		
		mControls.Add(new ControlDesc(pVisualComponent, regionName, pR1, pR2, pC1, pC2, pAttachX, pAttachY, pWidth, pHeight));

		_Restructure();
	}

	//---------------------------------------------------------------------------------------//
	void FBGridLayout::_Updaterows( int pR )
	{
		while(mRows.GetCount() <= pR)
		{
			mRows.Add(RowDesc());
		}
	}

	//---------------------------------------------------------------------------------------//
	void FBGridLayout::_Updatecols( int pC )
	{
		while(mCols.GetCount() <= pC)
		{
			mCols.Add(ColDesc());
		}
	}

	//---------------------------------------------------------------------------------------//
	int FBGridLayout::_GetSpace( const RowDesc& pRow ) const
	{
		return (pRow.mSpacing == 0) ? mDefaultspacing : pRow.mSpacing;
	}

	//---------------------------------------------------------------------------------------//
	int FBGridLayout::_GetSpace( const ColDesc& pCol ) const
	{
		return (pCol.mSpacing == 0) ? mDefaultspacing : pCol.mSpacing;
	}

	//---------------------------------------------------------------------------------------//
	void FBGridLayout::_Restructure()
	{
		SetAutoRestructure(false);
		_ComputeRegion();
		// replace all our layout
		SetAutoRestructure(true);
		Restructure(false);
	}

	//---------------------------------------------------------------------------------------//
	void FBGridLayout::_ComputeRegion()
	{
		int width = Region.Position.X[1] - Region.Position.X[0];
		int height = Region.Position.Y[1] - Region.Position.Y[0];

		// check if in our controls, we need to update our row/col fixed size
		for(int i=0; i < mControls.GetCount(); ++i)
		{
			ControlDesc* desc = mControls[i];
			if( desc->mH && (mRows[desc->mR1].mHeight < desc->mH) )
			{
				mRows[desc->mR1].mHeight = desc->mH;
			}

			if( desc->mW && (mCols[desc->mC1].mWidth < desc->mW) )
			{
				mCols[desc->mC1].mWidth = desc->mW;
			}
		}

		// compute the size of each col/rows        
		int wfixed = 0;
		int	wspace = 0;
		double wratio = 0.0;
		for (int i=0; i < mCols.GetCount(); ++i)
		{
			const ColDesc& col = mCols[i];
			wspace += _GetSpace(col);
			if(col.mWidth)
			{
				wfixed += col.mWidth;
			}
			else
			{
				wratio += col.mRatio;
			}
		}

		int wavailable = width - wspace - wfixed;
		int x = 0;
		for (int i=0; i < mCols.GetCount(); ++i)
		{
			ColDesc& col = mCols[i];
			col.mX = x + _GetSpace(col);
			if(col.mWidth)
			{
				col.mW = col.mWidth;
			}
			else
			{
				col.mW = int((col.mRatio / wratio) * wavailable);
			}
			x = col.Right();
		}

		int hfixed = 0;
		int	hspace = 0;
		double hratio = 0.0;
		for (int i=0; i < mRows.GetCount(); ++i)
		{
			const RowDesc& row = mRows[i];
			hspace += _GetSpace(row);
			if(row.mHeight)
			{
				hfixed += row.mHeight;
			}
			else
			{
				hratio += row.mRatio;
			}
		}

		int havailable = height - hspace - hfixed;
		int y = 0;
		for (int i=0; i < mRows.GetCount(); ++i)
		{
			RowDesc& row = mRows[i];
			row.mY = y + _GetSpace(row);
			if(row.mHeight)
			{
				row.mH = row.mHeight;
			}
			else
			{
				row.mH = int((row.mRatio / hratio) * havailable);
			}
			y = row.Bottom();
		}

		// update all our control's region
		for (int i=0; i < mControls.GetCount(); ++i)
		{
			ControlDesc* desc = mControls[i];
			// compute width of control
			int woffset = 0;
			if(desc->mW)
			{
				woffset = desc->mW;
			}
			else
			{
				woffset = mCols[desc->mC2].Right() - mCols[desc->mC1].mX;
			}

			// compute x pos of control
			if(desc->mAttachX == kFBAttachLeft)
			{
				x = mCols[desc->mC1].mX;
			}
			else
			{
				x = mCols[desc->mC2].Right() - woffset - width;
			}

			desc->mVisualComponent->Region.X = x;
			desc->mVisualComponent->Region.AttachType.Width = kFBAttachNone;
			desc->mVisualComponent->Region.Width = woffset;

			// compute height of control
			int hoffset = 0;
			if(desc->mH)
			{
				hoffset = desc->mH;
			}
			else
			{
				hoffset = mRows[desc->mR2].Bottom() - mRows[desc->mR1].mY;
			}

			// compute y pos of control
			if(desc->mAttachY == kFBAttachTop)
			{
				y = mRows[desc->mR1].mY;
			}
			else
			{
				y = mRows[desc->mR2].Bottom() - hoffset - height;
			}

			desc->mVisualComponent->Region.Y = y;
			desc->mVisualComponent->Region.AttachType.Height = kFBAttachNone;
			desc->mVisualComponent->Region.Height = hoffset;
		}
	}

	//---------------------------------------------------------------------------------------//
}