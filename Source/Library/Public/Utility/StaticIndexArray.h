// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <set>


template<typename Type>
struct TDefaultValidator
{
	static bool IsValid(const Type& Value)
	{
		return Value.IsValid();
	}
	static void Invalidate(Type& Value)
	{
		Value.Invalidate();
	}
};

template<typename T, typename Validator = TDefaultValidator<ElementType>> class TStaticIndexArray;

/**
 * Array that keeps its indexes constant during operations
 * Should be packed at the end of operations due to inefficient memory size
 * Removal operation is O(log n). Others are as in usual array
 * Memory footprint is horrendous
 * 
 * Default validation class requires 2 functions to be implemented in type
 *	bool IsValid() const;
 *	void Invalidate();
 * 
 * User validation class must implement 2 functions:
 *	static bool IsValid(const ElementType& Element); 
 *	static void Invalidate(ElementType& Element);
 */
template<typename ElementType, typename ValidatorType>
class TStaticIndexArray
{

public:
	//typename InElementType ElementType;
	//typename InValidatorType ValidatorType;

	TStaticIndexArray() {}

	/** Add to first free spot or to the end */
	int32 Add(const ElementType& Element)
	{
		int32 AddIndex;
		if (NumHoles() > 0)
		{			
			int32 FreeIndex = *FreeIndices.begin();
			FreeIndices.erase(FreeIndices.begin());
			
			Items[FreeIndex] = Element;

			AddIndex = FreeIndex;
		}
		else
		{
			AddIndex = Items.Add(Element);
		}

		return AddIndex;
	}

	/** Add to first free spot or to the end */
	int32 Add(ElementType&& Element)
	{
		return Add(MoveTempIfPossible(Element));
	}

	/** Add to first free spot or to the end */
	ElementType& Add_GetRef(ElementType&& Element)
	{
		return Add_GetRef(MoveTempIfPossible(Element));
	}

	/** Add to first free spot or to the end */
	ElementType& Add_GetRef(const ElementType& Element)
	{
		return Items[Add(Element)];
	}

	/** Returns true if index and element were valid */
	bool RemoveAt(int32 Index)
	{
		if (IsValidIndex(Index) && !IsEmptyAt(Index))
		{
			FreeIndices.emplace(Index);
			ValidatorType::Invalidate(Items[Index]);
			return true;
		}

		return false;
	}

	/** Check validity of element under Index. 
	 * Does not check validity of the Index 
	 */
	bool IsEmptyAt(int32 Index) const
	{
		return !ValidatorType::IsValid(Items[Index]);
	}

	/** Does not check validity of element under index
	 * @return	true if Index is in range of container index
	 */
	bool IsValidIndex(int32 Index) const
	{
		return Items.IsValidIndex(Index);
	}

	/**
	 * Check validity of index and element
	 */
	bool IsElementAt(int32 Index) const
	{
		return IsValidIndex(Index) && !IsEmptyAt(Index);
	}

	/**
	 * Check validity of index and invalidity element
	 */
	bool IsHoleAt(int32 Index) const
	{
		return IsValidIndex(Index) && IsEmptyAt(Index);
	}

	/** Total number of positions including free */
	int32 NumTotal() const
	{
		return Items.Num();
	}
	/** Number of free positions in container */
	int32 NumHoles() const
	{
		return FreeIndices.size();
	}
	/** Number of occupied positions */
	int32 NumOccupied() const
	{
		return NumTotal() - NumHoles();
	}
	
	/** Validated set to prevent unchecked invalidation 
	 *  Can be used to set value in container without leaving unchecked holes
	 */
	void SetValueAt(int32 Index, ElementType& NewValue)
	{		
		if (!ValidatorType::IsValid(NewValue))
		{
			RemoveAt(Index);
		}
		else if(IsValidIndex(Index))
		{
			Items[Index] = NewValue;
		}
	}

	const ElementType& operator[](int32 Index) const
	{
		return Items[Index];
	}

	/** 
	 * Unsafe if value will be invalidated by reference
	 * Use SetValueAt if possible or use ValidateAt after finishing operations on the reference
	 */
	ElementType& GetRef(int32 Index)
	{
		return Items[Index];
	}

	/** Validates element at Index and updated free spots set if needed */
	bool ValidateAt(int32 Index)
	{
		if (!IsValidIndex(Index)) return false;

		if (IsEmptyAt(Index))
		{
			if (FreeIndices.find(Index) == FreeIndices.end())
			{
				FreeIndices.emplace(Index);
			}
			return false;
		}

		return true;		
	}

	/** Pack array and remove holes in it
	 * @param	OutIndexRemap	Index change map<OldIndex,NewIndex>  
	 * @return	NewSize
	 */
	int32 Pack(TMap<int32, int32>& OutIndexRemap)
	{
		OutIndexRemap.Reset();

		TArray<ElementType> PackedArray;
		PackedArray.Reserve(Items.Num());
		for (int OldIndex = 0; OldIndex < Items.Num(); OldIndex++)		
		{			
			if (ValidatorType::IsValid(Items[OldIndex]))
			{
				int32 NewIndex = PackedArray.Add(MoveTemp(Items[OldIndex]));
				OutIndexRemap.Add(OldIndex, NewIndex);
			}
		}
		PackedArray.Shrink();

		Items = PackedArray;
		return Items.Num();
	}

	/** Return array as is */
	TArray<ElementType> Array() const
	{
		return Items;
	}


	/** Cannot shrink array 
	 * Use Pack function to shrink array
	 */
	void Reserve(int32 Number)
	{
		if (Number >= Items.Num())
		{
			Items.Reserve(Number);
		}
	}

	/**
	 * Empties the array. It calls the destructors on held items if needed.
	 *
	 * @param Slack (Optional) The expected usage size after empty operation. Default is 0.
	 */
	void Empty(int32 Slack = 0)
	{
		Items.Empty(Slack);
		FreeIndices.clear();
	}
private:
	TArray<ElementType> Items;

	//Hate using STL in unreal context, but I have no choice here
	// FreeIndices must be sorted set
	std::set<int32> FreeIndices;
};

