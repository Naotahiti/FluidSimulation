// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "FluidManager.generated.h"

UENUM(BlueprintType) // pas intégré
enum class EFluidType : uint8
{
	Water UMETA(DisplayName = "Water"),
	Oil UMETA(DisplayName = "Oil")
};
USTRUCT()
struct FFluidParticle
{
    GENERATED_BODY()

    FVector Position;
    FVector PreviousPosition;
    FVector Velocity;

    float Density = 0.f;
    float Pressure = 0.f;
};

UCLASS()
class FLUIDSIMULATION_API AFluidManager : public AActor
{
    GENERATED_BODY()

public:
    AFluidManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    void SpawnParticles();

    void Simulate(float DeltaTime);

    void BuildSpatialGrid();

    void SolveFluid();

    void SolveBoundsCollision(FFluidParticle& P);

    void SolveParticleCollisions();

    void UpdateRender();

    float SmoothingKernel(float Radius, float Distance);

    FVector CalculatePressureForce(int ParticleIndex);

    FVector CalculateViscosityForce(int ParticleIndex);

    FIntVector GetGridCell(const FVector& Position) const;

public:

    UPROPERTY(VisibleAnywhere)
    UBoxComponent* BoundsBox;

    UPROPERTY(VisibleAnywhere)
    UInstancedStaticMeshComponent* ISM;

  

    //UPROPERTY(EditAnywhere, Category = "Fluid")
    //int NumX = 50;

    //UPROPERTY(EditAnywhere, Category = "Fluid")
    //int NumZ = 50;

    UPROPERTY(EditAnywhere, Category = "Fluid")
    int numtospawn;

    UPROPERTY(EditAnywhere, Category = "Fluid")
    float Spacing = 15.f;

    UPROPERTY(EditAnywhere, Category = "Fluid")
    float ParticleRadius = 10.f;

    UPROPERTY(EditAnywhere, Category = "Fluid")
    float InteractionRadius = 25.f;

    UPROPERTY(EditAnywhere, Category = "Fluid")
    float RestDensity = 1.f;

    UPROPERTY(EditAnywhere, Category = "Fluid")
    float PressureMultiplier = 300.f;

    UPROPERTY(EditAnywhere, Category = "Fluid")
    float ViscosityStrength = 0.15f;

    UPROPERTY(EditAnywhere, Category = "Fluid")
    FVector Gravity = FVector(0.f, 0.f, -980.f);

    UPROPERTY(EditAnywhere, Category = "Fluid")
    int SolverIterations = 4;

    TArray<FFluidParticle> Particles;

    TMap<FIntVector, TArray<int>> SpatialGrid;
};
