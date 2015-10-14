#ifndef __MBT_GUITOOLS_H__
#define __MBT_GUITOOLS_H__

#include "mbtParamList.h"
#include "fbsdk/fbsdk.h"

namespace mbt
{
	/**
	*	\brief Base class for a line layout (either vertical or horizontal)
	*
	*	This class is made to ease the creation of Tool in Python. It manages
	*	all the 'FBLayout' region stuff (no need to use FBAddRegionParam anymore!).
	*
	*	Generally you just Add a Control to the layout specifying some parameters.
	*
	*	There are 2 kinds of Add: Add with fixed size and AddRelative which ensure
	*	the control Added will occupy a 'percentage' of the available space AFTER the 
	*	fixed space has been assigned.
	*/
	class FBBoxLayout : public FBLayout
	{
		FBClassDeclare(FBBoxLayout, FBLayout);

	protected:
		/**
		*	\brief GUI control infos about a VisualComponent.
		*/
		class ControlDesc
		{
		public:
			FBString			mRegionName; //!< name of the region.
			int					mSize; //!< size of the VisualComponent.
			FBVisualComponent*	mVisualComponent; //!< the VisualComponent (HANDLE).
			double				mRatio; //!< ratio of the VisualComponent.
			ParamListInt		mCustomparams; //!< space / width / height.

		public:
			ControlDesc(const FBString& pRegionName, FBVisualComponent& pVisualComponent, int pSize, double pRatio, const ParamListInt& pCustomparams);
			virtual ~ControlDesc();

			int GetSpace()const;
			void SetSpace(int pSpace);
		};

	protected:
		FBArrayTemplate<ControlDesc*>	mControls; //!< List of infos for each VisualComponent (AGGREGATON).
		int								mDefaultSpace; //!< Default space between VisualComponents.
		double							mTotalRatio; //!< Total ratio required by the sum of VisualComponents.
		int								mAllocatedsize; //!< total size required by the sum of VisualComponents and the space between them.
		FBAttachType					mFloworientation; //!< the way the VisualComponents will be inserted.

	public:
		/**
		*	\brief Constructor
		*/
		FBBoxLayout(FBAttachType pOri);
		/**
		*	\brief Destructor
		*/
		virtual void FBDestroy();

		/**
		*	\brief Add a control to layout specifying its FIXED size.
		*
		*	\param : pCustomparams
		*	space: space between previous control
		*	width: fixed width if layout is vertical based
		*	height: fixed height if layout is horizontal based.
		*
		*	\warning : calling this method set pVisualComponent.Name with the auto-created Region's name.
		*/
		void Add(FBVisualComponent& pVisualComponent, int pSize, ParamListInt& pCustomparams);

		/**
		*	\brief Add a control to layout specifying its RATIO. This means
		*	the control will be assigned a size based on the space left when all FIXED
		*	size have been allocated.
		*
		*	pCustomparams:
		*	space: space between prev control
		*	width: fixed width if layout is vertical based
		*	height: fixed height if layout is horizontal based.
		*
		*	\warning : calling this method set pVisualComponent.Name with the auto-created Region's name.
		*/
		void AddRelative(FBVisualComponent& pVisualComponent, double pRatio, ParamListInt& pCustomparams);

		/**
		*	\brief Remove a control from the layout.
		*/
		void Remove(const FBVisualComponent& pVisualComponent);
	
		/**
		*	\brief Remove all controls from layout.
		*/
		void RemoveAll();

	protected:
		virtual void _ComputeRegion() = 0;
		//{raise RuntimeError("Must be reimplemented!" );}

	private:
		void _Restructure();
		void _Resize(HISender pSender, HKEvent pEvent );
		ControlDesc* _GetDesc(const FBVisualComponent& pVisualComponent);
		void _Add(FBVisualComponent& pVisualComponent, int pSize, double pRatio, ParamListInt& pCustomparams);
	};

	/**
	*	\brief This class manages a FBBoxLayout Horizontal (see FBBoxLayout for documentation on how to Add/Remove control).
	*	Add method specify the fixed width of a control.
	*/
	class FBHBoxLayout : public FBBoxLayout
	{
		FBClassDeclare(FBHBoxLayout, FBBoxLayout);
	public:
		/**
		*	\brief Constructor
		*	\param pFloworientation: 
		*	FBAttachType.kFBAttachLeft : all controls added from left to right ->
		*	FBAttachType.kFBAttachRight: all controls added from right to left <-
		*/
		FBHBoxLayout(FBAttachType pFloworientation = kFBAttachLeft);
		
	protected:
		virtual void _ComputeRegion();
	};

	/**
	*	\brief This class manages a FBBoxLayout Vertical (see FBBoxLayout for documentation on how to Add/Remove control).
	*	Add method specify the fixed height of a control.
	*/
	class FBVBoxLayout : public FBBoxLayout
	{
		FBClassDeclare(FBVBoxLayout, FBBoxLayout);
	public:
		/**
		*	\brief Constructor
		*	\param pFloworientation: 
		*	FBAttachType.kFBAttachTop : all controls added from top to bottom 
		*	FBAttachType.kFBAttachBottom: all controls added from bottom to top
		*/
		FBVBoxLayout(FBAttachType pFloworientation = kFBAttachTop);

