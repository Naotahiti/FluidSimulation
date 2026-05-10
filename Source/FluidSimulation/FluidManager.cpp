#include "FluidManager.h"

AFluidManager::AFluidManager()
{
    PrimaryActorTick.bCanEverTick = true;

    BoundsBox = CreateDefaultSubobject<UBoxComponent>(TEXT("BoundsBox"));
    RootComponent = BoundsBox;

    BoundsBox->SetBoxExtent(FVector(500.f, 500.f, 500.f));

    ISM = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ISM"));
    ISM->SetupAttachment(BoundsBox);
}

void AFluidManager::BeginPlay()
{
    Super::BeginPlay();

    SpawnParticles();
}

void AFluidManager::SpawnParticles()
{
 


    for(int i = 0;i<numtospawn ; i++)
    {
        FFluidParticle P;

        
        FVector BoundsMin = BoundsBox->GetComponentLocation() - BoundsBox->GetScaledBoxExtent();
        FVector BoundsMax = BoundsBox->GetComponentLocation() + BoundsBox->GetScaledBoxExtent();

        FVector v = FMath::RandPointInBox({ BoundsMin, BoundsMax });
        FTransform t;
        P.Position = v;
        P.PreviousPosition = P.Position;
        t.SetLocation(v);
        t.SetScale3D(FVector(ParticleRadius,ParticleRadius,ParticleRadius));
        Particles.Add(P);

        ISM->AddInstance(t);
    }

}

void AFluidManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    Simulate(DeltaTime);

    BuildSpatialGrid();

    for (int i = 0; i < SolverIterations; i++)
    {
        SolveFluid();

        for (FFluidParticle& P : Particles)
        {
            SolveBoundsCollision(P);
        }
    }
    for (int Iteration = 0; Iteration < SolverIterations; Iteration++)
    {
        SolveParticleCollisions();

        for (FFluidParticle& P : Particles)
        {
            SolveBoundsCollision(P);
        }
    }

    UpdateRender();
}

void AFluidManager::Simulate(float DeltaTime)
{
    for (FFluidParticle& P : Particles)
    {
        FVector CurrentPosition = P.Position;

        FVector Velocity = P.Position - P.PreviousPosition;

        Velocity += Gravity * DeltaTime;

        P.Position += Velocity * DeltaTime * 60.f;

        P.PreviousPosition = CurrentPosition;
    }
}

FIntVector AFluidManager::GetGridCell(const FVector& Position) const
{
    return FIntVector(
        FMath::FloorToInt(Position.X / InteractionRadius),
        FMath::FloorToInt(Position.Y / InteractionRadius),
        FMath::FloorToInt(Position.Z / InteractionRadius)
    );
}

void AFluidManager::BuildSpatialGrid()
{
    SpatialGrid.Empty();

    for (int i = 0; i < Particles.Num(); i++)
    {
        FIntVector Cell = GetGridCell(Particles[i].Position);

        SpatialGrid.FindOrAdd(Cell).Add(i);
    }
}

float AFluidManager::SmoothingKernel(float Radius, float Distance)
{
    if (Distance >= Radius)
        return 0.f;

    float Value = Radius * Radius - Distance * Distance;

    return Value * Value * Value;
}

FVector AFluidManager::CalculatePressureForce(int ParticleIndex)
{
    FVector Force = FVector::ZeroVector;

    FFluidParticle& Particle = Particles[ParticleIndex];

    FIntVector Cell = GetGridCell(Particle.Position);

    for (int x = -1; x <= 1; x++)
    {
        for (int y = -1; y <= 1; y++)
        {
            for (int z = -1; z <= 1; z++)
            {
                FIntVector NeighborCell = Cell + FIntVector(x, y, z);

                if (!SpatialGrid.Contains(NeighborCell))
                    continue;

                for (int NeighborIndex : SpatialGrid[NeighborCell])
                {
                    if (NeighborIndex == ParticleIndex)
                        continue;

                    FFluidParticle& Neighbor = Particles[NeighborIndex];

                    FVector Delta = Neighbor.Position - Particle.Position;

                    float Distance = Delta.Length();

                    if (Distance > InteractionRadius || Distance <= 0.001f)
                        continue;

                    FVector Direction = Delta / Distance;

                    float Density = SmoothingKernel(
                        InteractionRadius,
                        Distance
                    );

                    float Pressure = (Density - RestDensity)
                        * PressureMultiplier;

                    Force -= Direction * Pressure * 0.01f;
                }
            }
        }
    }

    return Force;
}

