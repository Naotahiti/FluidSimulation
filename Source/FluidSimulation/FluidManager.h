// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PaperSpriteComponent.h"
#include "PaperSprite.h"
#include "FluidManager.generated.h"

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
	bool iswater;

	UPROPERTY(EditAnywhere, Category = "fluid")
	float distance; //between particles and their neighbors at spawn

	UPROPERTY(EditAnywhere, Category = "fluid")
	TSubclassOf<UPaperSpriteComponent> SpriteComponentClass;

	TArray<UPaperSprite*> spritelist;

};