	protected:
		virtual void _ComputeRegion();
	};

	/**
	*	\brief More advance layout that allow organization of control in a grid. 
	*	User can place a control at specific coordinates. 
	*	User can also setup parameters that affect whole row or column.
	*/
	class FBGridLayout : public FBLayout
	{
		FBClassDeclare(FBGridLayout, FBLayout);
	
	private:
		/**
		*	\brief GUI infos about a column.
		*/
		class ColDesc
		{
		public:
			int		mWidth;
			double	mRatio;
			int		mSpacing;
			int		mX;
			int		mW;
		public:
			ColDesc();
			int Right()const;
		};

		/**
		*	\brief GUI infos about a row.
		*/
		class RowDesc
		{
		public:
			int		mHeight;
			double	mRatio;
			int		mSpacing;
			int		mY;
			int		mH;
		public:
			RowDesc();
			int Bottom()const;
		};

		/**
		*	\brief GUI control infos about a VisualComponent.
		*/
		class ControlDesc
		{
		public:
			FBVisualComponent*	mVisualComponent;
			FBString			mRegionName;
			int					mR1;
			int					mR2;
			int					mC1;
			int					mC2;
			FBAttachType		mAttachX;
			FBAttachType		mAttachY;
			int					mW;
			int					mH;

		public:
			ControlDesc(FBVisualComponent& pVisualComponent, const FBString& pRegionName, int pR1, int pR2, int pC1, int pC2, FBAttachType pAttachX, FBAttachType pAttachY, int pW, int pH);
		};

	private:
		int								mDefaultspacing; //!< Default space between VisualComponents.
		FBArrayTemplate<ControlDesc*>	mControls; //!< List of infos for each VisualComponent (AGGREGATON).
		FBArrayTemplate<RowDesc>		mRows; //!< List of infos for each row.
		FBArrayTemplate<ColDesc>		mCols; //!< List of infos for each column.

	public:
		/**
		*	\brief Constructor.
		*/
		FBGridLayout(int pSpacing = 4);

		/**
		*	\brief Destructor
		*/
		virtual void FBDestroy();

		/**
		*	\brief Get the number of rows.
		*/
		int GetNbRows()const;

		/**
		*	\brief Get the number of columns.
		*/
		int GetNbCols()const;
	
		/**
		*	\brief Add control in row r and column c.
		*	\param pAttachX: specifies a control horizontal attachment in a column (kFBAttachLeft, kFBAttachRight)
		*	\param pAttachY: specifies a control vertical attachment in a row (kFBAttachTop, kFBAttachBottom)
		*	\param pWidth: fixed width of a control
		*	\param pHeight: fixed height of a control
		*/
		void Add(FBVisualComponent& pVisualComponent, int pR, int pC, FBAttachType pAttachX = kFBAttachLeft, FBAttachType pAttachY = kFBAttachTop, int pWidth = 0, int pHeight = 0);

		/**
		*	\brief Add control in a range of coordinates. Control will span from row1 to row2 and from col1 to col2.
		*	\param pAttachX: specifies a control horizontal attachment in a column (kFBAttachLeft, kFBAttachRight)
		*	\param pAttachY: specifies a control vertical attachment in a row (kFBAttachTop, kFBAttachBottom)
		*/
		void AddRange(FBVisualComponent& pVisualComponent, int pR1, int pR2, int pC1, int pC2, FBAttachType pAttachX = kFBAttachLeft, FBAttachType pAttachY = kFBAttachTop);

		/**
		*	\brief Remove a control from the grid.
		*/ 
		void Remove(const FBVisualComponent& pVisualComponent);
	
		/**
		*	\brief Remove all controls from the grid.
		*/
		void RemoveAll();
		
		/**
		*	\brief Set row r fixed height.
		*/
		void SetRowHeight(int pR, int pH);

		/**
		*	\brief Set row r ratio. this row will be allocated height according to this ratio,
		*	when all fixed heights have been allocated.
		*/
		void SetRowRatio(int pR, double pRatio);

		/**
		*	\brief Set row r spacing between previous row.
		*/
		void SetRowSpacing(int pR, int pSpacing);

		/**
		*	\brief Set col c fixed width.
		*/
		void SetColWidth(int pC, int pW);

		/**
		*	\brief Set col c ratio. this col will be allocated width according to this ratio,
		*	when all fixed width have been allocated.
		*/
		void SetColRatio(int pC, double pRatio);

		/**
		*	\brief Set col c spacing between previous col.
		*/
		void SetColSpacing(int pC, int pSpacing);

	private:
		ControlDesc* _GetDesc(const FBVisualComponent& pVisualComponent);
		void _Resize(HISender pSender, HKEvent pEvent );
		void _Add(FBVisualComponent& pVisualComponent, int pR1, int pR2, int pC1, int pC2, FBAttachType pAttachX, FBAttachType pAttachY, int pWidth, int pHeight);
		void _Updaterows(int pR);
		void _Updatecols(int pC);
		int _GetSpace(const RowDesc& pRow)const;
		int _GetSpace(const ColDesc& pCol)const;
		void _Restructure();
		void _ComputeRegion();
	};
}

#endif // __MBT_GUITOOLS_H__