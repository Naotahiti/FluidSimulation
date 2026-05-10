// Fill out your copyright notice in the Description page of Project Settings.


#include "FluidManager.h"
#include "Particle.h"


// Sets default values
AFluidManager::AFluidManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	ism = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ParticleMesh"));
	

}

// Called when the game starts or when spawned
void AFluidManager::BeginPlay()
{
	Super::BeginPlay();

	managerpos = GetActorLocation();
	spawnparticles();
	
}

// Called every frame
void AFluidManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AFluidManager::spawnparticles()
{
	for (int i = 0; i < gridX; i++)
	{
		
		for (int j = 0; j < gridY; j++)

		{
			FVector pos = managerpos + FVector( managerpos.X , distance * i , distance*j)  ;
			
			Particle* newparticle = new Particle(pos , FVector(0.,0.,0.));
			particles.Add(newparticle);

			// add new mesh each time a boid is created
			FTransform InstanceTransform;
			InstanceTransform.SetLocation(pos);
			InstanceTransform.SetScale3D(FVector(0.2, 0.2, 0.2));
			ism->AddInstance(InstanceTransform);
			ism->MarkRenderStateDirty();

		}
	}
	
}

