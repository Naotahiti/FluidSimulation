// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Particle.h"
#include "FluidManager.generated.h"

UENUM(BlueprintType)
enum class EFluidType : uint8
{
	Water UMETA(DisplayName = "Water"),
	Oil UMETA(DisplayName = "Oil")
};

UCLASS()
class FLUIDSIMULATION_API AFluidManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFluidManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;



public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void spawnparticles();

	TArray<int> chunks;

	UPROPERTY(EditAnywhere, Category = "fluid")
	int numofparticles = 1000;

	UPROPERTY(EditAnywhere,Category = "fluid")
	EFluidType fluid_type;

	UPROPERTY(EditAnywhere, Category = "fluid")
	float distance; //between particles and their neighbors at spawn

	UPROPERTY(EditAnywhere , Category = "fluid")
	float waterdensity;

	UPROPERTY(EditAnywhere , Category = "fluid")
	float oildensity;

	UPROPERTY(EditAnywhere, Category = "fluid")
	int gridX;

	UPROPERTY(EditAnywhere, Category = "fluid")
	int gridY;

	FVector managerpos; // actor's position

	UPROPERTY(EditAnywhere, Category = "fluid")
	UInstancedStaticMeshComponent* ism;

	TArray <Particle*> particles;

};