FVector AFluidManager::CalculateViscosityForce(int ParticleIndex)
{
    FVector Force = FVector::ZeroVector;

    FFluidParticle& Particle = Particles[ParticleIndex];

    FVector Velocity =
        Particle.Position - Particle.PreviousPosition;

    FIntVector Cell = GetGridCell(Particle.Position);

    for (int x = -1; x <= 1; x++)
    {
        for (int y = -1; y <= 1; y++)
        {
            for (int z = -1; z <= 1; z++)
            {
                FIntVector NeighborCell = Cell + FIntVector(x, y, z);

                if (!SpatialGrid.Contains(NeighborCell))
                    continue;

                for (int NeighborIndex : SpatialGrid[NeighborCell])
                {
                    if (NeighborIndex == ParticleIndex)
                        continue;

                    FFluidParticle& Neighbor = Particles[NeighborIndex];

                    FVector Delta = Neighbor.Position - Particle.Position;

                    float Distance = Delta.Length();

                    if (Distance > InteractionRadius)
                        continue;

                    FVector NeighborVelocity =
                        Neighbor.Position - Neighbor.PreviousPosition;

                    Force +=
                        (NeighborVelocity - Velocity)
                        * ViscosityStrength;
                }
            }
        }
    }

    return Force;
}

void AFluidManager::SolveFluid()
{
    for (int i = 0; i < Particles.Num(); i++)
    {
        FVector PressureForce =
            CalculatePressureForce(i);

        FVector ViscosityForce =
            CalculateViscosityForce(i);

        Particles[i].Position +=
            (PressureForce + ViscosityForce);
    }
}
void AFluidManager::SolveParticleCollisions()
{
    float MinDistance = ParticleRadius * 2.f;

    for (int i = 0; i < Particles.Num(); i++)
    {
        for (int j = i + 1; j < Particles.Num(); j++)
        {
            FVector Delta =
                Particles[j].Position
                - Particles[i].Position;

            float Distance = Delta.Length();

            if (Distance <= 0.001f)
                continue;

            if (Distance < MinDistance)
            {
                FVector Direction =
                    Delta / Distance;

                float Correction =
                    (MinDistance - Distance) * 0.5f;

                Particles[i].Position -=
                    Direction * Correction;

                Particles[j].Position +=
                    Direction * Correction;
            }
        }
    }
}

void AFluidManager::SolveBoundsCollision(FFluidParticle& P)
{
    FVector Extent = BoundsBox->GetScaledBoxExtent();
    FVector Center = BoundsBox->GetComponentLocation();

    FVector Min = Center - Extent;
    FVector Max = Center + Extent;

    FVector Velocity =
        P.Position - P.PreviousPosition;

    float Bounce = 0.15f;

    if (P.Position.X < Min.X)
    {
        P.Position.X = Min.X;
        Velocity.X *= -Bounce;
    }
    else if (P.Position.X > Max.X)
    {
        P.Position.X = Max.X;
        Velocity.X *= -Bounce;
    }

    if (P.Position.Y < Min.Y)
    {
        P.Position.Y = Min.Y;
        Velocity.Y *= -Bounce;
    }
    else if (P.Position.Y > Max.Y)
    {
        P.Position.Y = Max.Y;
        Velocity.Y *= -Bounce;
    }

    if (P.Position.Z < Min.Z)
    {
        P.Position.Z = Min.Z;
        Velocity.Z *= -Bounce;
    }
    else if (P.Position.Z > Max.Z)
    {
        P.Position.Z = Max.Z;
        Velocity.Z *= -Bounce;
    }

    P.PreviousPosition = P.Position - Velocity;
}

void AFluidManager::UpdateRender()
{
    for (int i = 0; i < Particles.Num(); i++)
    {
        FTransform T;
        T.SetLocation(Particles[i].Position);
        T.SetScale3D(FVector(ParticleRadius, ParticleRadius, ParticleRadius));
        ISM->UpdateInstanceTransform(
            i,
            FTransform(T),
            false,
            false
        );
    }

    ISM->MarkRenderStateDirty();
}
