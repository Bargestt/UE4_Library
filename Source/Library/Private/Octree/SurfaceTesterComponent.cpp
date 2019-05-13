// Fill out your copyright notice in the Description page of Project Settings.

#include "SurfaceTesterComponent.h"
#include "SceneProxy.h"
#include "DrawDebugHelpers.h"




USurfaceTesterComponent::USurfaceTesterComponent()
{
	ShowOctreeCells = false;
}

FPrimitiveSceneProxy* USurfaceTesterComponent::CreateSceneProxy()
{
	class FSurfaceTesterSceneProxy final : public FBaseSceneProxy
	{
	public:
		SIZE_T GetTypeHash() const override
		{
			static size_t UniquePointer;
			return reinterpret_cast<size_t>(&UniquePointer);
		}

		FSurfaceTesterSceneProxy(const USurfaceTesterComponent* InComponent, bool DrawOnlyIfSelected, FColor BaseColor)
			: FBaseSceneProxy(InComponent, DrawOnlyIfSelected, BaseColor)
			, Faces(InComponent->GetFaces())
			, FaceSize(InComponent->CellSize)
			, BoxExtents(InComponent->VolumeSize)
		{
			bWillEverBeLit = false;
		}
	
		virtual void Draw(FPrimitiveDrawInterface* PDI, const FMatrix& LocalToWorld, const FLinearColor SelectionColor) const override
		{
			DrawOrientedWireBox(PDI, LocalToWorld.GetOrigin(), LocalToWorld.GetScaledAxis(EAxis::X), LocalToWorld.GetScaledAxis(EAxis::Y), LocalToWorld.GetScaledAxis(EAxis::Z), BoxExtents, SelectionColor, SDPG_World, 4);

			for (const FFace& face : Faces)
			{
				PrimitiveDraw::DrawQuad(PDI, LocalToWorld.TransformPosition(face.Origin), face.Orient, FaceSize, SelectionColor, 1);
			}
		}

	protected:
		const TArray<FFace> Faces;
		const float FaceSize;
		const FVector BoxExtents;
	};


	if (ShowOctreeCells)
	{
		return Super::CreateSceneProxy();
	}
	return new FSurfaceTesterSceneProxy(this, false, FColor::Red);
}

void USurfaceTesterComponent::GenerateFaces()
{
	Faces.Reset(0);

	const TArray<FIntVector> neighbours = 
	{
		{ -1, 0, 0 }, { 1, 0, 0 },
		{ 0, -1, 0 }, { 0, 1, 0 },
		{ 0, 0, -1 }, { 0, 0, 1 },
	};


	for (int z = 0; z < CellsDimensions.Z; z++)
	{
		for (int y = 0; y < CellsDimensions.Y; y++)
		{
			for (int x = 0; x < CellsDimensions.X; x++)
			{
				FIntVector curCoord = FIntVector(x, y, z);
				FTesterCell cell = Result.GetCell(curCoord);

				if (cell.IsSafe)
				{
					continue;
				}

				for (const FIntVector& vec : neighbours)
				{
					FIntVector neighbourCoord = curCoord + vec;
					if (Result.IsValidCoordinate(neighbourCoord))
					{
						const FTesterCell& neighbour = Result.GetCell(neighbourCoord);
						if (neighbour.IsSafe)
						{
							FVector dir = (neighbour.RelativeLocation - cell.RelativeLocation) / 2;

							Faces.Add({ cell.RelativeLocation + dir , FRotationMatrix::MakeFromX(dir).ToQuat() });
						}
					}
				}

			}
		}
	}
}
