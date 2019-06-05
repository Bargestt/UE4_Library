// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EdgeFinder.h"


struct FPointsOctree
{
	typedef int32 ElementType;

	struct ElementContainer
	{
		FVector Location;
		ElementType Data;

		bool operator ==(const ElementContainer& Other) const { return Other.Data == this->Data; }
	};

	struct FTreeNode
	{
		TArray<ElementContainer> Data;


		FVector Center;
		float Bounds;

		TArray<FTreeNode> Children;

		FTreeNode(const FVector& Center, float Bounds) : Center(Center), Bounds(Bounds) {}

		void DrawDebug(const UWorld* World, const FTransform& Transform, bool IncludeChildren = false) const;

		bool IsLeaf() const { return Children.Num() <= 0; }

		bool IsInside(const FVector& Pos) const {
			return FMath::Abs(Pos.X - Center.X) < Bounds && FMath::Abs(Pos.Y - Center.Y) < Bounds && FMath::Abs(Pos.Z - Center.Z) < Bounds;
		}

		bool CanAddMore() const { return Data.Num() < MaxElementsPerNode; }

		TArray<ElementType> GetData() const {
			TArray<ElementType> Ret;
			for (const ElementContainer& d : Data)
			{
				Ret.Add(d.Data);
			}
			return Ret;
		}


		void AddElementToLocation(const ElementContainer& Element)
		{
			if (IsInside(Element.Location))
			{
				if (IsLeaf() && CanAddMore())
				{
					Data.AddUnique(Element);
				}
				else
				{
					if (Children.Num() == 0)
					{
						float NewBounds = Bounds / 2;

						Children.Add(FTreeNode(Center + NewBounds * FVector(1, 1, -1), NewBounds));
						Children.Add(FTreeNode(Center + NewBounds * FVector(-1, -1, -1), NewBounds));
						Children.Add(FTreeNode(Center + NewBounds * FVector(-1, 1, -1), NewBounds));
						Children.Add(FTreeNode(Center + NewBounds * FVector(1, -1, -1), NewBounds));

						Children.Add(FTreeNode(Center + NewBounds * FVector(1, 1, 1), NewBounds));
						Children.Add(FTreeNode(Center + NewBounds * FVector(-1, -1, 1), NewBounds));
						Children.Add(FTreeNode(Center + NewBounds * FVector(-1, 1, 1), NewBounds));
						Children.Add(FTreeNode(Center + NewBounds * FVector(1, -1, 1), NewBounds));

						for (const ElementContainer& d : Data)
						{
							for (FTreeNode& Node : Children)
							{
								Node.AddElementToLocation(d);
							}
						}
						Data.Empty();
					}

					for (FTreeNode& Node : Children)
					{
						Node.AddElementToLocation(Element);
					}
				}
			}
		}

		TArray<ElementType> GetElementsByLocation(const FVector& Location)
		{
			if (IsInside(Location))
			{
				if (IsLeaf())
				{
					return GetData();
				}
				else
				{
					TArray<ElementType> FoundNodes;
					for (FTreeNode& Node : Children)
					{
						FoundNodes.Append(Node.GetElementsByLocation(Location));
					}
					return FoundNodes;
				}
			}

			return TArray<ElementType>();
		}
	};

	static int8 MaxElementsPerNode;

public:
	FTreeNode RootNode;

	FPointsOctree() : FPointsOctree(FVector(0), 0) {}


	FPointsOctree(const FVector& Center, float Bounds) : RootNode(FTreeNode(Center, Bounds)) {}


	void AddElementToLocation(const FVector& Location, ElementType Element)
	{
		RootNode.AddElementToLocation({ Location, Element });
	}

	TArray<ElementType> GetElementsByLocation(const FVector& Location)
	{
		return RootNode.GetElementsByLocation(Location);
	}


	bool IsInside(const FVector& Pos) const { return RootNode.IsInside(Pos); }


};

//////////////////////////////////////////////////////////////////////////

//	NOT FINISHED

//////////////////////////////////////////////////////////////////////////


/**
 * 
 */
class LIBRARY_API EdgeFInderOctree : public FEdgeFinder
{
	FPointsOctree Tree;

	FVector Bounds;

	float MapCellSize;
public:
	EdgeFInderOctree(const FSurfaceNavLocalData& NavData) : FEdgeFinder(NavData) {}
	~EdgeFInderOctree() {}

	virtual int32 FindEdgeIndex(const FVector& Location) const override;

	virtual bool IsReady() const override;

	virtual bool Rebuild() override;




	void BuildOctree(float CellSize, float Size);
	bool GetEdgesFromTree(const FVector& Location, TArray<int32>& OutEdges);
};
