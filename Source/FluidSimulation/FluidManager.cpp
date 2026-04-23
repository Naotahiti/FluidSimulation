// Fill out your copyright notice in the Description page of Project Settings.


#include "FluidManager.h"

// Sets default values
AFluidManager::AFluidManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	

}

// Called when the game starts or when spawned
void AFluidManager::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AFluidManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AFluidManager::spawnparticles()
{
	for (int i = 0; i < numofparticles; i++)
	{
		
		this->AddComponentByClass(SpriteComponentClass, false, FTransform::Identity, true);
	}
}

